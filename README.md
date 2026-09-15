# Minesweeper

A Minesweeper clone written in C++20 with [raylib](https://www.raylib.com/), built as a single-file project to learn the library.

## Features

- Three difficulties: Beginner (9x9, 10 mines), Intermediate (16x16, 40 mines), Expert (30x16, 99 mines)
- First-click safety: mines are placed only after the first reveal, and never under or next to the clicked cell
- Recursive flood-fill reveal of empty regions
- Right-click flagging with a remaining-mine counter
- Timer (capped at 999s) and persistent best times per difficulty, saved to `minesweeper_times.txt`
- Menu screen showing best times; `R` restarts, `ESC` returns to the menu

## Build

Requires raylib. The included Visual Studio project targets toolset v143, C++20.

1. Place raylib headers in `include/` and the compiled raylib library in `lib/` next to the project file.
2. Open `minesweeper.slnx` and build the `Debug|x64` configuration — it is the only configuration wired to the `include`/`lib` paths. The `Release` and `Win32` configurations do not set raylib include/library paths; add them or install raylib into the default MSVC paths to use them.

On Linux with raylib installed system-wide:

```
g++ -std=c++20 minesweeper.cpp $(pkg-config --cflags --libs raylib) -o minesweeper
```

## Design notes

- The whole game lives in `minesweeper.cpp` (~600 lines): a `Minesweeper` class owns the grid, game state, and rendering; `main` handles the menu/game state switch and resizes the window to fit each difficulty's board.
- Deferring mine placement until the first click makes the first move always safe without a separate "regenerate until safe" loop.
- Best times are stored as three whitespace-separated integers in a plaintext file in the working directory. Fields that are missing, non-numeric, or outside 0–999 are treated as "no time recorded".

## Known limitations

- Flood-fill reveal is recursive; fine at these board sizes, but would need an explicit stack for much larger grids.
- No chord-click (revealing neighbors of a satisfied number) or question marks.

## License

MIT — see [LICENSE](LICENSE).
