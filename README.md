# ⚔️⏸️ **EldenPause** _v1.1.3_
[![GitHub All Releases](https://img.shields.io/github/downloads/iArtorias/elden_pause/total.svg)](https://github.com/iArtorias/elden_pause/releases)

### A useful mod for Elden Ring allowing you to pause the game any time you want.

## Usage

1. Download the [latest release](https://github.com/iArtorias/elden_pause/releases/latest)
2. Copy the `dinput8.dll` and `elden_pause.ini` to your Elden Ring game folder
   * If you're using [Elden Ring Mod Loader](https://www.nexusmods.com/eldenring/mods/117) by [techiew](https://github.com/techiew/EldenRingModLoader), copy `dinput8.dll` and `elden_pause.ini` to the `mods` directory and rename the library to something like `elden_pause.dll`
4. Launch the game with EAC disabled
   * To do this, create the file `steam_appid.txt` with the text `1245620` in your game folder and run `eldenring.exe` directly
5. Press `P` button to pause the game. Press the same button again to resume the game.
6. If you're using controller, press `Start` button

## Remapping the keys

1. Open `elden_pause.ini`
2. If you want to change the keyboard key code, edit the `KeyboardButton` option value
3. If you want to change the controller key code, edit the `ControllerButton` option value
4. If you want to use a combo of two buttons, edit the `ControllerButton2` option value (`0xFFFF` by default aka `Disabled`)
   * `Optional:` you can change the `ControllerEnabled` option value to `false` if you don't need to use the controller code

## Advantages

- No DirectX hooking, just the native code usage
- A single library only
- Universal solution (unless the developers decide to patch it out)
- Easy to use

## Demo
- Visit the following [link](https://i.imgur.com/MeYPyva.mp4)

## NexusMods link
- [Find it here](https://www.nexusmods.com/eldenring/mods/231)

## Stuff used

- **Mem** (https://github.com/0x1F9F1/mem)
- **IniReader** (https://github.com/X-rays5/inireader)

## Credits

- `EMPRESS` for the help with locating a specific flag in memory

## Compilation notes

- **Visual Studio 2022** is recommended to compile this project
