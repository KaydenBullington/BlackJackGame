# BlackJackGame
A Blackjack project for OOP (P01) that runs in the terminal. It has two views of the same game: an FTXUI terminal UI and a plain `cout`/`cin` console version. The game uses a fresh shuffled 52-card deck every round, the player starts with 100 chips, and a bet can be anywhere from 1 chip up to all of your chips.

## Game chosen
Blackjack (Option B), with Double Down and Split added for extra credit.

## How to build and run
From inside this folder (Windows, VS Code terminal):

```powershell
cmake -S . -B build -G Ninja
cmake --build build
.\build\game.exe           # FTXUI terminal UI
.\build\game.exe --text    # plain console version
.\build\game.exe --test    # rule tests (PASS/FAIL)
```

After the first time, only `cmake --build build` is needed. Re-run the first line if a new `.cpp` file is added.

## How to play
**TUI**
- Betting: Up/Down changes the bet by 1, Left/Right by 10, Enter deals
- Your turn: `h` hit, `s` stand, `d` double down, `p` split
- Round over: Enter for the next round
- Game over: `r` to play again
- `q` quits at any time

**Console (`--text`)**
- Type a bet, then `h`, `s`, `d`, or `p` and press Enter
- `q` quits

## Rules
- Deal order is player, dealer, player, dealer. The dealer's second card stays face down until the dealer plays.
- 2-10 are face value, J/Q/K are 10, an Ace is 11 unless that would bust the hand, then it's 1.
- A natural (Ace + 10-value card on the first two cards) pays 3:2. Both having a natural is a push. If only the dealer has one, the player loses. The round ends right after the deal in all of these cases.
- Going over 21 is a bust and an instant loss.
- The dealer hits on 16 or less and stands on 17 or more, including soft 17.
- When the dealer plays, the cards come out one at a time, half a second apart.
- A normal win pays 1:1, and a tie is a push.
- The game is over when the player has 0 chips.
- **Double Down:** only on the first two cards of a hand. The bet doubles, the player gets exactly one more card, and the hand is done.
- **Split:** only when the first two cards are the same rank, once per round. Each hand gets its own bet and is played one after the other.

## Design
- `Blackjack` holds all of the rules and state. It has no `cout`, `cin`, or FTXUI. An `enum class Phase` (Betting, PlayerTurn, DealerTurn, RoundOver, GameOver) controls what is allowed, and any call that isn't allowed returns `false` and changes nothing.
- `Hand` has a vector of `Card`s (composition) and knows its own value, soft, bust, and natural.
- `ConsoleView` and `TuiView` both inherit from `GameView`. `main()` holds a `std::unique_ptr<GameView>` and calls `view->run()` without knowing which view it has.
- Tests in `src/Tests.cpp` use stacked decks so every rule can be checked without luck.

## Screenshot
(TUI mid-game screenshot goes here)

## Known issues
None that I know of.

## Changes to Card or Deck
None.
