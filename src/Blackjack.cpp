#include "Blackjack.hpp"

#include <vector>
#include <string>

//Real game: the deck gets shuffled at the start of every round
Blackjack::Blackjack(int startingChips)
    : startingChips(startingChips), chips(startingChips), currentHand(0), stacked(false),
      phase(Phase::Betting), deck(), dealerHand(), playerHands(), bets(), outcomes(), payout(0) {}

//Test game: uses the stacked deck exactly as given, no shuffling
Blackjack::Blackjack(const Deck& stackedDeck, int startingChips)
    : startingChips(startingChips), chips(startingChips), currentHand(0), stacked(true),
      phase(Phase::Betting), deck(stackedDeck), dealerHand(), playerHands(), bets(), outcomes(), payout(0) {}

/*-------------------------------------------------*
*                     Actions                      *
*--------------------------------------------------*/

bool Blackjack::startNewGame(int bet) {
    if (phase != Phase::Betting || !isValidBet(bet)) {
        return false;
    }

    //Fresh shuffled deck every round (a stacked test deck is left alone)
    if (!stacked) {
        deck.reset();
        deck.shuffle();
    }

    chips -= bet; //Bet comes out right away
    playerHands.clear();
    playerHands.push_back(Hand());
    bets = {bet}; //Initial bet for the first hand
    outcomes = {Outcome::None};
    dealerHand.clear();
    currentHand = 0;
    payout = 0;

    //Deal initial cards: player, dealer, player, dealer
    playerHands[0].addCard(deck.deal());
    dealerHand.addCard(deck.deal());
    playerHands[0].addCard(deck.deal());
    dealerHand.addCard(deck.deal());

    //Check for naturals right after the deal, the player doesn't get to act
    if (playerHands[0].isBlackjack() || dealerHand.isBlackjack()) {
        settleHand(0);
        phase = Phase::RoundOver;
    } else {
        phase = Phase::PlayerTurn;
    }
    return true;
}

bool Blackjack::playerHit() {
    if (phase != Phase::PlayerTurn) {
        return false;
    }
    playerHands[currentHand].addCard(deck.deal());

    //Bust ends this hand, and 21 can't get any better so move on too
    if (playerHands[currentHand].isBust() || playerHands[currentHand].getValue() == 21) {
        nextHand();
    }
    return true;
}

bool Blackjack::playerStand() {
    if (phase != Phase::PlayerTurn) {
        return false;
    }
    nextHand();
    return true;
}

bool Blackjack::playerDoubleDown() {
    if (!canDoubleDown()) {
        return false;
    }
    chips -= bets[currentHand]; //Sub additional bet
    bets[currentHand] *= 2; //Double it
    playerHands[currentHand].addCard(deck.deal()); //Player draws exactly one card
    nextHand(); //That hand is done
    return true;
}

bool Blackjack::playerSplit() {
    if (!canSplit()) {
        return false;
    }
    chips -= bets[0]; //Sub additional bet for the new hand
    bets.push_back(bets[0]); //Add same bet for the split hand
    outcomes.push_back(Outcome::None);

    //Save both cards BEFORE clearing the hand
    Card first = playerHands[0].getCardNum(0);
    Card second = playerHands[0].getCardNum(1);

    playerHands[0].clear();
    playerHands[0].addCard(first); //Keep the first card in the current hand

    Hand newHand;
    newHand.addCard(second); //New hand gets the second card
    playerHands.push_back(newHand);

    //Deal one new card to each hand
    playerHands[0].addCard(deck.deal());
    playerHands[1].addCard(deck.deal());
    return true;
}

//Each call is ONE dealer move, so a view can wait between calls
//Dealer must hit on 16 or less and stand on 17 or more (soft 17 too)
bool Blackjack::dealerStep() {
    if (phase != Phase::DealerTurn) {
        return false;
    }
    if (dealerHand.getValue() < 17) {
        dealerHand.addCard(deck.deal()); //Draw one card
    } else {
        finishRound(); //Dealer is done, settle every hand
    }
    return true;
}

bool Blackjack::nextRound() {
    if (phase != Phase::RoundOver) {
        return false;
    }
    playerHands.clear();
    bets.clear();
    outcomes.clear();
    dealerHand.clear();
    currentHand = 0;
    payout = 0;

    if (chips == 0) {
        phase = Phase::GameOver; //Out of chips
    } else {
        phase = Phase::Betting;
    }
    return true;
}

bool Blackjack::restart() {
    if (phase != Phase::GameOver) {
        return false;
    }
    chips = startingChips;
    phase = Phase::Betting;
    return true;
}

/*-------------------------------------------------*
*                 Private helpers                  *
*--------------------------------------------------*/

bool Blackjack::anyHandLeft() const {
    for (int i = 0; i < (int)playerHands.size(); i++) {
        if (!playerHands[i].isBust()) {
            return true;
        }
    }
    return false;
}

void Blackjack::nextHand() {
    if (currentHand + 1 < (int)playerHands.size()) {
        currentHand++; //Go play the split hand
    } else if (anyHandLeft()) {
        phase = Phase::DealerTurn; //Dealer flips the hole card and plays one step at a time
    } else {
        finishRound(); //Every hand busted, so the dealer doesn't need to play
    }
}

void Blackjack::finishRound() {
    for (int i = 0; i < (int)playerHands.size(); i++) {
        settleHand(i);
    }
    phase = Phase::RoundOver;
}

void Blackjack::settleHand(int i) {
    const Hand& hand = playerHands[i];
    int bet = bets[i];

    //A natural only counts on the first two cards (not after a split)
    bool playerNatural = playerHands.size() == 1 && hand.isBlackjack();
    bool dealerNatural = dealerHand.isBlackjack();

    int won = 0; //Chips going back to the player for this hand
    if (hand.isBust()) {
        outcomes[i] = Outcome::PlayerBust;
    } else if (playerNatural && dealerNatural) {
        outcomes[i] = Outcome::Push;
        won = bet; //Bet returned
    } else if (playerNatural) {
        outcomes[i] = Outcome::PlayerBlackjack;
        won = bet + bet * 3 / 2; //Bet back plus 3:2
    } else if (dealerNatural) {
        outcomes[i] = Outcome::DealerWin;
    } else if (dealerHand.isBust()) {
        outcomes[i] = Outcome::DealerBust;
        won = bet * 2; //Bet back plus 1:1
    } else if (hand.getValue() > dealerHand.getValue()) {
        outcomes[i] = Outcome::PlayerWin;
        won = bet * 2;
    } else if (hand.getValue() < dealerHand.getValue()) {
        outcomes[i] = Outcome::DealerWin;
    } else {
        outcomes[i] = Outcome::Push;
        won = bet;
    }

    chips += won;
    payout += won;
}

/*-------------------------------------------------*
*                    Questions                     *
*--------------------------------------------------*/

Phase Blackjack::getPhase() const { return phase; }
int Blackjack::getChips() const { return chips; }

bool Blackjack::isValidBet(int bet) const {
    return bet >= 1 && bet <= chips;
}

bool Blackjack::canSplit() const {
    if (phase != Phase::PlayerTurn || playerHands.size() != 1) {
        return false; //Only one split per round
    }
    const Hand& hand = playerHands[0];
    if (hand.handSize() != 2 || chips < bets[0]) {
        return false;
    }
    const Card& firstCard = hand.getCardNum(0);
    const Card& secondCard = hand.getCardNum(1);
    return firstCard.getRank() == secondCard.getRank();
}

bool Blackjack::canDoubleDown() const {
    if (phase != Phase::PlayerTurn) {
        return false;
    }
    return playerHands[currentHand].handSize() == 2 && chips >= bets[currentHand];
}

//The hole card stays face down until the dealer plays
bool Blackjack::dealerHoleHidden() const {
    return phase == Phase::PlayerTurn;
}

int Blackjack::getHandCount() const { return (int)playerHands.size(); }
int Blackjack::getCurrentHand() const { return currentHand; }
const Hand& Blackjack::getPlayerHand(int handIndex) const { return playerHands.at(handIndex); }
const Hand& Blackjack::getDealerHand() const { return dealerHand; }
int Blackjack::getBet(int handIndex) const { return bets.at(handIndex); }
Outcome Blackjack::getOutcome(int handIndex) const { return outcomes.at(handIndex); }

int Blackjack::getRoundNet() const {
    int totalBet = 0;
    for (int i = 0; i < (int)bets.size(); i++) {
        totalBet += bets[i];
    }
    return payout - totalBet;
}

bool Blackjack::isPlayerBust(int handIndex) const {
    return playerHands.at(handIndex).isBust();
}

bool Blackjack::isDealerBust() const {
    return dealerHand.isBust();
}

int Blackjack::getPlayerValue(int handIndex) const {
    return playerHands.at(handIndex).getValue();
}

int Blackjack::getDealerValue() const {
    return dealerHand.getValue();
}

std::string Blackjack::getPlayerHandString(int handIndex) const {
    return playerHands.at(handIndex).toString();
}

std::string Blackjack::getDealerHandString() const {
    if (!dealerHoleHidden()) {
        return dealerHand.toString();
    }
    //First card face up, second card face down
    return dealerHand.getCardNum(0).toString() + " ?? ";
}

std::string Blackjack::outcomeName(Outcome o) {
    switch (o) {
    case Outcome::PlayerBlackjack: return "Blackjack! Pays 3:2";
    case Outcome::PlayerWin:       return "You win";
    case Outcome::DealerWin:       return "Dealer wins";
    case Outcome::Push:            return "Push - bet returned";
    case Outcome::PlayerBust:      return "Bust";
    case Outcome::DealerBust:      return "Dealer busts - you win";
    case Outcome::None:            return "";
    }
    return "";
}
