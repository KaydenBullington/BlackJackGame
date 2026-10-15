#include "blackjack.hpp"

#include <vector>
#include <stdexcept>
#include <string>

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



bool Blackjack::canSplit() const {
    if (playerHands[currentHand].handSize() == 2) {
        const Card& firstCard = playerHands[currentHand].getCardNum(0);
        const Card& secondCard = playerHands[currentHand].getCardNum(1);
        return firstCard.getRank() == secondCard.getRank();
    }
    return false;
}


void Blackjack::playerSplit() {
    if (canSplit()) {
        chips -= bets[currentHand]; //Sub additional bet for the new hand
        bets.push_back(bets[currentHand]); //add same bet for the split hand

        //Create new hand with the second card
        Hand newHand;
        newHand.addCard(playerHands[currentHand].getCardNum(1));
        playerHands[currentHand].clear(); //Clear the current hand
        playerHands[currentHand].addCard(playerHands[currentHand].getCardNum(0)); //Keep the first card in the current hand
        playerHands.push_back(newHand); //Add the new hand to playerHands

        //Deal one new card to each hand
        playerHands[currentHand].addCard(deck.deal());
        playerHands.back().addCard(deck.deal());
    }
}

void Blackjack::dealerPlay() {
    while (dealerHand.getValue() < 17 && !dealerHand.isBust() && dealerHand.isSoft()) {
        dealerHand.addCard(deck.deal());
    }
}

bool Blackjack::isPlayerBust(int handIndex) const {
    return playerHands[handIndex].isBust();
}

bool Blackjack::isDealerBust() const {
    return dealerHand.isBust();
}

int Blackjack::getPlayerValue(int handIndex) const {
    return playerHands[handIndex].getValue();
}


int Blackjack::getDealerValue() const {
    return dealerHand.getValue();
}

std::string Blackjack::getPlayerHandString(int handIndex) const {
    return playerHands[handIndex].toString();
}

std::string Blackjack::getDealerHandString(bool revealFirstCard) const {
    if (revealFirstCard) {
        return dealerHand.toString();
    } else {
        std::string handStr = "??";
        for (int i = 1; i < dealerHand.handSize(); ++i) {
            handStr += " " + dealerHand.getCardNum(i).toString();
        }
        return handStr;
    }
}