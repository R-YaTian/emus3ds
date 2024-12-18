## Emus for 3DS (PicoDrive Sega Master System / Mega Drive core)

This is a port of notaz's PicoDrive emulator to the old 3DS / 2DS. Although PicoDrive is already highly optimized for ARM processors, if ported as is, it still doesn't run full speed for all Mega Drive games on the old 3DS / 2DS, as it's evident in the RetroArch's version. So this port heavily relies on the 3DS's 2nd core to emulate the FM synthesized music for the YM2612 FM chip to achieve 60 FPS. But the 2nd core on the old 3DS is unfortunately not fast enough to generate the FM synthesized music at a full 44100Hz, so sounds are generated at 30000Hz on an Old 3DS. It sounds ok for many games, except for those that use high-pitched notes or sound samples.

The screen rendering is done completely using the original PicoDrive's ARM processor-optimized renderer. 

You can play Master System, Mega Drive games and Sega CD games, and 32X games. CD games run a little slower (you should enable 1-2 frameskips) on an Old 3DS, but it runs very well on a New 3DS. 32X games can only played at a reasonable speed on a New 3DS using the .CIA version of the emulator.

The default maps for the controls are: 
1. 3DS' Y Button -> MD's A Button, 
2. 3DS' B Button -> MD's B Button,
3. 3DS' A Button -> MD's C Button,
4. 3DS' X Button -> MD's X Button, 
5. 3DS' L Button -> MD's Y Button,
6. 3DS' R Button -> MD's Z Button

This emulator uses the same user interface as VirtuaNES for 3DS, TemperPCE for 3DS, Snes9x for 3DS. It will run better on the New 3DS as usual, where all music and sound samples will be generated at 44100Hz.

### Homebrew Launcher:

1. Copy picodrive_3ds.3dsx into the /3ds folder on your SD card.
2. Copy gbk.bin into the /emus3ds folder on your SD card.
3. Place your ROMs inside any folder.
4. Go to your Homebrew Launcher and launch the picodrive_3ds emulator.

### CIA Version:

1. Use your favorite CIA installer to install picodrive_3ds.cia into your CFW.
2. Copy gbk.bin into the /emus3ds folder on your SD card.
3. Place your ROMs inside any folder.
4. Exit your CIA installer and go to your CFW's home screen to launch the app.

### CD-ROM BIOS

1. If you have the CD ROM BIOS, place them in the /emus3ds/picodrive_3ds/bios folder.
2. They must be named:
   - bios_CD_U.bin or us_scd2_9306.bin or SegaCDBIOS9303.bin or us_scd1_9210.bin,
   - bios_CD_J.bin or jp_mcd2_921222.bin or jp_mcd1_9112.bin or jp_mcd1_9111.bin,
   - bios_CD_E.bin or eu_mcd2_9306.bin or eu_mcd2_9303.bin or eu_mcd1_9210.bin,  
   for the respective regions.
3. You can place all three BIOS in the /emus3ds/picodrive_3ds/bios folder.


### When in-game,

1. Tap the bottom screen for the menu.
2. Use Up/Down to choose option, and A to confirm. (Hold down X while pressing Up/Down to Page Up/Page Down)
3. Use Left/Right to change between ROM selection and emulator options.
4. You can quit the emulator to your homebrew launcher / your CFW's home screen.

-------------------------------------------------------------------------------------------------------

## PicoDrive Screenshots

![alt tag](https://github.com/R-YaTian/emus3ds/blob/master/screenshots/PicoDrive01.bmp)

![alt tag](https://github.com/R-YaTian/emus3ds/blob/master/screenshots/PicoDrive02.bmp)

![alt tag](https://github.com/R-YaTian/emus3ds/blob/master/screenshots/PicoDrive03.bmp)

![alt tag](https://github.com/R-YaTian/emus3ds/blob/master/screenshots/PicoDrive04.bmp)

![alt tag](https://github.com/R-YaTian/emus3ds/blob/master/screenshots/PicoDrive05.bmp)

![alt tag](https://github.com/R-YaTian/emus3ds/blob/master/screenshots/PicoDrive06.bmp)


-------------------------------------------------------------------------------------------

## Change History

### v0.96R
- Added an option to disable 3D Slider, also fix some issue related to 3D enabled
- Auto disable 3D Slider when emulator menu is opened
- Added support for game screen swap
- Added dark mode UI theme
- Added i18n language support, added English translations
- Major improve for sound and csnd, fix various sound issues
- Update lodepng lib to latest version
- Change home button to raise emulator menu instead of return to 3ds homemenu
- Use 3dszlib instead of build-in old version of zlib
- Further improvements to overall application stability and other minor adjustments have been made to enhance the user experience

### v0.95c
- Fully Simplified Chinese support, big thx to [xxxxst](https://github.com/xxxxst)
- Now will sort files by Chinese Pinyin
- UI: Improve line breaks, bug fix for "\n" as line breaks
- Implement standard/ssf2 mapper. This fix "Demons Of Asteborg", "Astebros" and etc
- Fix config saving
- Fix crashing when exit from CIA version
- Add embedded input redirect server to support 2 players, thx [CarlosEFML](https://github.com/CarlosEFML)
- Fix "Decap Attack"
- Cheats: Fix 8 bit writes for codes, thx [sleepingkirby](https://github.com/sleepingkirby)
- Implement sf001, sf002, sf004 and smw64 mapper from [libretro](https://github.com/libretro/picodrive)
- Update codes for lk3 mapper from [libretro](https://github.com/libretro/picodrive)
- Update buildin carthw.cfg for more games
- Add hack for unlicensed games that don't handle the Z80 bus properly, thx [techmetx11](https://github.com/techmetx11)

### v0.94
- Added support to save battery-backed RAM for CD games
- Fixed problems with games that show parts of the previous screens at the left/right edges
- Added support for cheats.
- Added configuration for region selection between (Default, US, Europe, Japan)

### v0.93
- Fixes a sound bug that plays the previous sound from a CD-ROM game when you load up an SMS ROM.
- Re-ordered region priority to US, JP, EU.
- Added support for .32x extensions and 32X games.
  (but some games like Virtual Fighter, Virtual Racing Deluxe cause the emulator to crash, just like the RetroArch versions)
- Fixed ASM version of the 32X rendering routines to prevent crashing, and Blackthorne games.
- Fixed the frame-rate bug that is not consistent with the frame-rate selected in the menu.
- Enabled 32X / SVP dynarec when running in CIA mode and the necessary custom firmware is available.
- Sets the default mapping for Sega MD's X, Y, Z buttons.
- Fixed a read-ahead library bug that previous caused small ISO games to boot to the CD player.
- Fixed minor sound emulation issues and improved sound sync.
- Fixed YM2612 timer bug.
- Implemented more aggressive optimzation of the YM2612 assembly emulation. Less skipping in some games on the Old 3DS.
- Fixed playing PWM samples by deducting the DC offset of the waveform (CSND is unable to reliably play samples with a significant DC offset)

### v0.92
- Added support for Mega CD games 
- CD reads are implemented with read-ahead for speed
- Other optimizations for CD games now allow Sonic CD's 2D levels, Final Fight CD and probably others to be playable on an Old 3DS with frame drops with smooth audio, if you can accept that. (FMV's still run slowly due to heavy processing)
- Optimizations
- Fixed PicoDrive crashing bug when loading any save state immediately when the game starts running
- Removed option for flickering sprites as it doesn't do what I thought it should do
- Clear Mega CD PCM buffers on reset
- Fixed random crashing bug when loading CD games due to uninitialized variables
- Added configurable option for 3- or 6-button controller type.
- Added some minor optimizations for the YM2612 FM synth (although it did nothing to improve performance and quality on an old 3DS)
- Fixed bug where the SRAM was previously never saved.
- Now defaults World region games to 60 FPS.
- Added option to force 60 FPS, 50 FPS and default frame rate.
- Added more screen stretching options and properly handles H32 and H40 width modes.
- Reduced lag in the music and sound playback.
- Used the assembly version of the 32X renderer (untested).

### v0.91
- Added support for more extensions (.smd, .gen, .bin, .rom).
- Added option to apply a low pass filter to the audio.

### v0.90
- First release.

-------------------------------------------------------------------------------------------------------

## .CHX Cheat File format

The .CHX is a cheat file format that you can create with any text editor. Each line in the file corresponds to one cheat, and is of the following format:

     [Y/N],[CheatCode],[Name]

1. [Y/N] represents whether the cheat is enabled. Whenever you enable/disable it in the emulator, the .CHX cheat file will be modified to save your changes.
2. [CheatCode] must be an Genesis / Mega Drive Game Genie or Pro Action Replay cheat code. The cheat code looks like one of the following: 
- **RHVA-A6WR** (Game Genie)
- **FFFE0D:0099** (Pro Action Replay)
- **FFB933:00** (Pro Action Replay)
- **FFB9 3300** (Pro Action Replay)
3. [Name] is a short name that represents this cheat. Since this will appear in the emulator, keep it short (< 30 characters). 

The .CHX must have the same name as your ROM. This is an example of a .CHX file:

Filename: **Contra - Hard Corps (USA).chx**
```
N,RHVA-A6WR,Invincible
N,NNCT-AAH4,Start with 99 lives
N,H9GA-AAE6,Start with all weapons
N,AM4A-AA8C,You don't lose a weapon when you die 
```

NOTE: Some games may not boot when cheat codes are enabled at the start. Disable the cheat codes when booting the games, and enable them only after the game has started.

-------------------------------------------------------------------------------------------------------

## How to Build

You will need latest:
- devkitARM
- libctru
- citro3d

Then build by using *make -f picodrive-make*.

-------------------------------------------------------------------------------------------------------

## Credits

1. Notaz for his well-optimized PicoDrive emulator
2. Authors of the Citra 3DS Emulator team. Without them, this project would have been extremely difficult.
3. Fellow forummers on GBATemp for the bug reports and suggestions for improvements.
