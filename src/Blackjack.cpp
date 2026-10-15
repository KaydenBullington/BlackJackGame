#include "blackjack.hpp"

#include <vector>

Blackjack::Blackjack() : chips(1000), bets(0), deck(), dealerHand(), playerHands() {
    deck.shuffle(); // Shuffle the deck at the start of the game
}

void Blackjack::startNewGame(int bet) {
    this->bets = {bet};//Initial bet fo rthe efirst hand
    playerHands.clear();
    dealerHand.clear();
    deck.reset();
    deck.shuffle();

    // Deal initial cards
    playerHands.push_back(Hand());
    playerHands[0].addCard(deck.deal());
    dealerHand.addCard(deck.deal());
    playerHands[0].addCard(deck.deal());
    dealerHand.addCard(deck.deal());
}

void Blackjack::playerHit() {
    playerHands[currentHand].addCard(deck.deal());
}

void Blackjack::playerStand() {
    dealerPlay();
}

void Blackjack::playerDoubleDown() {
    chips -= bets[currentHand]; //sub additional bet
    bets[currentHand] *= 2; //Double it
    playerHands[currentHand].addCard(deck.deal()); //Player draws one card
    dealerPlay(); //Dealer plays after player doubles down
}

void Blackjack::playerSplit() {
    if (canSplit()) {}
}




