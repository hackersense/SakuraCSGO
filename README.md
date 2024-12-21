# Sakura V2
[![C++](https://img.shields.io/badge/language-C%2B%2B-%23f34b7d.svg?style=plastic)](https://en.wikipedia.org/wiki/C%2B%2B) 
[![CS:GO](https://img.shields.io/badge/game-CS%3AGO-yellow.svg?style=plastic)](https://store.steampowered.com/app/730/CounterStrike_Global_Offensive/) 
[![Windows](https://img.shields.io/badge/platform-Windows-0078d7.svg?style=plastic)](https://en.wikipedia.org/wiki/Microsoft_Windows) 
[![x86](https://img.shields.io/badge/arch-x86-red.svg?style=plastic)](https://en.wikipedia.org/wiki/X86)


Private cheat software for **Counter-Strike: Global Offensive** game. Designed as an internal cheat - [Dynamic-link library](https://en.wikipedia.org/wiki/Dynamic-link_library) (DLL) loadable into game process. Compatible with the Steam version of the game. Available for Windows systems.

## Getting started

### Prerequisites
CLion, Visual Studio or other C++ compiler, platform toolset v143 and Windows SDK 10.0.x.x are required in order to compile Sakura. If you don't have ones, you can download VS [here](https://visualstudio.microsoft.com/) (Windows SDK is installed during Visual Studio Installer).

### Downloading

There are two options of downloading the source code:

#### Without [git](https://git-scm.com)

Choose this option if you want pure source and you're not going to contribute to the repo.

To download source code this way [CLICK ME](https://github.com/hackersense/SakuraCSGO/archive/refs/heads/main.zip).

#### With [git](https://git-scm.com)

Choose this option if you're going to contribute to the repo or you want to use version control system. Git is required to step further, if not installed download it [here](https://git-scm.com).

Open git command prompt and enter following command:

    git clone --depth=1 https://github.com/hackersense/SakuraCSGO.git

`SakuraCSGO` folder should have been successfully created, containing all the source files.

### Compiling from source

When you have equipped a copy of the source code, next step is import **CMakeLists.txt** from CLion.

Then change cmake profile to `Release` and simply press **Build 'Sakura'**.

If everything went right you should receive `Sakura.dll`  binary file.

### Loading / Injecting into game process

Open your favorite [DLL injector](https://en.wikipedia.org/wiki/DLL_injection) and just inject `Sakura.dll` into `csgo.exe` process.

When injected, menu is openable under `HOME` key.

## FAQ

### How do I open menu?
Press <kbd>HOME</kbd> while focused on CS:GO window.

### Where is my config file saved?
Configuration files are saved inside `Sakura` folder in your `CSGO` folder (`%STEAMLIBRARY%\steamapps\common\Counter-Strike Global Offensive`).

### What hooking methods Sakura uses?
Currently implemented hooking methods are:
*   MinHook - trampoline hook
*   VmtHook - hook a function directly in a vtable

Hooking implementation files are located in [Hooks](https://github.com/hackersense/SakuraCSGO/tree/main/Source/hooks/core) directory.

## Acknowledgments

*   [ocornut](https://github.com/ocornut) and [contributors](https://github.com/ocornut/imgui/graphs/contributors) for creating and maintaining an amazing GUI library - [Dear imgui](https://github.com/ocornut/imgui).

## Credits

[Unknown Cheats Forum](https://www.unknowncheats.me/forum/counterstrike-global-offensive/)

## License

> Copyright (c) 2015-2025 HackerSense Team.
