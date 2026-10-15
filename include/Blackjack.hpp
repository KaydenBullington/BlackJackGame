#pragma once

#include "Card.hpp"
#include "Deck.hpp"
#include "Hand.hpp"

#include <string>
#include <vector>

class Blackjack {
    private:
        int chips; //Player's chips
        int currentHand; //Index of the current hand being played
        std::vector<Hand> playerHands; //Player's hands (for splits)
        std::vector<int> bets; //Bets for each hand (for splits)
        Deck deck; //Deck of cards
        Hand dealerHand; //Dealer's hand
        void nextHand(); //Move to the next hand for splits
        
    public:
        Blackjack(); //Constructor
        /*-------------------------------------------------*
        *                  Game Functions                  *
        *--------------------------------------------------*/
        void startNewGame(int bet); //Start a new game
        void playerHit(); //Player hits (draws a card)
        void playerStand(); //Player stands (ends their turn)
        void playerDoubleDown(); //Player doubles down (draws one card and ends their turn)
        void playerSplit(); //Player splits (creates two hands if the first two cards are of the same rank)
        bool canSplit() const; //Check if the player can split (first two cards are of the same rank)
        /*-------------------------------------------------*/

        void dealerPlay(); //Dealer plays (draws cards until reaching 17 or higher)
        bool isPlayerBust(int handIndex) const; //Check if the player is bust
        bool isDealerBust() const; //Check if the dealer is bust
        int getPlayerValue(int handIndex) const; //Get the player's hand value
        int getDealerValue() const; //Get the dealer's hand value
        std::string getPlayerHandString(int handIndex) const; //Get a string representation of the player's hand

        //Get a string representation of the dealer'shand, optionally revealing the first card
        std::string getDealerHandString(bool revealFirstCard) const; 
};