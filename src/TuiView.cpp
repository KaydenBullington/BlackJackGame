#include "TuiView.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/loop.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <chrono> //For milliseconds
#include <string>
#include <thread> //For sleep_for

using namespace ftxui;

//Drawing ONE card is pure presentation, so it lives in the view file
static Element drawCard(const Card& card) {
    Color c = card.isRed() ? Color::Red : Color::White;
    return vbox({
               hbox({text(card.rankSymbol()), filler()}),
               filler(),
               text(card.suitSymbol()) | bold | center,
               filler(),
               hbox({filler(), text(card.rankSymbol())}),
           }) |
           color(c) | size(WIDTH, EQUAL, 9) | size(HEIGHT, EQUAL, 7) | border;
}

//The dealer's hole card while it's still face down
static Element drawHiddenCard() {
    Elements rows;
    for (int i = 0; i < 7; i++) {
        rows.push_back(text("░░░░░░░░░"));
    }
    return vbox(rows) | color(Color::Blue) | size(WIDTH, EQUAL, 9) | size(HEIGHT, EQUAL, 7) | border;
}

//Every card in a hand, side by side
static Element drawHand(const Hand& hand) {
    Elements cards;
    for (int i = 0; i < hand.handSize(); i++) {
        cards.push_back(drawCard(hand.getCardNum(i)));
    }
    return hbox(cards);
}

//The message line at the end of a round, like "You win (+10 chips)"
static std::string roundSummary(const Blackjack& game) {
    std::string summary;
    for (int i = 0; i < game.getHandCount(); i++) {
        if (game.getHandCount() > 1) {
            summary += "Hand " + std::to_string(i + 1) + ": ";
        }
        summary += Blackjack::outcomeName(game.getOutcome(i)) + "   ";
    }

    int net = game.getRoundNet();
    if (net > 0) {
        summary += "(+" + std::to_string(net) + " chips)";
    } else if (net < 0) {
        summary += "(" + std::to_string(net) + " chips)";
    } else {
        summary += "(even)";
    }
    return summary;
}

TuiView::TuiView(Blackjack& game) : game(game) {}

void TuiView::run() {
    auto screen = ScreenInteractive::TerminalOutput();

    //View-only state: the bet being picked and the last error message
    int bet = 10;
    std::string message;

    //RENDERER: only READS from the game, it never changes anything
    auto renderer = Renderer([&] {
        Phase phase = game.getPhase();

        //---------------- Dealer ----------------
        Element dealerArea = text("");
        if (game.getDealerHand().handSize() > 0) {
            Elements dealerCards;
            for (int i = 0; i < game.getDealerHand().handSize(); i++) {
                //The GAME says if the hole card is hidden, the view just obeys
                if (i == 1 && game.dealerHoleHidden()) {
                    dealerCards.push_back(drawHiddenCard());
                } else {
                    dealerCards.push_back(drawCard(game.getDealerHand().getCardNum(i)));
                }
            }
            std::string dealerTitle = "Dealer";
            if (!game.dealerHoleHidden()) {
                dealerTitle += "  (" + std::to_string(game.getDealerValue()) + ")";
            }
            dealerArea = vbox({text(dealerTitle) | bold, hbox(dealerCards)});
        }

        //---------------- Player ----------------
        Elements playerAreas;
        for (int i = 0; i < game.getHandCount(); i++) {
            std::string title = game.getHandCount() > 1 ? "Hand " + std::to_string(i + 1) : "You";
            title += "  (" + std::to_string(game.getPlayerValue(i)) + ")";
            title += "   Bet: " + std::to_string(game.getBet(i));

            Element label = text(title) | bold;
            //Highlight the hand being played after a split
            if (phase == Phase::PlayerTurn && game.getHandCount() > 1 && i == game.getCurrentHand()) {
                label = text("> " + title) | bold | color(Color::Yellow);
            }
            playerAreas.push_back(vbox({label, drawHand(game.getPlayerHand(i))}));
        }
        if (phase == Phase::Betting) {
            playerAreas.push_back(text("Bet: " + std::to_string(bet)) | bold | center);
        }

        //---------------- Message line ----------------
        std::string info;
        if (!message.empty()) {
            info = message;
        } else if (phase == Phase::Betting) {
            info = "Pick your bet and press Enter to deal";
        } else if (phase == Phase::PlayerTurn) {
            info = "Your move";
        } else if (phase == Phase::DealerTurn) {
            info = "Dealer is playing...";
        } else if (phase == Phase::RoundOver) {
            info = roundSummary(game);
        } else {
            info = "Out of chips - GAME OVER";
        }

        //---------------- Keys for this phase ----------------
        std::string keys;
        if (phase == Phase::Betting) {
            keys = "↑/↓ bet ±1   ←/→ bet ±10   Enter deal   q quit";
        } else if (phase == Phase::PlayerTurn) {
            keys = "h hit   s stand";
            if (game.canDoubleDown()) {
                keys += "   d double";
            }
            if (game.canSplit()) {
                keys += "   p split";
            }
            keys += "   q quit";
        } else if (phase == Phase::DealerTurn) {
            keys = "q quit";
        } else if (phase == Phase::RoundOver) {
            keys = "Enter next round   q quit";
        } else {
            keys = "r play again   q quit";
        }

        return vbox({
                   text("BLACKJACK") | bold | center,
                   separator(),
                   dealerArea,
                   separator(),
                   vbox(playerAreas),
                   separator(),
                   text(info) | bold | center | color(Color::Yellow),
                   text("Chips: " + std::to_string(game.getChips())),
                   separator(),
                   text(keys) | dim | center,
               }) |
               border | size(WIDTH, GREATER_THAN, 60);
    });

    //EVENT HANDLER: each key becomes ONE call into the game, no rules here
    auto app = CatchEvent(renderer, [&](Event e) {
        if (e == Event::Character('q')) {
            screen.Exit();
            return true;
        }
        if (e == Event::Return || e.is_character()) {
            message = ""; //A new key press clears the old message
        }

        //Picking a bet (the game decides what a valid bet is)
        if (e == Event::ArrowUp && game.isValidBet(bet + 1)) {
            bet++;
            return true;
        }
        if (e == Event::ArrowDown && game.isValidBet(bet - 1)) {
            bet--;
            return true;
        }
        if (e == Event::ArrowRight) {
            bet = game.isValidBet(bet + 10) ? bet + 10 : game.getChips();
            return true;
        }
        if (e == Event::ArrowLeft) {
            bet = game.isValidBet(bet - 10) ? bet - 10 : 1;
            return true;
        }

        //Enter deals during betting, and starts the next round after one ends
        if (e == Event::Return) {
            if (game.getPhase() == Phase::Betting) {
                if (!game.startNewGame(bet)) {
                    message = "Bet must be 1 to " + std::to_string(game.getChips());
                }
            } else {
                game.nextRound();
            }
            //Keep the bet picker inside the chips we have now
            if (bet > game.getChips()) {
                bet = game.getChips();
            }
            if (bet < 1) {
                bet = 1;
            }
            return true;
        }

        if (e == Event::Character('h')) {
            return game.playerHit();
        }
        if (e == Event::Character('s')) {
            return game.playerStand();
        }
        if (e == Event::Character('d')) {
            if (!game.playerDoubleDown()) {
                message = "You can't double down right now";
            }
            return true;
        }
        if (e == Event::Character('p')) {
            if (!game.playerSplit()) {
                message = "You can't split right now";
            }
            return true;
        }
        if (e == Event::Character('r')) {
            return game.restart();
        }
        return false;
    });

    //Our own loop instead of screen.Loop(app) so the dealer can play slowly
    Loop loop(&screen, app);
    int ticks = 0;
    while (!loop.HasQuitted()) {
        loop.RunOnce(); //Handle any key presses and redraw the screen
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        //During the dealer's turn, make one dealer move every 10 ticks (half a second)
        if (game.getPhase() == Phase::DealerTurn) {
            ticks++;
            if (ticks >= 10) {
                ticks = 0;
                game.dealerStep();
                screen.PostEvent(Event::Custom); //Tells FTXUI to redraw
            }
        } else {
            ticks = 0;
        }
    }
}
