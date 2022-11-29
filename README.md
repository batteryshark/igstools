# PercussionMaster IGS Tools

## Patch Notes

This patch contains emulators for the IO controls and PCI card, among some optional quality-of-life patches for the game itself.

- If the SDL_VIDEODRIVER is set to 'dummy', the game will be patched to run without any display (for testing).

- If the game does not detect /dev/pccard0 on your device, it will assume that you need A27 ASIC emulation, and will provide that for you.

Envars:

PM_WINDOWED: Defined to enable windowed mode for the game.

PM_KEYIO: Defined to enable keyboard io in place of real IO controls. Supports an evdev or X11 backend.
Note: If a different event device is to be specified, use the additional PM_KEYIO_EVDEV to specify that path.

PM_A27LOG: Defined to create a logfile of A27 ASIC events. Useful for capturing real packets between the PCCARD and the game.

PM_AUTOPLAY:Defined to set the game to autoplay mode.

PM_DEVTEST:Defined replaces the Operator menu with a hidden Development Test menu that allows testing of various ingame screens, start any song/chart, record your own charts and play them, and some additional items.

PM_TRACKBALL:Defined replaces the Operator menu with a hidden trackball test menu that allows testing of the trackball IO.

PM_QCTEST: Defined replaces the Operator menu with an additional Quality Control test menu with some additional options such as an integrity test of game files.



