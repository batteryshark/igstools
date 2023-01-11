# PercussionMaster Release Notes

After about 12 years of on-and-off work, getting a real PercussionMaster PC, and a ton of analysis. This game is at a point where it should be fully playable.

Requirements: Some form of x86/64 linux (ubuntu based, steamos, whatever).
Packages (based on 32bit Linux Mint, adjust for non-debian, 64bit etc...):
freeglut3 libsdl1.2debian osspd

1. Unzip the 'gamedata.zip' file - that will be your game root.
2. Unzip the 'libs.zip' file to your game root.
3. Unzip the 'patch.zip' file to your game root.
4. Unzip the 'songdata.zip' file to your game root.
5. Copy the contents in the "libs" subdirectory to "/usr/local/lib"  OR set LD_LIBRARY_PATH to the path of the "libs" subdirectory.
6. [Optional] Use "chmod 777 /dev/input/by-path/whatever_your_event_kbd_is"
7. Run the game with the patch preloaded (e.g. LD_PRELOAD=./pm_patch.so ./peng). Note: Some installations will not auto wrap OSS audio, in that case, also preload libpulsedsp.so.
8. Enjoy!

# Optional Environment Variables:

## Enables IO Keyboard Emulation (probably want this)
### Note: If you don't give your user read access to the evdev keyboard file, it will fallback to X11 input mode.
export PM_KEYIO=1

## Enables Windowed Mode 
export PM_WINDOWED=1

## Enables A27 packet logging (only useful on a real machine)
export PM_A27LOG=/path/to/logs

## Speeds up Intro Warning Screens
export PM_SKIPWARNING=1

## Enables Hidden Trackball Test Mode (replacing original test mode)
export PM_TRACKBALLTEST=1

## Enables Hidden DevTest Mode and translates Song Test Mode (replacing original test mode)
export PM_DEVTEST=1

## Enables Hidden QCTest Mode (replacing original test mode)
export PM_QCTEST=1

## Enables MonkeyInput for Testing 
export PM_AUTOPLAY=1

# Keyboard Bindings:

```
ESC Quit

[System]
1 Test 
2 Service Credit
3 Dev 1
4 Dev 2

[Player 1 -- Left]
S Blue
F Red
Z Rim [Left]
X Drum [Left]
C Drum [Right]
V Rim [Right]

[Player 2 -- Right]
H Blue
K Red
B Rim [Left]
N Drum [Left]
M Drum [Right]
, Rim [Right]
```









