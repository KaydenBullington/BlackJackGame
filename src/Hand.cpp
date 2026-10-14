#include "Hand.hpp"

#include <vector>

Hand::Hand() : cards() {} //Constructor


//Add a card to the hand
void Hand::addCard(const Card& c) {
    cards.push_back(c);
} 

//Get the total value of the hand
int Hand::getValue() const {
    int total = 0;
    int numAces = 0;
        
        for(const Card& c : cards) {
            int rank = c.getRank();
            if(rank > 9) { //Face cards
                total += 10;
            } else if(rank == 0) { //Ace
                total += 11;
                numAces++;
            } else {
                total += rank + 1; //Number cards
            }
        }

        while(total > 21 && numAces > 0) {
            total -= 10; //Count Ace as 1 instead of 11
            numAces--;
        }

        return total;
}
bool Hand::isSoft() const {
    int total = 0;
    int numAces = 0;

        for(const Card& c : cards) {
            int rank = c.getRank();
                if(rank > 9) { //Face cards
                     total += 10;
                } else if(rank == 0) { //Ace
                    total += 11;
                    numAces++;
                } else {
                    total += rank + 1; //Number cards
                }
        }

        while (total > 21 && numAces > 0) {
            total -= 10;
            numAces--;
        }

    return numAces > 0;   //any ace left still counted as 11
}


//  std::string Hand::toString() const {

//  } //Get a string representation of the hand

