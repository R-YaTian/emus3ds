#include <algorithm>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <vector>

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <3ds.h>

#include <dirent.h>

#include "3dstypes.h"
#include "3dsexit.h"
#include "3dsgpu.h"
#include "3dsopt.h"
#include "3dssound.h"
#include "3dsmenu.h"
#include "3dsui.h"
#include "3dsfont.h"
#include "3dsgbk.h"
#include "3dsconfig.h"
#include "3dsfiles.h"
#include "3dsinput.h"
#include "3dsmenu.h"
#include "3dsmain.h"
#include "3dsdbg.h"
#include "3dsinterface.h"
#include "3dscheat.h"

#include "lodepng.h"

SEmulator emulator;
ScreenSettings screenSettings;

int frameCount60 = 60;
u64 frameCountTick = 0;
int framesSkippedCount = 0;
const char *romFileName = 0;
char romFileNameFullPath[_MAX_PATH];
char romFileNameLastSelected[_MAX_PATH];

//-------------------------------------------------------
// Clear screen with logo.
//-------------------------------------------------------
void clearScreenWithLogo()
{
	unsigned char* image;
	unsigned width, height;

    int widthAdjust = screenSettings.GameScreen == GFX_TOP ? 0 : 40;
    int error = lodepng_decode32_file(&image, &width, &height, impl3dsTitleImage);

    if (!error && width == 400 && height == 240)
    {
        // lodepng outputs big endian rgba so we need to convert
        for (int i = 0; i < 2; i++)
        {
            u8* src = image;
            uint32* fb = (uint32 *) gfxGetFramebuffer(screenSettings.GameScreen, GFX_LEFT, NULL, NULL);
            for (int y = 0; y < 240; y++)
            {
                for (int x = 0; x < 400; x++)
                {
                    uint32 r = *src++;
                    uint32 g = *src++;
                    uint32 b = *src++;
                    uint32 a = *src++;

                    if (x < widthAdjust || x >= (screenSettings.GameScreenWidth + widthAdjust))
                        continue;

                    uint32 c = ((r << 24) | (g << 16) | (b << 8) | 0xff);
                    fb[(x - widthAdjust) * 240 + (239 - y)] = c;
                }
            }
            gfxScreenSwapBuffers(screenSettings.GameScreen, false);
        }

        free(image);
    }
}


//-------------------------------------------------------
// Clear target screen
//-------------------------------------------------------
void clearScreen(gfxScreen_t targetScreen) {
    uint bytes = 0;
    switch (gfxGetScreenFormat(targetScreen))
    {
        case GSP_RGBA8_OES:
            bytes = 4;
            break;

        case GSP_BGR8_OES:
            bytes = 3;
            break;

        case GSP_RGB565_OES:
        case GSP_RGB5_A1_OES:
        case GSP_RGBA4_OES:
            bytes = 2;
            break;
    }

    u8 *frame = gfxGetFramebuffer(targetScreen, GFX_LEFT, NULL, NULL);
    memset(frame, 0, (targetScreen == GFX_TOP ? 400 : 320) * 240 * bytes);
}


//-------------------------------------------------------
// Do swap screen transition
//-------------------------------------------------------
void swapScreenTransition()
{
    gfxSetScreenFormat(screenSettings.SecondScreen, GSP_RGB565_OES);
    gfxSetScreenFormat(screenSettings.GameScreen, GSP_RGBA8_OES);
    //clearScreen(screenSettings.GameScreen);
    gspWaitForVBlank();
}


//----------------------------------------------------------------------
// Start up menu.
//----------------------------------------------------------------------
SMenuItem emulatorNewMenu[] = {
    MENU_MAKE_PICKER(24000, getText("  语言"), getText("选择应用程序显示的语言"), &optionsForLanguage, DIALOG_TYPE_INFO),
    MENU_MAKE_PICKER(23000, getText("  主题"), getText("选择应用于界面的主题"), &optionsForTheme, DIALOG_TYPE_INFO),
    MENU_MAKE_PICKER(18000, getText("  字体"), getText("用于用户界面的字体(仅适用于ASCII字符)"), &optionsForFont, DIALOG_TYPE_INFO),
    MENU_MAKE_ACTION(6001,  getText("  退出")),
    MENU_MAKE_LASTITEM  ()
};

extern SMenuItem emulatorMenu[];


//-------------------------------------------------------
// Sets up all the cheats to be displayed in the menu.
//-------------------------------------------------------
SMenuItem cheatMenu[MAX_CHEATS + 1] =
{
    MENU_MAKE_HEADER2   (getText("金手指")),
    MENU_MAKE_LASTITEM  ()
};


//-------------------------------------------------------
// Load the ROM and reset the CPU.
//-------------------------------------------------------

bool emulatorSettingsLoad(bool, bool, bool);
bool emulatorSettingsSave(bool, bool, bool);

bool emulatorLoadRom()
{
    impl3dsClearAllCheats();

    menu3dsShowDialog(getText("加载 ROM"), getText("加载中... 请稍后."), Themes[settings3DS.Theme].dialogColorInfo, NULL);

    char romFileNameFullPathOriginal[_MAX_PATH];
    strncpy(romFileNameFullPathOriginal, romFileNameFullPath, _MAX_PATH - 1);

    //emulatorSettingsSave(true, true, false);
    snprintf(romFileNameFullPath, _MAX_PATH, "%s%s", file3dsGetCurrentDir(), romFileName);

    char romFileNameFullPath2[_MAX_PATH];
    strncpy(romFileNameFullPath2, romFileNameFullPath, _MAX_PATH - 1);

    // Load up the new ROM settings first.
    //
    emulatorSettingsLoad(false, true, false);
    impl3dsApplyAllSettings();

    if (!impl3dsLoadROM(romFileNameFullPath2))
    {
        // If the ROM loading fails:
        // 1. Restore the original ROM file path.
        strncpy(romFileNameFullPath, romFileNameFullPathOriginal, _MAX_PATH - 1);

        // 2. Reload original settings
        //emulatorSettingsLoad(false, true, false);
        impl3dsApplyAllSettings();

        menu3dsHideDialog();

        return false;
    }
    impl3dsApplyAllSettings();

    if (settings3DS.AutoSavestate)
        impl3dsLoadState(0);

    emulator.emulatorState = EMUSTATE_EMULATE;
    aptCheckHomePressRejected();

    cheat3dsLoadCheatTextFile(file3dsReplaceFilenameExtension(romFileNameFullPath, ".chx"));
    menu3dsHideDialog();

    // Fix: Game-specific settings that never get saved.
    impl3dsCopyMenuToOrFromSettings(false);

    return true;
}


//----------------------------------------------------------------------
// Menus
//----------------------------------------------------------------------
#define MAX_FILES 1000
SMenuItem fileMenu[MAX_FILES + 1];
//char romFileNames[MAX_FILES][_MAX_PATH];
// By changing the romFileNames to fileList (strings allocated on demand)
// this fixes the crashing problem on Old 3DS when running Luma 8 and Rosalina 2.
//
std::vector<std::string> fileList;

int totalRomFileCount = 0;

//----------------------------------------------------------------------
// Load all ROM file names (up to 1000 ROMs)
//----------------------------------------------------------------------
void fileGetAllFiles(void)
{
    fileList = file3dsGetFiles(impl3dsRomExtensions, MAX_FILES);

    totalRomFileCount = 0;

    // Increase the total number of files we can display.
    for (int i = 0; i < fileList.size() && i < MAX_FILES; i++)
    {
        //strncpy(romFileNames[i], fileList[i].c_str(), _MAX_PATH);
        totalRomFileCount++;
        fileMenu[i].Type = MENUITEM_ACTION;
        fileMenu[i].ID = i;
        fileMenu[i].Text = fileList[i].c_str();
    }
    fileMenu[totalRomFileCount].Type = MENUITEM_LASTITEM;
}


//----------------------------------------------------------------------
// Find the ID of the last selected file in the file list.
//----------------------------------------------------------------------
int fileFindLastSelectedFile()
{
    for (int i = 0; i < totalRomFileCount && i < MAX_FILES; i++)
    {
        if (strncmp(fileMenu[i].Text, romFileNameLastSelected, _MAX_PATH) == 0)
            return i;
    }
    return -1;
}


//----------------------------------------------------------------------
// Load global settings, and game-specific settings.
//----------------------------------------------------------------------
bool emulatorSettingsLoad(bool includeGlobalSettings, bool includeGameSettings, bool showMessage = true)
{
    if (includeGlobalSettings)
    {
        bool success = impl3dsReadWriteSettingsGlobal(false);
        if (success)
        {
            input3dsSetDefaultButtonMappings(settings3DS.GlobalButtonMapping, settings3DS.GlobalTurbo, false);
            impl3dsApplyAllSettings(false);
        }
        else
        {
            impl3dsInitializeDefaultSettingsGlobal();
            input3dsSetDefaultButtonMappings(settings3DS.GlobalButtonMapping, settings3DS.GlobalTurbo, true);
            impl3dsApplyAllSettings(false);
            return false;
        }
    }

    if (includeGameSettings)
    {
        bool success = impl3dsReadWriteSettingsByGame(false);
        if (success)
        {
            input3dsSetDefaultButtonMappings(settings3DS.ButtonMapping, settings3DS.Turbo, false);
            impl3dsApplyAllSettings();
            return true;
        }
        else
        {
            impl3dsInitializeDefaultSettingsByGame();
            input3dsSetDefaultButtonMappings(settings3DS.ButtonMapping, settings3DS.Turbo, true);
            impl3dsApplyAllSettings();

            //return emulatorSettingsSave(true, showMessage);
            return true;
        }
    }
    return true;
}


//----------------------------------------------------------------------
// Save global settings, and game-specific settings.
//----------------------------------------------------------------------
bool emulatorSettingsSave(bool includeGlobalSettings, bool includeGameSettings, bool showMessage)
{
    int widthAdjust = screenSettings.GameScreen == GFX_TOP ? 0 : 40;

    if (showMessage)
    {
        consoleClear();
        ui3dsDrawRect(50 + widthAdjust, 140, 270 + widthAdjust, 154, 0x000000);
        ui3dsDrawStringWithNoWrapping(50 + widthAdjust, 140, 270 + widthAdjust, 154, 0x3f7fff, HALIGN_CENTER, getText("保存设置到SD卡..."));
    }

    if (includeGameSettings)
    {
        impl3dsReadWriteSettingsByGame(true);
    }

    if (includeGlobalSettings)
    {
        impl3dsReadWriteSettingsGlobal(true);
    }

    if (showMessage)
    {
        ui3dsDrawRect(50 + widthAdjust, 140, 270 + widthAdjust, 154, 0x000000);
    }

    return true;
}


//----------------------------------------------------------------------
// Start up menu.
//----------------------------------------------------------------------
void menuSelectFile(void)
{
    gfxSetDoubleBuffering(screenSettings.SecondScreen, true);

    fileGetAllFiles();
    int previousFileID = fileFindLastSelectedFile();
    menu3dsClearMenuTabs();
    menu3dsAddTab(getText("选项"), emulatorNewMenu);
    menu3dsAddTab(getText("选择 ROM"), fileMenu);
    menu3dsSetTabSubTitle(0, NULL);
    menu3dsSetTabSubTitle(1, file3dsGetCurrentDir());
    menu3dsSetCurrentMenuTab(1);
    menu3dsSetValueByID(0, 18000, settings3DS.Font);
    menu3dsSetValueByID(0, 23000, settings3DS.Theme);
    menu3dsSetValueByID(0, 24000, settings3DS.Language);
    if (previousFileID >= 0)
        menu3dsSetSelectedItemIndexByID(1, previousFileID);
    menu3dsSetTransferGameScreen(false);

    bool animateMenu = true;
    int selection = 0;
    do
    {
        if (appExiting || !aptMainLoop())
            return;

        selection = menu3dsShowMenu(NULL, animateMenu);
        animateMenu = false;

        if (selection >= 0 && selection < 1000)
        {
            // Load ROM
            //
            //romFileName = romFileNames[selection];
            romFileName = fileList[selection].c_str();
            strncpy(romFileNameLastSelected, romFileName, _MAX_PATH);
            if (romFileName[0] == 1)
            {
                if (strcmp(romFileName, "\x01 ..") == 0)
                    file3dsGoToParentDirectory();
                else
                    file3dsGoToChildDirectory(&romFileName[2]);

                fileGetAllFiles();
                menu3dsClearMenuTabs();
                menu3dsAddTab(getText("选项"), emulatorNewMenu);
                menu3dsAddTab(getText("选择 ROM"), fileMenu);
                menu3dsSetCurrentMenuTab(1);
                menu3dsSetTabSubTitle(1, file3dsGetCurrentDir());
                selection = -1;
            }
            else
            {
                if (!emulatorLoadRom())
                {
                    menu3dsShowDialog(getText("加载 ROM"), getText("无法加载 ROM."), Themes[settings3DS.Theme].dialogColorWarn, optionsForOk);
                    menu3dsHideDialog();
                }
                else
                {
                    menu3dsHideMenu();
                    consoleInit(screenSettings.SecondScreen, NULL);
                    consoleClear();
                    return;
                }
            }
        }
        else if (selection == 6001)
        {
            int result = menu3dsShowDialog(getText("退出"), getText("立即退出?"), Themes[settings3DS.Theme].dialogColorWarn, optionsForNoYes);
            menu3dsHideDialog();

            if (result == 1)
            {
                emulator.emulatorState = EMUSTATE_END;
                return;
            }
        }

        selection = -1;     // Bug fix: Fixes crashing when setting options before any ROMs are loaded.
    }
    while (selection == -1);

    menu3dsHideMenu();
}


//----------------------------------------------------------------------
// Checks if file exists.
//----------------------------------------------------------------------
bool IsFileExists(const char * filename) {
    if (FILE * file = fopen(filename, "r")) {
        fclose(file);
        return true;
    }
    return false;
}


//----------------------------------------------------------------------
// Menu when the emulator is paused in-game.
//----------------------------------------------------------------------
bool menuSelectedChanged(int ID, int value)
{
    if (ID >= 50000 && ID <= 51000)
    {
        // Handle cheats
        int enabled = value;
        impl3dsSetCheatEnabledFlag(ID - 50000, enabled == 1);
        cheat3dsSetCheatEnabledFlag(ID - 50000, enabled == 1);
        return false;
    }

    return impl3dsOnMenuSelectedChanged(ID, value);
}


//----------------------------------------------------------------------
// Menu when the emulator is paused in-game.
//----------------------------------------------------------------------
void menuPause()
{
    gfxSetDoubleBuffering(screenSettings.SecondScreen, true);

    bool settingsUpdated = false;
    bool cheatsUpdated = false;
    bool settingsSaved = false;
    bool returnToEmulation = false;

    menu3dsClearMenuTabs();
    menu3dsAddTab(getText("模拟器"), emulatorMenu);
    menu3dsAddTab(getText("设置"), optionMenu);
    menu3dsAddTab(getText("控制"), controlsMenu);
    menu3dsAddTab(getText("金手指"), cheatMenu);
    menu3dsAddTab(getText("选择ROM"), fileMenu);

    impl3dsCopyMenuToOrFromSettings(false);

    int previousFileID = fileFindLastSelectedFile();
    menu3dsSetTabSubTitle(0, NULL);
    menu3dsSetTabSubTitle(1, NULL);
    menu3dsSetTabSubTitle(2, NULL);
    menu3dsSetTabSubTitle(3, NULL);
    menu3dsSetTabSubTitle(4, file3dsGetCurrentDir());
    if (previousFileID >= 0)
        menu3dsSetSelectedItemIndexByID(4, previousFileID);
    menu3dsSetCurrentMenuTab(0);
    menu3dsSetTransferGameScreen(true);

    bool animateMenu = true;

    while (aptMainLoop() && !appExiting)
    {
        int selection = menu3dsShowMenu(menuSelectedChanged, animateMenu);
        animateMenu = false;

        if (selection == -1 || selection == 1000)
        {
            // Cancels the menu and resumes game
            //
            returnToEmulation = true;

            break;
        }
        else if (selection < 1000)
        {
            // Load ROM
            //
            //romFileName = romFileNames[selection];
            romFileName = fileList[selection].c_str();
            if (romFileName[0] == 1)
            {
                if (strcmp(romFileName, "\x01 ..") == 0)
                    file3dsGoToParentDirectory();
                else
                    file3dsGoToChildDirectory(&romFileName[2]);

                fileGetAllFiles();
                menu3dsClearMenuTabs();
                menu3dsAddTab(getText("模拟器"), emulatorMenu);
                menu3dsAddTab(getText("设置"), optionMenu);
                menu3dsAddTab(getText("控制"), controlsMenu);
                menu3dsAddTab(getText("金手指"), cheatMenu);
                menu3dsAddTab(getText("选择ROM"), fileMenu);
                menu3dsSetCurrentMenuTab(4);
                menu3dsSetTabSubTitle(4, file3dsGetCurrentDir());
            }
            else
            {
                strncpy(romFileNameLastSelected, romFileName, _MAX_PATH);

                bool loadRom = true;
                if (settings3DS.AutoSavestate) {
                    menu3dsShowDialog(getText("即时存档"), getText("自动保存..."), Themes[settings3DS.Theme].dialogColorWarn, NULL);
                    bool result = impl3dsSaveState(0);
                    menu3dsHideDialog();

                    if (!result) {
                        int choice = menu3dsShowDialog(getText("自动保存失败"), getText("自动保存失败.\n强制加载?"), Themes[settings3DS.Theme].dialogColorWarn, optionsForNoYes);
                        if (choice != 1) {
                            loadRom = false;
                        }
                    }
                }

                if (loadRom)
                {
                    // Save settings and cheats, before loading
                    // your new ROM.
                    //
                    if (impl3dsCopyMenuToOrFromSettings(true))
                    {
                        emulatorSettingsSave(true, true, false);
                    }
                    else
                    {
                        emulatorSettingsSave(true, false, false);
                    }
                    settingsSaved = true;

                    if (!emulatorLoadRom())
                    {
                        menu3dsShowDialog(getText("选择ROM"), getText("无法加载 ROM."), Themes[settings3DS.Theme].dialogColorWarn, optionsForOk);
                        menu3dsHideDialog();
                    }
                    else
                        break;
                }
            }
        }
        else if (selection >= 2001 && selection <= 2010)
        {
            int slot = selection - 2000;
            char text[200];
#define getText getTextFromMap
            sprintf(text, getText("保存到存档位 %d...\n请稍后"), slot);
            menu3dsShowDialog(getText("即时存档"), text, Themes[settings3DS.Theme].dialogColorInfo, NULL, -1, true);
            bool result = impl3dsSaveState(slot);
            menu3dsHideDialog();

            if (result)
            {
                sprintf(text, getText("存档位 %d 保存完成."), slot);
                result = menu3dsShowDialog(getText("即时存档"), text, Themes[settings3DS.Theme].dialogColorSuccess, optionsForOk, -1, true);
                menu3dsHideDialog();
            }
            else
            {
                sprintf(text, getText("无法保存到存档位 %d!"), slot);
                result = menu3dsShowDialog(getText("即时存档"), text, Themes[settings3DS.Theme].dialogColorWarn, optionsForOk, -1, true);
                menu3dsHideDialog();
            }

            menu3dsSetSelectedItemIndexByID(0, 1000);
        }
        else if (selection >= 3001 && selection <= 3010)
        {
            int slot = selection - 3000;
            char text[200];

            bool result = impl3dsLoadState(slot);
            if (result)
            {
                emulator.emulatorState = EMUSTATE_EMULATE;
                consoleClear();
                aptCheckHomePressRejected();
                break;
            }
            else
            {
                sprintf(text, getText("无法加载存档位 %d!"), slot);
                menu3dsShowDialog(getText("即时存档"), text, Themes[settings3DS.Theme].dialogColorWarn, optionsForOk, -1, true);
#undef getText
                menu3dsHideDialog();
            }
        }
        else if (selection == 4001)
        {
            menu3dsShowDialog(getText("截屏"), getText("开始截屏...\n请稍后."), Themes[settings3DS.Theme].dialogColorInfo, NULL);

            char ext[256];
            const char *path = NULL;

            // Loop through and look for an non-existing
            // file name.
            //
            int i = 1;
            while (i <= 999)
            {
                snprintf(ext, 255, ".b%03d.bmp", i);
                path = file3dsReplaceFilenameExtension(romFileNameFullPath, ext);
                if (!IsFileExists(path))
                    break;
                path = NULL;
                i++;
            }

            bool success = false;
            if (path)
            {
                success = menu3dsTakeScreenshot(path);
            }
            menu3dsHideDialog();

            if (success)
            {
                char text[600];
#define getText getTextFromMap
                snprintf(text, 600, getText("完成! 文件已保存到 %s"), path);
                menu3dsShowDialog(getText("截屏"), text, Themes[settings3DS.Theme].dialogColorSuccess, optionsForOk, -1, true);
#undef getText
                menu3dsHideDialog();
            }
            else
            {
                menu3dsShowDialog(getText("截屏"), getText("截屏时发生错误!"), Themes[settings3DS.Theme].dialogColorWarn, optionsForOk);
                menu3dsHideDialog();
            }
        }
        else if (selection == 5001)
        {
            int result = menu3dsShowDialog(getText("重置控制台"), getText("确定吗?"), Themes[settings3DS.Theme].dialogColorWarn, optionsForNoYes);
            menu3dsHideDialog();

            if (result == 1)
            {
                impl3dsResetConsole();
                emulator.emulatorState = EMUSTATE_EMULATE;
                consoleClear();
                aptCheckHomePressRejected();
                break;
            }
        }
        else if (selection == 6001)
        {
            int result = menu3dsShowDialog(getText("退出"),  getText("立即退出?"), Themes[settings3DS.Theme].dialogColorWarn, optionsForNoYes);
            if (result == 1)
            {
                emulator.emulatorState = EMUSTATE_END;
                break;
            }
            else
                menu3dsHideDialog();

        }
        else
        {
            bool endMenu = impl3dsOnMenuSelected(selection);
            if (endMenu)
            {
                returnToEmulation = true;
                break;
            }
        }
    }

    menu3dsHideMenu();

    // Save settings and cheats
    //
    if (!settingsSaved && impl3dsCopyMenuToOrFromSettings(true))
    {
        emulatorSettingsSave(true, true, true);
    }
    impl3dsApplyAllSettings();

    if (emulator.emulatorState != EMUSTATE_END && settings3DS.GameScreen != (int) screenSettings.GameScreen)
    {
        menu3dsDrawBlackScreen();
        ui3dsUpdateScreenSettings((gfxScreen_t) settings3DS.GameScreen);
        menu3dsDrawBlackScreen();
        swapScreenTransition();
    }

    cheat3dsSaveCheatTextFile(file3dsReplaceFilenameExtension(romFileNameFullPath, ".chx"));

    if (returnToEmulation)
    {
        emulator.emulatorState = EMUSTATE_EMULATE;
        consoleClear();
        aptCheckHomePressRejected();
    }
}


//--------------------------------------------------------
// Initialize the emulator engine and everything else.
// This calls the impl3dsInitializeCore, which executes
// initialization code specific to the emulation core.
//--------------------------------------------------------
void emulatorInitialize()
{
    emulator.enableDebug = false;
    emulator.emulatorState = 0;
    emulator.waitBehavior = 0;

    file3dsInitialize();

    romFileNameLastSelected[0] = 0;

    if (!gpu3dsInitialize())
    {
        printf ("Unable to initialize GPU\n");
        exit(0);
    }

    printf ("Initializing...\n");

    if (!impl3dsInitializeCore())
    {
        printf ("Unable to initialize emulator core\n");
        exit(0);
    }

    if (!snd3dsInitialize())
    {
        printf ("Unable to initialize CSND\n");
        exit (0);
    }

    ui3dsInitialize();

    if (romfsInit())
    {
        printf ("Unable to initialize romfs\n");
        exit(0);
    }

    printf ("Initialization complete\n");

    osSetSpeedupEnable(1);    // Performance: use the higher clock speed for new 3DS.

    enableAptHooks();

    emulatorSettingsLoad(true, false, true);

    if (settings3DS.GameScreen != (int) screenSettings.GameScreen)
    {
        ui3dsUpdateScreenSettings((gfxScreen_t) settings3DS.GameScreen);
        swapScreenTransition();
    }

    // Do this one more time.
    if (file3dsGetCurrentDir()[0] == 0)
        file3dsInitialize();

    srvInit();
}


//--------------------------------------------------------
// Finalize the emulator.
//--------------------------------------------------------
void emulatorFinalize()
{
    consoleClear();
    impl3dsFinalize();

#ifndef EMU_RELEASE
    printf("gspWaitForP3D:\n");
#endif
    gspWaitForVBlank();
    gpu3dsWaitForPreviousFlush();
    gspWaitForVBlank();

#ifndef EMU_RELEASE
    printf("snd3dsFinalize:\n");
#endif
    snd3dsFinalize();

#ifndef EMU_RELEASE
    printf("gpu3dsFinalize:\n");
#endif
    gpu3dsFinalize();

#ifndef EMU_RELEASE
    printf("ptmSysmExit:\n");
#endif
    ptmSysmExit();

    disableAptHooks();
    finalizeIRED();

#ifndef EMU_RELEASE
    printf("romfsExit:\n");
#endif
    romfsExit();

#ifndef EMU_RELEASE
    printf("hidExit:\n");
#endif
	hidExit();

#ifndef EMU_RELEASE
    printf("aptExit:\n");
#endif
	aptExit();

#ifndef EMU_RELEASE
    printf("srvExit:\n");
#endif
	srvExit();
}


bool firstFrame = true;


//---------------------------------------------------------
// Counts the number of frames per second, and prints
// it to the bottom screen every 60 frames.
//---------------------------------------------------------
char frameCountBuffer[70];
void updateFrameCount()
{
    if (frameCountTick == 0)
        frameCountTick = svcGetSystemTick();

    if (frameCount60 == 0)
    {
        u64 newTick = svcGetSystemTick();
        float timeDelta = ((float)(newTick - frameCountTick))/TICKS_PER_SEC;
        int fpsmul10 = (int)((float)600 / timeDelta);

#if !defined(EMU_RELEASE) && !defined(DEBUG_CPU) && !defined(DEBUG_APU)
        consoleClear();
#endif

        if (settings3DS.HideUnnecessaryBottomScrText == 0)
        {
            if (framesSkippedCount)
                snprintf (frameCountBuffer, 69, "FPS: %2d.%1d (%d skipped)\n", fpsmul10 / 10, fpsmul10 % 10, framesSkippedCount);
            else
                snprintf (frameCountBuffer, 69, "FPS: %2d.%1d \n", fpsmul10 / 10, fpsmul10 % 10);

            ui3dsDrawRect(2, 2, 200, 16, 0x000000);
            ui3dsDrawStringWithNoWrapping(2, 2, 200, 16, 0x7f7f7f, HALIGN_LEFT, frameCountBuffer);
        }

        frameCount60 = 60;
        framesSkippedCount = 0;

#if !defined(EMU_RELEASE) && !defined(DEBUG_CPU) && !defined(DEBUG_APU)
        printf ("\n\n");
        for (int i=0; i<100; i++)
        {
            t3dsShowTotalTiming(i);
        }
        t3dsResetTimings();
#endif
        frameCountTick = newTick;

    }

    frameCount60--;
}


//----------------------------------------------------------
// This is the main emulation loop. It calls the
//    impl3dsRunOneFrame
//   (which must be implemented for any new core)
// for the execution of the frame.
//----------------------------------------------------------
void emulatorLoop()
{
	// Main loop
    //emulator.enableDebug = true;
    emulator.waitBehavior = FPS_WAIT_FULL;

    int emuFramesSkipped = 0;
    long emuFrameTotalActualTicks = 0;
    long emuFrameTotalAccurateTicks = 0;

    bool firstFrame = true;
    appSuspended = 0;

    snd3DS.generateSilence = false;

    gpu3dsResetState();

    frameCount60 = 60;
    frameCountTick = 0;
    framesSkippedCount = 0;

    long startFrameTick = svcGetSystemTick();

    bool skipDrawingFrame = false;

    // Reinitialize the console.
    consoleInit(screenSettings.SecondScreen, NULL);
    gfxSetDoubleBuffering(screenSettings.GameScreen, true);
    gfxSetDoubleBuffering(screenSettings.SecondScreen, false);

    menu3dsDrawBlackScreen();
    if (settings3DS.HideUnnecessaryBottomScrText == 0)
    {
#define getText getTextFromMap
        std::string helpText = screenSettings.GameScreen == GFX_BOTTOM ? getText("点触下屏幕") : getText("点触屏幕");
        if (!aptIsHomeAllowed())
            helpText += getText("或按下Home键");
        helpText += getText("呼出菜单");
        ui3dsDrawStringWithNoWrapping(0, 100, screenSettings.SecondScreenWidth, 115,
            0x7f7f7f, HALIGN_CENTER,
            helpText.c_str(), 0, true);
#undef getText
    }

    snd3dsStartPlaying();

    impl3dsEmulationBegin();
    initIRED();

	while (true)
	{
        startFrameTick = svcGetSystemTick();
        aptMainLoop();

        if (appExiting || appSuspended)
            break;

        gpu3dsStartNewFrame();
        if(screenSettings.GameScreen == GFX_TOP && !settings3DS.Disable3DSlider) {
            gfxSet3D(true);
            gpu3dsCheckSlider();
        } else
            gfxSet3D(false);
        updateFrameCount();

        input3dsScanInputForEmulation();
        input3dsScanInputForEmulation2P();

        if (emulator.emulatorState != EMUSTATE_EMULATE)
            break;

        impl3dsEmulationRunOneFrame(firstFrame, skipDrawingFrame);

        firstFrame = false;

        // This either waits for the next frame, or decides to skip
        // the rendering for the next frame if we are too slow.
        //
#ifndef EMU_RELEASE
       if (emulator.isReal3DS)
#endif
        {

            // Check the keys to see if the user is fast-forwarding
            //
           int keysHeld = input3dsGetCurrentKeysHeld();
            emulator.fastForwarding = false;
           if ((settings3DS.UseGlobalEmuControlKeys && (settings3DS.GlobalButtonHotkeyDisableFramelimit & keysHeld)) ||
               (!settings3DS.UseGlobalEmuControlKeys && (settings3DS.ButtonHotkeyDisableFramelimit & keysHeld)))
               emulator.fastForwarding = true;

            long currentTick = svcGetSystemTick();
            long actualTicksThisFrame = currentTick - startFrameTick;
            long ticksPerFrame = settings3DS.TicksPerFrame;
            if (emulator.fastForwarding)
                ticksPerFrame = TICKS_PER_FRAME_FASTFORWARD;

            emuFrameTotalActualTicks += actualTicksThisFrame;  // actual time spent rendering past x frames.
            emuFrameTotalAccurateTicks += ticksPerFrame;  // time supposed to be spent rendering past x frames.

            int isSlow = 0;

            long skew = emuFrameTotalAccurateTicks - emuFrameTotalActualTicks;

            if (skew < 0)
            {
                // We've skewed out of the actual frame rate.
                // Once we skew beyond 0.1 (10%) frames slower, skip the frame.
                //
                if (skew < -ticksPerFrame/10 && emuFramesSkipped < settings3DS.MaxFrameSkips)
                {
                    skipDrawingFrame = true;
                    emuFramesSkipped++;

                    framesSkippedCount++;   // this is used for the stats display every 60 frames.
                }
                else
                {
                    skipDrawingFrame = false;

                    if (emuFramesSkipped >= settings3DS.MaxFrameSkips)
                    {
                        emuFramesSkipped = 0;
                        emuFrameTotalActualTicks = actualTicksThisFrame;
                        emuFrameTotalAccurateTicks = ticksPerFrame;
                    }
                }
            }
            else
            {

                float timeDiffInMilliseconds = (float)skew * 1000000 / TICKS_PER_SEC;
                if (emulator.waitBehavior == FPS_WAIT_HALF)
                    timeDiffInMilliseconds /= 2;
                else if (emulator.waitBehavior == FPS_WAIT_NONE)
                    timeDiffInMilliseconds = 1;
                emulator.waitBehavior = FPS_WAIT_FULL;

                // Reset the counters.
                //
                emuFrameTotalActualTicks = 0;
                emuFrameTotalAccurateTicks = 0;
                emuFramesSkipped = 0;

                svcSleepThread ((long)(timeDiffInMilliseconds * 1000));
                skipDrawingFrame = false;
            }

        }

	}

    if (!appSuspended)
        snd3dsStopPlaying();

    // Wait for the sound thread to leave the snd3dsMixSamples entirely
    // to prevent a race condition between the PTMU_GetBatteryChargeState (when
    // drawing the menu) and GSPGPU_FlushDataCache (in the sound thread).
    //
    // (There's probably a better way to do this, but this will do for now)
    //
    // svcSleepThread(500000);
}


//---------------------------------------------------------
// Main entrypoint.
//---------------------------------------------------------
int main()
{
    emulatorInitialize();
    clearScreenWithLogo();

    ui3dsSetFont(settings3DS.Font, true);
    ui3dsSetLanguage(settings3DS.Language, true);
    gbk3dsLoadGBKImage();
    menuSelectFile();

    while (aptMainLoop()) {
        switch (emulator.emulatorState) {
            case EMUSTATE_PAUSEMENU:
                menuPause();
                break;
            case EMUSTATE_EMULATE:
                emulatorLoop();
                break;
            case EMUSTATE_END:
                appExiting = 1;
                break;
        }
        if (appExiting)
            break;
    }

    fileList.clear();

    if (emulator.emulatorState > 0 && settings3DS.AutoSavestate)
        impl3dsLoadState(0);

    // printf("emulatorFinalize:\n");
    emulatorFinalize();
    // printf("Exiting...\n");
    return 0;
}
