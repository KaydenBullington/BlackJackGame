#pragma once

#include "Card.hpp"
#include "Deck.hpp"
#include "Hand.hpp"

#include <string>
#include <vector>

//What the player is allowed to do right now
enum class Phase { Betting, PlayerTurn, DealerTurn, RoundOver, GameOver };

//How one of the player's hands finished
enum class Outcome { None, PlayerBlackjack, PlayerWin, DealerWin, Push, PlayerBust, DealerBust };

//THE GAME: rules and state only, no cout/cin and no FTXUI
class Blackjack {
    private:
        int startingChips; //Chips the player starts with (used by restart)
        int chips; //Player's chips
        int currentHand; //Index of the current hand being played
        bool stacked; //True when a test gave us a stacked deck (don't reshuffle it)
        Phase phase; //What the player can do right now
        Deck deck; //Deck of cards
        Hand dealerHand; //Dealer's hand
        std::vector<Hand> playerHands; //Player's hands (for splits)
        std::vector<int> bets; //Bets for each hand (for splits)
        std::vector<Outcome> outcomes; //Result for each hand (for splits)
        int payout; //Chips paid back to the player this round

        bool anyHandLeft() const; //True if at least one player hand didn't bust
        void nextHand(); //Move to the next hand, or to the dealer's turn after the last one
        void finishRound(); //Every hand gets settled and the round is over
        void settleHand(int i); //Decide the outcome of one hand and pay it

    public:
        Blackjack(int startingChips = 100); //Real games: shuffled deck
        Blackjack(const Deck& stackedDeck, int startingChips = 100); //Tests: stacked deck

        //-------------------- Actions --------------------
        //Each one returns false (and changes nothing) if it isn't allowed right now
        bool startNewGame(int bet); //Betting -> PlayerTurn (or RoundOver on a natural)
        bool playerHit(); //Player draws a card
        bool playerStand(); //Player is done with the current hand
        bool playerDoubleDown(); //Double the bet, draw one card, hand is done
        bool playerSplit(); //Turn a pair into two hands
        bool dealerStep(); //Dealer makes ONE move (draw a card or finish), so views can show it slowly
        bool nextRound(); //RoundOver -> Betting (or GameOver)
        bool restart(); //GameOver -> Betting with fresh chips

        //-------------------- Questions --------------------
        Phase getPhase() const; //Current phase
        int getChips() const; //Chips the player has
        bool isValidBet(int bet) const; //Is this bet allowed?
        bool canSplit() const; //Can the current hand be split?
        bool canDoubleDown() const; //Can the current hand double down?
        bool dealerHoleHidden() const; //Is the dealer's second card face down?

        int getHandCount() const; //How many hands the player has (2 after a split)
        int getCurrentHand() const; //Index of the hand being played
        const Hand& getPlayerHand(int handIndex) const; //One of the player's hands
        const Hand& getDealerHand() const; //Dealer's hand
        int getBet(int handIndex) const; //Bet on one hand
        Outcome getOutcome(int handIndex) const; //Result of one hand
        int getRoundNet() const; //Chips won (+) or lost (-) this round

        bool isPlayerBust(int handIndex) const; //Check if the player is bust
        bool isDealerBust() const; //Check if the dealer is bust
        int getPlayerValue(int handIndex) const; //Get the player's hand value
        int getDealerValue() const; //Get the dealer's hand value
        std::string getPlayerHandString(int handIndex) const; //Player's hand as text
        std::string getDealerHandString() const; //Dealer's hand as text ("??" for the hole card)

        static std::string outcomeName(Outcome o); //"Blackjack!", "You win", ...
};
