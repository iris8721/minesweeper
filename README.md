# Minesweeper

A Minesweeper clone written in C++20 with [raylib](https://www.raylib.com/), built as a single-file project to learn the library.

## Features

- Three difficulties: Beginner (9x9, 10 mines), Intermediate (16x16, 40 mines), Expert (30x16, 99 mines)
- First-click safety: mines are placed only after the first reveal, and never under the clicked cell
- Recursive flood-fill reveal of empty regions
- Right-click flagging with a remaining-mine counter
- Timer (capped at 999s) and persistent best times per difficulty, saved to `minesweeper_times.txt`
- Menu screen showing best times; `R` restarts, `ESC` returns to the menu

## Build

Requires Visual Studio (project targets toolset v143, C++20) and raylib.

1. Place raylib headers in `include/` and the compiled raylib library in `lib/` next to the project file.
2. Open `minesweeper.slnx` and build the `Debug|x64` configuration — it is the only configuration wired to the `include`/`lib` paths.

Note: the `Release` and `Win32` configurations do not set raylib include/library paths; add them or install raylib into the default MSVC paths to use them. Build unverified on this machine (no Windows toolchain available).

## Design notes

- The whole game lives in `minesweeper.cpp` (~600 lines): a `Minesweeper` class owns the grid, game state, and rendering; `main` handles the menu/game state switch and resizes the window to fit each difficulty's board.
- Deferring mine placement until the first click makes the first move always safe without a separate "regenerate until safe" loop.
- Best times are stored as three whitespace-separated integers in a plaintext file next to the executable.

## Known limitations

- Flood-fill reveal is recursive; fine at these board sizes, but would need an explicit stack for much larger grids.
- The best-times file is unvalidated plaintext — a corrupt file falls back to defaults only if it fails to open, not if it contains garbage.
- No chord-click (revealing neighbors of a satisfied number) or question marks.

## License

MIT — see [LICENSE](LICENSE).
