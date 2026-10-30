//./game          -> runs the TuiView (FTXUI)
//./game --text   -> runs the ConsoleView (cout/cin)
//./game --test   -> runs the rule tests and prints PASS/FAIL

#include "Blackjack.hpp"
#include "ConsoleView.hpp"
#include "GameView.hpp"
#include "Tests.hpp"
#include "TuiView.hpp"

#include <memory>
#include <string>

int main(int argc, char **argv) {
    std::string mode = (argc > 1) ? argv[1] : "";
    if (mode == "--test") {
        return runTests();
    }

    Blackjack game;

    //Polymorphism: main() never knows which kind of view it has
    std::unique_ptr<GameView> view;
    if (mode == "--text") {
        view = std::make_unique<ConsoleView>(game);
    } else {
        view = std::make_unique<TuiView>(game);
    }

    view->run();
    return 0;
}
