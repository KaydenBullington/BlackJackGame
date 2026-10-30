#include "Tests.hpp"
#include "Blackjack.hpp"

#include <iostream>
#include <string>
#include <vector>

//Card numbers (same as Card.hpp): rank = value % 13, suit = value / 13
//Clubs 0-12, Diamonds 13-25, Hearts 26-38, Spades 39-51
//Inside a suit: Ace 0, Two 1, ... Nine 8, Ten 9, Jack 10, Queen 11, King 12
//Stacked decks deal in order: player, dealer, player, dealer, then any extra cards

//Prints one PASS/FAIL line and counts the failures
static void check(bool ok, const std::string& name, int& failures) {
    std::cout << (ok ? "PASS  " : "FAIL  ") << name << '\n';
    if (!ok) {
        failures++;
    }
}

//Builds a hand from card numbers
static Hand makeHand(const std::vector<int>& values) {
    Hand h;
    for (int v : values) {
        h.addCard(Card(v));
    }
    return h;
}

//Lets the dealer take every step at once (tests don't need to wait)
static void finishDealer(Blackjack& game) {
    while (game.dealerStep()) {
    }
}

static void handTests(int& failures) {
    std::cout << "--- Hand values ---\n";

    Hand aceKing = makeHand({0, 12}); //A + K
    check(aceKing.getValue() == 21 && aceKing.isBlackjack(), "A + K = 21 and is a natural", failures);

    Hand aceSix = makeHand({0, 5}); //A + 6
    check(aceSix.getValue() == 17 && aceSix.isSoft(), "A + 6 = 17 soft", failures);

    Hand aceSixTen = makeHand({0, 5, 9}); //A + 6 + 10
    check(aceSixTen.getValue() == 17 && !aceSixTen.isSoft(), "A + 6 + 10 = 17 hard", failures);

    Hand aceAceNine = makeHand({0, 13, 8}); //A + A + 9
    check(aceAceNine.getValue() == 21, "A + A + 9 = 21", failures);

    Hand kingQueenFive = makeHand({12, 11, 4}); //K + Q + 5
    check(kingQueenFive.isBust(), "K + Q + 5 busts", failures);

    Hand tenTenAce = makeHand({9, 22, 0}); //10 + 10 + A (three cards, not a natural)
    check(tenTenAce.getValue() == 21 && !tenTenAce.isBlackjack(), "10 + 10 + A = 21 but is not a natural", failures);
}

static void dealerTests(int& failures) {
    std::cout << "--- Dealer rules ---\n";

    //Player 10C 9C = 19, dealer AD 6D = soft 17, next card 5H
    Blackjack soft17(Deck({Card(9), Card(13), Card(8), Card(18), Card(30)}));
    soft17.startNewGame(10);
    check(soft17.dealerHoleHidden(), "hole card is hidden during the player's turn", failures);
    soft17.playerStand();
    finishDealer(soft17);
    check(soft17.getDealerHand().handSize() == 2 && soft17.getDealerValue() == 17,
          "dealer stands on soft 17", failures);
    check(!soft17.dealerHoleHidden(), "hole card shows after the dealer plays", failures);
    check(soft17.getOutcome(0) == Outcome::PlayerWin && soft17.getChips() == 110,
          "19 beats 17 and pays 1:1 (bet 10 -> +10)", failures);

    //Player 10C 9C = 19, dealer 10D 6D = 16, next card 5H
    Blackjack hit16(Deck({Card(9), Card(22), Card(8), Card(18), Card(30)}));
    hit16.startNewGame(10);
    hit16.playerStand();
    check(hit16.getPhase() == Phase::DealerTurn && !hit16.dealerHoleHidden() && hit16.getDealerHand().handSize() == 2,
          "standing flips the hole card and starts the dealer's turn", failures);
    check(!hit16.playerHit() && !hit16.playerStand(), "player can't act during the dealer's turn", failures);
    hit16.dealerStep();
    check(hit16.getDealerHand().handSize() == 3 && hit16.getPhase() == Phase::DealerTurn,
          "dealer draws one card per step", failures);
    hit16.dealerStep();
    check(hit16.getPhase() == Phase::RoundOver && !hit16.dealerStep(), "dealer finishes on the next step", failures);
    check(hit16.getDealerHand().handSize() == 3 && hit16.getDealerValue() == 21,
          "dealer hits on 16", failures);
    check(hit16.getOutcome(0) == Outcome::DealerWin && hit16.getChips() == 90,
          "dealer 21 beats player 19", failures);

    //Player 10C 9C = 19, dealer 10D 6D = 16, next card KH busts the dealer
    Blackjack dealerBust(Deck({Card(9), Card(22), Card(8), Card(18), Card(38)}));
    dealerBust.startNewGame(10);
    dealerBust.playerStand();
    finishDealer(dealerBust);
    check(dealerBust.isDealerBust() && dealerBust.getOutcome(0) == Outcome::DealerBust && dealerBust.getChips() == 110,
          "dealer bust means the player wins", failures);

    //Player 10C 8C = 18, dealer 10D 8D = 18
    Blackjack push(Deck({Card(9), Card(22), Card(7), Card(20)}));
    push.startNewGame(10);
    push.playerStand();
    finishDealer(push);
    check(push.getOutcome(0) == Outcome::Push && push.getChips() == 100, "a tie is a push", failures);
}

static void naturalTests(int& failures) {
    std::cout << "--- Naturals ---\n";

    //Player AC KC, dealer 9D 7D
    Blackjack natural(Deck({Card(0), Card(21), Card(12), Card(19)}));
    natural.startNewGame(10);
    check(natural.getPhase() == Phase::RoundOver, "natural ends the round right after the deal", failures);
    check(natural.getOutcome(0) == Outcome::PlayerBlackjack && natural.getRoundNet() == 15 && natural.getChips() == 115,
          "natural pays 3:2 (bet 10 -> +15)", failures);

    //Player AC KC, dealer AD KD
    Blackjack bothNatural(Deck({Card(0), Card(13), Card(12), Card(25)}));
    bothNatural.startNewGame(10);
    check(bothNatural.getOutcome(0) == Outcome::Push && bothNatural.getChips() == 100,
          "natural vs dealer natural pushes", failures);

    //Player 10C 9C, dealer AD KD
    Blackjack dealerNatural(Deck({Card(9), Card(13), Card(8), Card(25)}));
    dealerNatural.startNewGame(10);
    check(dealerNatural.getPhase() == Phase::RoundOver && dealerNatural.getOutcome(0) == Outcome::DealerWin &&
              dealerNatural.getChips() == 90,
          "dealer natural only: player loses without acting", failures);
}

static void playerTests(int& failures) {
    std::cout << "--- Player turn ---\n";

    //Player 10C 6C = 16, dealer 9D 7D, player hits KH
    Blackjack bust(Deck({Card(9), Card(21), Card(5), Card(19), Card(38)}));
    bust.startNewGame(10);
    bust.playerHit();
    check(bust.isPlayerBust(0) && bust.getOutcome(0) == Outcome::PlayerBust && bust.getChips() == 90,
          "going over 21 is a bust and a loss", failures);
    check(bust.getDealerHand().handSize() == 2, "dealer doesn't play after a bust", failures);
    check(!bust.playerHit() && !bust.playerStand(), "hit and stand refused after the round is over", failures);

    //Double down: player 6C 5C = 11, dealer 10D 7D = 17, player gets 10H
    Blackjack doubleDown(Deck({Card(5), Card(22), Card(4), Card(19), Card(35)}));
    doubleDown.startNewGame(10);
    check(doubleDown.canDoubleDown(), "can double down on the first two cards", failures);
    doubleDown.playerDoubleDown();
    finishDealer(doubleDown);
    check(doubleDown.getBet(0) == 20 && doubleDown.getPlayerHand(0).handSize() == 3,
          "double down doubles the bet and draws exactly one card", failures);
    check(doubleDown.getPhase() == Phase::RoundOver && doubleDown.getChips() == 120,
          "double down win pays on the doubled bet (+20)", failures);

    //Split: player 8C 8H, dealer 10D 7D, then 3C, 10S, 10C
    Blackjack split(Deck({Card(7), Card(22), Card(33), Card(19), Card(2), Card(48), Card(9)}));
    split.startNewGame(10);
    check(split.canSplit(), "a pair of 8s can split", failures);
    split.playerSplit();
    check(split.getHandCount() == 2 && split.getChips() == 80, "split makes two hands with their own bet", failures);
    check(split.getPlayerValue(0) == 11 && split.getPlayerValue(1) == 18, "each split hand gets one new card", failures);
    check(!split.canSplit(), "can't split a second time", failures);
    split.playerHit(); //Hand 1 hits 10C -> 21, moves to hand 2 by itself
    check(split.getCurrentHand() == 1, "after hand 1 is done, hand 2 is played", failures);
    split.playerStand(); //Hand 2 stands on 18, dealer stands on 17
    finishDealer(split);
    check(split.getOutcome(0) == Outcome::PlayerWin && split.getOutcome(1) == Outcome::PlayerWin &&
              split.getChips() == 120,
          "both split hands get paid", failures);

    //No pair, no split: player 10C 9C
    Blackjack noSplit(Deck({Card(9), Card(21), Card(8), Card(19)}));
    noSplit.startNewGame(10);
    check(!noSplit.canSplit() && !noSplit.playerSplit(), "split refused without a pair", failures);
}

static void phaseTests(int& failures) {
    std::cout << "--- Phases and bets ---\n";

    Blackjack fresh;
    check(!fresh.playerHit() && fresh.getPhase() == Phase::Betting && fresh.getChips() == 100 &&
              fresh.getHandCount() == 0,
          "hit() during Betting returns false and changes nothing", failures);
    check(!fresh.playerStand() && !fresh.playerDoubleDown() && !fresh.nextRound(),
          "stand, double, and nextRound refused during Betting", failures);
    check(!fresh.startNewGame(0) && !fresh.startNewGame(101) && fresh.getChips() == 100,
          "bets of 0 or more than your chips are refused", failures);

    //Bet everything and lose to a dealer natural: player 10C 9C, dealer AD KD
    Blackjack broke(Deck({Card(9), Card(13), Card(8), Card(25)}));
    broke.startNewGame(100);
    broke.nextRound();
    check(broke.getChips() == 0 && broke.getPhase() == Phase::GameOver, "0 chips means game over", failures);
    check(!broke.startNewGame(1), "can't bet after game over", failures);
    check(broke.restart() && broke.getPhase() == Phase::Betting && broke.getChips() == 100,
          "restart gives you 100 chips again", failures);
}

int runTests() {
    int failures = 0;
    handTests(failures);
    dealerTests(failures);
    naturalTests(failures);
    playerTests(failures);
    phaseTests(failures);

    std::cout << '\n' << (failures == 0 ? "All tests passed" : std::to_string(failures) + " test(s) failed") << '\n';
    return failures == 0 ? 0 : 1;
}
