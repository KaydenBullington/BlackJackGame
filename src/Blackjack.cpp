#include "blackjack.hpp"

#include <vector>

Blackjack::Blackjack() : chips(1000), bet(0), deck(), dealerHand(), playerHand() {
    deck.shuffle(); // Shuffle the deck at the start of the game
}

void Blackjack::startNewGame(int bet) {
    this->bet = bet;
    playerHand.clear();
    dealerHand.clear();
    deck.reset();
    deck.shuffle();

    // Deal initial cards
    playerHand.addCard(deck.deal());
    dealerHand.addCard(deck.deal());
    playerHand.addCard(deck.deal());
    dealerHand.addCard(deck.deal());
}

void Blackjack::playerHit() {
    playerHand.addCard(deck.deal());
}

void Blackjack::playerStand() {
    dealerPlay();
}

void Blackjack::playerDoubleDown() {
    chips -= bet; //sub additional bet
    bet *= 2; //Double it
    playerHand.addCard(deck.deal()); //Player draws one card
    dealerPlay(); //Dealer plays after player doubles down
}

void Blackjack::playerSplit() {
   
}