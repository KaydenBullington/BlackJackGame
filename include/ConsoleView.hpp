#pragma once

#include "Blackjack.hpp"
#include "GameView.hpp"

#include <string>

//A VIEW: shows the game with cout and reads commands with getline
class ConsoleView : public GameView {
    private:
        Blackjack& game; //Uses the game, does not own it

        void showTable() const; //Print the dealer and all player hands
        void showResults() const; //Print how each hand finished
        static bool isNumber(const std::string& line); //True if the line is only digits

    public:
        explicit ConsoleView(Blackjack& game);
        void run() override;
};
