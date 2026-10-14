#pragma once

#include "Card.hpp"
#include <vector>
#include "Card.hpp"

class Hand {
    private:
        std::vector<Card> cards; //Cards on hand
    public:
        Hand(); //Constructor
        void addCard(const Card& c); //Add a card to the hand
        void clear(); //Clear the hand
        bool isSoft() const; //Check if the hand is soft (contains an Ace counted as 11)
        bool isBust() const; //Check if the hand is bust (value exceeds 21)
        int getValue() const; //Get the total value of the hand
        std::string toString() const; //Get a string representation of the hand
};
