#pragma once

#include "Blackjack.hpp"
#include "GameView.hpp"

//A VIEW: shows the same game with FTXUI
class TuiView : public GameView {
    private:
        Blackjack& game; //Uses the game, does not own it

    public:
        explicit TuiView(Blackjack& game);
        void run() override;
};
