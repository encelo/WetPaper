[![Linux](https://github.com/encelo/WetPaper/workflows/Linux/badge.svg)](https://github.com/encelo/WetPaper/actions?workflow=Linux)
[![macOS](https://github.com/encelo/WetPaper/workflows/macOS/badge.svg)](https://github.com/encelo/WetPaper/actions?workflow=macOS)
[![Windows](https://github.com/encelo/WetPaper/workflows/Windows/badge.svg)](https://github.com/encelo/WetPaper/actions?workflow=Windows)
[![MinGW](https://github.com/encelo/WetPaper/workflows/MinGW/badge.svg)](https://github.com/encelo/WetPaper/actions?workflow=MinGW)
[![Emscripten](https://github.com/encelo/WetPaper/workflows/Emscripten/badge.svg)](https://github.com/encelo/WetPaper/actions?workflow=Emscripten)
[![Android](https://github.com/encelo/WetPaper/workflows/Android/badge.svg)](https://github.com/encelo/WetPaper/actions?workflow=Android)
[![CodeQL](https://github.com/encelo/WetPaper/workflows/CodeQL/badge.svg)](https://github.com/encelo/WetPaper/actions?workflow=CodeQL)

# *WetPaper*
An arcade game made with the nCine for the Global Game Jam 2025 by the 3 a.m team with "Bubbles" as a theme.

The jam team was composed of Encelo and Cominu for programming, and awachirri88 and JustABallOfAnger for the art.

You can find the original version here: https://globalgamejam.org/games/2025/papel-mojado-9

## Changelog from the jam version

- Clean code (variable renaming, dead code removal, bug fixing)
- Add a `Config` namespace to remove magic numbers and strings
- Fill `NCPROJECT` variables
- Add a CTRL + H shortcut to show/hide the ImGui debug interface
- Add a game icon
- Add GitHub Actions YAML scripts

- Add support for window scaling factor
- Use the new `NCPROJECT_DEBUG` compiler definition
- Change the quit keyboard shortcut to CTRL + Q

- Use a `ncine::TextNode` object to write the game name on the title screen
- Add a splash screen showing the nCine logo fading in and out
- Add a menu system based on pages and events
- Add an input binder and actions system to rebind controls
