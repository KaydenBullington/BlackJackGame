#include "ConsoleView.hpp"

#include <chrono> //For milliseconds
#include <iostream>
#include <string>
#include <thread> //For sleep_for

ConsoleView::ConsoleView(Blackjack& game) : game(game) {}

//A bet has to be all digits (and not too long so stoi can't overflow)
bool ConsoleView::isNumber(const std::string& line) {
    if (line.empty() || line.size() > 6) {
        return false;
    }
    for (char c : line) {
        if (c < '0' || c > '9') {
            return false;
        }
    }
    return true;
}

void ConsoleView::showTable() const {
    std::cout << "\nDealer: " << game.getDealerHandString();
    if (!game.dealerHoleHidden()) {
        std::cout << "(" << game.getDealerValue() << ")"; //Only show the total once the hole card is up
    }
    std::cout << '\n';

    for (int i = 0; i < game.getHandCount(); i++) {
        std::cout << "You";
        if (game.getHandCount() > 1) {
            std::cout << " (hand " << i + 1 << ")";
        }
        std::cout << ": " << game.getPlayerHandString(i) << "(" << game.getPlayerValue(i) << ")"
                  << "   Bet: " << game.getBet(i);

        //Point at the hand being played after a split
        if (game.getPhase() == Phase::PlayerTurn && game.getHandCount() > 1 && i == game.getCurrentHand()) {
            std::cout << "  <-- playing";
        }
        std::cout << '\n';
    }
}

void ConsoleView::showResults() const {
    for (int i = 0; i < game.getHandCount(); i++) {
        if (game.getHandCount() > 1) {
            std::cout << "Hand " << i + 1 << ": ";
        }
        std::cout << Blackjack::outcomeName(game.getOutcome(i)) << '\n';
    }

    int net = game.getRoundNet();
    if (net > 0) {
        std::cout << "You won " << net << " chips.";
    } else if (net < 0) {
        std::cout << "You lost " << -net << " chips.";
    } else {
        std::cout << "You broke even.";
    }
    std::cout << "   Chips: " << game.getChips() << '\n';
}

void ConsoleView::run() {
    std::string line;
    std::cout << "=========== BLACKJACK ===========\n";

    while (true) {
        if (game.getPhase() == Phase::Betting) {
            std::cout << "\nChips: " << game.getChips() << "   Bet (1-" << game.getChips() << ", q to quit): ";
            if (!std::getline(std::cin, line) || line == "q") {
                return;
            }
            //The game checks the bet, the view just reports when it says no
            if (!isNumber(line) || !game.startNewGame(std::stoi(line))) {
                std::cout << "  Please enter a whole number from 1 to " << game.getChips() << ".\n";
            }
        }
        else if (game.getPhase() == Phase::PlayerTurn) {
            showTable();
            std::cout << "(h)it  (s)tand";
            if (game.canDoubleDown()) {
                std::cout << "  (d)ouble down";
            }
            if (game.canSplit()) {
                std::cout << "  s(p)lit";
            }
            std::cout << "  q to quit: ";
            if (!std::getline(std::cin, line) || line == "q") {
                return;
            }

            //Each command is ONE call into the game
            if (line == "h") {
                game.playerHit();
            } else if (line == "s") {
                game.playerStand();
            } else if (line == "d") {
                if (!game.playerDoubleDown()) {
                    std::cout << "  You can't double down right now.\n";
                }
            } else if (line == "p") {
                if (!game.playerSplit()) {
                    std::cout << "  You can't split right now.\n";
                }
            } else {
                std::cout << "  Please type h, s, d, p, or q.\n";
            }
        }
        else if (game.getPhase() == Phase::DealerTurn) {
            //Show the dealer's hand, wait half a second, then let the dealer make one move
            std::cout << "Dealer: " << game.getDealerHandString() << "(" << game.getDealerValue() << ")" << std::endl; //endl so it shows before the wait
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            game.dealerStep();
        }
        else if (game.getPhase() == Phase::RoundOver) {
            showTable();
            showResults();
            std::cout << "Press Enter for the next round (q to quit): ";
            if (!std::getline(std::cin, line) || line == "q") {
                return;
            }
            game.nextRound();
        }
        else { //GameOver
            std::cout << "\nYou're out of chips. GAME OVER.\nPlay again? (y/n): ";
            if (!std::getline(std::cin, line) || line != "y") {
                return;
            }
            game.restart();
        }
    }
}
