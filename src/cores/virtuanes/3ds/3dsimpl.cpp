//=============================================================================
// Contains all the hooks and interfaces between the emulator interface
// and the main emulator core.
//=============================================================================

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <3ds.h>

#include <dirent.h>

#include "3dstypes.h"
#include "3dsemu.h"
#include "3dsexit.h"
#include "3dsgpu.h"
#include "3dssound.h"
#include "3dsui.h"
#include "3dsinput.h"
#include "3dsfiles.h"
#include "3dsinterface.h"
#include "3dsmain.h"
#include "3dsimpl.h"
#include "3dsopt.h"
#include "3dsconfig.h"
#include "3dsdbg.h"
#include "3dsvideo.h"

//---------------------------------------------------------
// All other codes that you need here.
//---------------------------------------------------------
#include "3dsimpl.h"
#include "3dsimpl_gpu.h"
#include "shaderfast2_shbin.h"
#include "shaderslow_shbin.h"
#include "shaderslow2_shbin.h"

#include "MMU.h"
#include "APU.h"
#include "PPU.h"
#include "ROM.h"
#include "NES.h"
#include "Pad.h"
#include "Config.h"
#include "palette.h"
#include "inputRedirect.h"

#define SETTINGS_ALLSPRITES         0
#define SETTINGS_GLOBALINSERTCOIN1  1
#define SETTINGS_GLOBALINSERTCOIN2  2
#define SETTINGS_INSERTCOIN1        3
#define SETTINGS_INSERTCOIN2        4

//----------------------------------------------------------------------
// Settings
//----------------------------------------------------------------------
SSettings3DS settings3DS;

//----------------------------------------------------------------------
// Menu options
//----------------------------------------------------------------------

SMenuItem optionsForScreenSwap[] = {
    MENU_MAKE_DIALOG_ACTION (0, "上屏幕",               ""),
    MENU_MAKE_DIALOG_ACTION (1, "下屏幕",                  ""),
    MENU_MAKE_LASTITEM  ()
};

SMenuItem optionsForFont[] = {
    MENU_MAKE_DIALOG_ACTION (0, "Tempesta",               ""),
    MENU_MAKE_DIALOG_ACTION (1, "Ronda",                  ""),
    MENU_MAKE_DIALOG_ACTION (2, "Arial",                  ""),
    MENU_MAKE_LASTITEM  ()
};

SMenuItem optionsForStretch[] = {
    MENU_MAKE_DIALOG_ACTION (0, "不拉伸",                    "点对点"),
    MENU_MAKE_DIALOG_ACTION (1, "适配4:3",                  "拉伸到320x240"),
    MENU_MAKE_DIALOG_ACTION (2, "全屏",                     "拉伸到全屏幕"),
    MENU_MAKE_DIALOG_ACTION (3, "裁剪适配4:3",               "裁剪并拉伸到320x240"),
    MENU_MAKE_DIALOG_ACTION (4, "裁剪全屏",                  "裁剪并拉伸到全屏幕"),
    MENU_MAKE_LASTITEM  ()
};

SMenuItem optionsForPalette[] = {
    MENU_MAKE_DIALOG_ACTION (0, "FCEUX",                    "默认"),
    MENU_MAKE_DIALOG_ACTION (1, "Composite Direct (FBX)",   "直接捕捉调色板"),
    MENU_MAKE_DIALOG_ACTION (2, "NES Classic (FBX)",        "提取自NES Classic"),
    MENU_MAKE_DIALOG_ACTION (3, "PC-10",                    "Playchoice-10街机"),
    MENU_MAKE_DIALOG_ACTION (4, "PVM Style D93 (FBX)",      "支持D93色温的Sony PVM"),
    MENU_MAKE_DIALOG_ACTION (5, "Smooth (FBX)",             "Firebrandx的终极调色板"),
    MENU_MAKE_DIALOG_ACTION (6, "Sony CXA",                 "消费级Sony TV电视配置"),
    MENU_MAKE_DIALOG_ACTION (7, "Wavebeam",                 "Nakedarthur的最终专业级调色板"),
    MENU_MAKE_LASTITEM  ()
};

SMenuItem optionsForFrameskip[] = {
    MENU_MAKE_DIALOG_ACTION (0, "关闭",             ""),
    MENU_MAKE_DIALOG_ACTION (1, "开启 (最高1帧)",    ""),
    MENU_MAKE_DIALOG_ACTION (2, "开启 (最高2帧)",    ""),
    MENU_MAKE_DIALOG_ACTION (3, "开启 (最高3帧)",    ""),
    MENU_MAKE_DIALOG_ACTION (4, "开启 (最高4帧)",    ""),
    MENU_MAKE_LASTITEM  ()
};

SMenuItem optionsForFrameRate[] = {
    MENU_MAKE_DIALOG_ACTION (0, "ROM默认",                   ""),
    MENU_MAKE_DIALOG_ACTION (1, "50 FPS",                   ""),
    MENU_MAKE_DIALOG_ACTION (2, "60 FPS",                   ""),
    MENU_MAKE_LASTITEM  ()
};

SMenuItem optionsForAutoSaveSRAMDelay[] = {
    MENU_MAKE_DIALOG_ACTION (1, "1秒",    ""),
    MENU_MAKE_DIALOG_ACTION (2, "10秒",   ""),
    MENU_MAKE_DIALOG_ACTION (3, "60秒",   ""),
    MENU_MAKE_DIALOG_ACTION (4, "关闭",        "点触下屏幕保存"),
    MENU_MAKE_LASTITEM  ()
};

SMenuItem optionsForTurboFire[] = {
    MENU_MAKE_DIALOG_ACTION (0, "无",         ""),
    MENU_MAKE_DIALOG_ACTION (10, "最慢",      ""),
    MENU_MAKE_DIALOG_ACTION (8, "更慢",       ""),
    MENU_MAKE_DIALOG_ACTION (6, "慢",         ""),
    MENU_MAKE_DIALOG_ACTION (4, "快",         ""),
    MENU_MAKE_DIALOG_ACTION (2, "更快",       ""),
    MENU_MAKE_DIALOG_ACTION (1, "特快",       ""),
    MENU_MAKE_LASTITEM  ()
};

SMenuItem optionsForButtons[] = {
    MENU_MAKE_DIALOG_ACTION (0,             "无",               ""),
    MENU_MAKE_DIALOG_ACTION (BTNNES_A,      "NES A键",          ""),
    MENU_MAKE_DIALOG_ACTION (BTNNES_B,      "NES B键",          ""),
    MENU_MAKE_DIALOG_ACTION (BTNNES_SELECT, "NES SELECT键",     ""),
    MENU_MAKE_DIALOG_ACTION (BTNNES_START,  "NES START键",      ""),
    MENU_MAKE_LASTITEM  ()
};

SMenuItem optionsFor3DSButtons[] = {
    MENU_MAKE_DIALOG_ACTION (0,                 "无",          ""),
    MENU_MAKE_DIALOG_ACTION (KEY_A,             "3DS A键",     ""),
    MENU_MAKE_DIALOG_ACTION (KEY_B,             "3DS B键",     ""),
    MENU_MAKE_DIALOG_ACTION (KEY_X,             "3DS X键",     ""),
    MENU_MAKE_DIALOG_ACTION (KEY_Y,             "3DS Y键",     ""),
    MENU_MAKE_DIALOG_ACTION (KEY_L,             "3DS L键",     ""),
    MENU_MAKE_DIALOG_ACTION (KEY_R,             "3DS R键",     ""),
    MENU_MAKE_DIALOG_ACTION (KEY_ZL,            "New3DS ZL键",     ""),
    MENU_MAKE_DIALOG_ACTION (KEY_ZR,            "New3DS ZR键",     ""),
    MENU_MAKE_LASTITEM  ()
};


SMenuItem optionsForSpriteFlicker[] =
{
    MENU_MAKE_DIALOG_ACTION (0, "模拟实机",             "显示类似实机的闪烁效果"),
    MENU_MAKE_DIALOG_ACTION (1, "视觉优先",             "以较低模拟精确度换取更好的显示效果"),
    MENU_MAKE_LASTITEM  ()
};

SMenuItem optionMenu[] = {
    MENU_MAKE_HEADER1   ("全局设置"),
    MENU_MAKE_PICKER    (11000, "  屏幕比例", "您希望屏幕以何种方式显示?", optionsForStretch, DIALOGCOLOR_CYAN),
    MENU_MAKE_PICKER    (18000, "  字体", "用于用户界面的字体(仅适用于字母和数字)", optionsForFont, DIALOGCOLOR_CYAN),
    MENU_MAKE_PICKER    (15000, "  游戏显示屏幕", "选择使用上屏或下屏进行游玩", optionsForScreenSwap, DIALOGCOLOR_CYAN),
    MENU_MAKE_CHECKBOX  (15001, "  隐藏副屏幕的文本", 0),
    MENU_MAKE_CHECKBOX  (21001, "  禁用3D调节杆", 0),
    MENU_MAKE_DISABLED  (""),
    MENU_MAKE_CHECKBOX  (21000, "  退出时自动保存即时存档并在启动时自动加载", 0),
    MENU_MAKE_DISABLED  (""),
    MENU_MAKE_PICKER    (69696, "  调色板", "选取您喜爱的NES调色板", optionsForPalette, DIALOGCOLOR_CYAN),
    MENU_MAKE_DISABLED  (""),
    MENU_MAKE_HEADER1   ("游戏设置"),
    MENU_MAKE_PICKER    (10000, "  跳帧", "跳帧可以加快游戏速度,但可能会导致画面不平滑", optionsForFrameskip, DIALOGCOLOR_CYAN),
    MENU_MAKE_PICKER    (12000, "  帧率", "某些游戏默认运行于 50 或 60 FPS. 可在需要时覆盖帧率设置.", optionsForFrameRate, DIALOGCOLOR_CYAN),
    MENU_MAKE_PICKER    (19000, "  精灵闪烁", "精灵在实机上将会闪烁. 您可以禁用以获得更好的视觉效果.", optionsForSpriteFlicker, DIALOGCOLOR_CYAN),
    MENU_MAKE_DISABLED  (""),
    MENU_MAKE_HEADER1   ("音频"),
    MENU_MAKE_CHECKBOX  (20002, "  将音量设置应用于所有游戏", 0),
    MENU_MAKE_GAUGE     (14000, "  音量扩增", 0, 8, 4),
    MENU_MAKE_LASTITEM  ()
};


SMenuItem controlsMenu[] = {
    MENU_MAKE_HEADER1   ("按键设置"),
    MENU_MAKE_CHECKBOX  (20000, "  为所有游戏映射按键", 0),
    MENU_MAKE_CHECKBOX  (20001, "  为所有游戏映射连发按键", 0),
    MENU_MAKE_DISABLED  (""),
    MENU_MAKE_HEADER2   ("3DS A键"),
    MENU_MAKE_PICKER    (13010, "  映射到", "", optionsForButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_PICKER    (13020, "  映射到", "", optionsForButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_GAUGE     (13000, "  连发速度", 0, 10, 0),
    MENU_MAKE_DISABLED  (""),
    MENU_MAKE_HEADER2   ("3DS B键"),
    MENU_MAKE_PICKER    (13011, "  映射到", "", optionsForButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_PICKER    (13021, "  映射到", "", optionsForButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_GAUGE     (13001, "  连发速度", 0, 10, 0),
    MENU_MAKE_DISABLED  (""),
    MENU_MAKE_HEADER2   ("3DS X键"),
    MENU_MAKE_PICKER    (13012, "  映射到", "", optionsForButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_PICKER    (13022, "  映射到", "", optionsForButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_GAUGE     (13002, "  连发速度", 0, 10, 0),
    MENU_MAKE_DISABLED  (""),
    MENU_MAKE_HEADER2   ("3DS Y键"),
    MENU_MAKE_PICKER    (13013, "  映射到", "", optionsForButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_PICKER    (13023, "  映射到", "", optionsForButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_GAUGE     (13003, "  连发速度", 0, 10, 0),
    MENU_MAKE_DISABLED  (""),
    MENU_MAKE_HEADER2   ("3DS L键"),
    MENU_MAKE_PICKER    (13014, "  映射到", "", optionsForButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_PICKER    (13024, "  映射到", "", optionsForButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_GAUGE     (13004, "  连发速度", 0, 10, 0),
    MENU_MAKE_DISABLED  (""),
    MENU_MAKE_HEADER2   ("3DS R键"),
    MENU_MAKE_PICKER    (13015, "  映射到", "", optionsForButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_PICKER    (13025, "  映射到", "", optionsForButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_GAUGE     (13005, "  连发速度", 0, 10, 0),
    MENU_MAKE_DISABLED  (""),
    MENU_MAKE_HEADER2   ("New3DS ZL键"),
    MENU_MAKE_PICKER    (13016, "  映射到", "", optionsForButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_PICKER    (13026, "  映射到", "", optionsForButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_GAUGE     (13006, "  连发速度", 0, 10, 0),
    MENU_MAKE_DISABLED  (""),
    MENU_MAKE_HEADER2   ("New3DS ZR键"),
    MENU_MAKE_PICKER    (13017, "  映射到", "", optionsForButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_PICKER    (13027, "  映射到", "", optionsForButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_GAUGE     (13007, "  连发速度", 0, 10, 0),
    MENU_MAKE_DISABLED  (""),
    MENU_MAKE_HEADER2   ("3DS SELECT键"),
    MENU_MAKE_PICKER    (13018, "  映射到", "", optionsForButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_PICKER    (13028, "  映射到", "", optionsForButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_DISABLED  (""),
    MENU_MAKE_HEADER2   ("3DS START键"),
    MENU_MAKE_PICKER    (13019, "  映射到", "", optionsForButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_PICKER    (13029, "  映射到", "", optionsForButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_DISABLED  (""),
    MENU_MAKE_HEADER1   ("模拟器功能设置"),
    MENU_MAKE_CHECKBOX  (50003, "为所有游戏应用映射", 0),
    MENU_MAKE_PICKER    (23001, "打开模拟器菜单", "", optionsFor3DSButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_PICKER    (23002, "快进", "", optionsFor3DSButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_DISABLED  ("  (在New3DS上效果更好. 可能会导致游戏卡死或出错.)"),
    MENU_MAKE_PICKER    (23003, "为1P投币 (VS Games)", "", optionsFor3DSButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_PICKER    (23004, "为2P投币 (VS Games)", "", optionsFor3DSButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_LASTITEM  ()
};


//-------------------------------------------------------
SMenuItem optionsForDisk[] =
{
    MENU_MAKE_DIALOG_ACTION (0, "插入磁盘",         ""),
    MENU_MAKE_DIALOG_ACTION (1, "切换到磁盘1的A面",  ""),
    MENU_MAKE_DIALOG_ACTION (2, "切换到磁盘1的B面",  ""),
    MENU_MAKE_DIALOG_ACTION (3, "切换到磁盘2的A面",  ""),
    MENU_MAKE_DIALOG_ACTION (4, "切换到磁盘2的B面",  ""),
    MENU_MAKE_DIALOG_ACTION (5, "切换到磁盘3的A面",  ""),
    MENU_MAKE_DIALOG_ACTION (6, "切换到磁盘3的B面",  ""),
    MENU_MAKE_DIALOG_ACTION (7, "切换到磁盘4的A面",  ""),
    MENU_MAKE_DIALOG_ACTION (8, "切换到磁盘4的B面",  ""),
    MENU_MAKE_LASTITEM  ()
};


//-------------------------------------------------------
// Standard in-game emulator menu.
// You should not modify those menu items that are
// marked 'do not modify'.
//-------------------------------------------------------
SMenuItem emulatorMenu[] = {
    MENU_MAKE_HEADER2   ("模拟器"),               // Do not modify
    MENU_MAKE_ACTION    (1000, "  返回游戏"),    // Do not modify
    MENU_MAKE_PICKER2   (30000,"  选择磁盘", "", optionsForDisk, DIALOGCOLOR_CYAN),
    MENU_MAKE_HEADER2   (""),

    MENU_MAKE_HEADER2   ("即时存档"),
    MENU_MAKE_ACTION    (2001, "  保存存档位 #1"),   // Do not modify
    MENU_MAKE_ACTION    (2002, "  保存存档位 #2"),   // Do not modify
    MENU_MAKE_ACTION    (2003, "  保存存档位 #3"),   // Do not modify
    MENU_MAKE_ACTION    (2004, "  保存存档位 #4"),   // Do not modify
    MENU_MAKE_ACTION    (2005, "  保存存档位 #5"),   // Do not modify
    MENU_MAKE_HEADER2   (""),

    MENU_MAKE_ACTION    (3001, "  加载存档位 #1"),   // Do not modify
    MENU_MAKE_ACTION    (3002, "  加载存档位 #2"),   // Do not modify
    MENU_MAKE_ACTION    (3003, "  加载存档位 #3"),   // Do not modify
    MENU_MAKE_ACTION    (3004, "  加载存档位 #4"),   // Do not modify
    MENU_MAKE_ACTION    (3005, "  加载存档位 #5"),   // Do not modify
    MENU_MAKE_HEADER2   (""),

    MENU_MAKE_HEADER2   ("其他"),                 // Do not modify
    MENU_MAKE_ACTION    (4001, "  截屏"),// Do not modify
    MENU_MAKE_ACTION    (5001, "  重置控制台"),  // Do not modify
    MENU_MAKE_ACTION    (6001, "  退出"),           // Do not modify
    MENU_MAKE_LASTITEM  ()
};





//------------------------------------------------------------------------
// Memory Usage = 0.003 MB   for 4-point rectangle (triangle strip) vertex buffer
#define RECTANGLE_BUFFER_SIZE           0x1000

//------------------------------------------------------------------------
// Memory Usage = 0.003 MB   for 6-point quad vertex buffer (Citra only)
#define CITRA_VERTEX_BUFFER_SIZE        0x1000

// Memory Usage = Not used (Real 3DS only)
#define CITRA_TILE_BUFFER_SIZE          0x1000


//------------------------------------------------------------------------
// Memory Usage = 0.003 MB   for 6-point quad vertex buffer (Real 3DS only)
#define REAL3DS_VERTEX_BUFFER_SIZE      0x1000

// Memory Usage = 0.003 MB   for 2-point rectangle vertex buffer (Real 3DS only)
#define REAL3DS_TILE_BUFFER_SIZE        0x1000


//---------------------------------------------------------
// Our textures
//---------------------------------------------------------

NES* nes = NULL;

unsigned char linecolor[256];



//---------------------------------------------------------
// Settings related to the emulator.
//---------------------------------------------------------
extern SSettings3DS settings3DS;


//---------------------------------------------------------
// Provide a comma-separated list of file extensions
//---------------------------------------------------------
const char *impl3dsRomExtensions = "nes,fds";


//---------------------------------------------------------
// The title image .PNG filename.
//---------------------------------------------------------
const char *impl3dsTitleImage = "romfs:/virtuanes_3ds_top.png";


//---------------------------------------------------------
// The title that displays at the bottom right of the
// menu.
//---------------------------------------------------------
const char *impl3dsTitleText = "VirtuaNES for 3DS v1.03c";


//---------------------------------------------------------
// The bitmaps for the emulated console's UP, DOWN, LEFT,
// RIGHT keys.
//---------------------------------------------------------
u32 input3dsDKeys[4] = { BTNNES_UP, BTNNES_DOWN, BTNNES_LEFT, BTNNES_RIGHT };


//---------------------------------------------------------
// The list of valid joypad bitmaps for the emulated
// console.
//
// This should NOT include D-keys.
//---------------------------------------------------------
u32 input3dsValidButtonMappings[10] = { BTNNES_A, BTNNES_B, BTNNES_SELECT, BTNNES_START, 0, 0, 0, 0, 0, 0 };


//---------------------------------------------------------
// The maps for the 10 3DS keys to the emulated consoles
// joypad bitmaps for the following 3DS keys (in order):
//   A, B, X, Y, L, R, ZL, ZR, SELECT, START
//
// This should NOT include D-keys.
//---------------------------------------------------------
u32 input3dsDefaultButtonMappings[10] = { BTNNES_A, BTNNES_B, BTNNES_A, BTNNES_B, 0, 0, 0, 0, BTNNES_SELECT, BTNNES_START };


//---------------------------------------------------------
// Initializes the emulator core.
//---------------------------------------------------------
bool impl3dsInitializeCore()
{
    // Original call to nespalInitialize()
    nespalInitialize(settings3DS.NESPalette);

	// Initialize our GPU.
	// Load up and initialize any shaders
	//
    if (emulator.isReal3DS)
    {
    	gpu3dsLoadShader(0, (u32 *)shaderslow_shbin, shaderslow_shbin_size, 0);     // copy to screen
    	gpu3dsLoadShader(1, (u32 *)shaderfast2_shbin, shaderfast2_shbin_size, 6);   // draw tiles
    }
    else
    {
    	gpu3dsLoadShader(0, (u32 *)shaderslow_shbin, shaderslow_shbin_size, 0);     // copy to screen
        gpu3dsLoadShader(1, (u32 *)shaderslow2_shbin, shaderslow2_shbin_size, 0);   // draw tiles
    }

	gpu3dsInitializeShaderRegistersForRenderTarget(0, 10);
	gpu3dsInitializeShaderRegistersForTexture(4, 14);
	gpu3dsInitializeShaderRegistersForTextureOffset(6);


    // Create all the necessary textures
    //
    //nesTileCacheTexture = gpu3dsCreateTextureInLinearMemory(1024, 1024, GPU_RGBA5551);

    if (!video3dsInitializeSoftwareRendering(512, 256, GX_TRANSFER_FMT_RGB565))
        return false;

	// allocate all necessary vertex lists
	//
    if (emulator.isReal3DS)
    {
        gpu3dsAllocVertexList(&GPU3DSExt.rectangleVertexes, RECTANGLE_BUFFER_SIZE, sizeof(SVertexColor), 2, SVERTEXCOLOR_ATTRIBFORMAT);
        gpu3dsAllocVertexList(&GPU3DSExt.quadVertexes, REAL3DS_VERTEX_BUFFER_SIZE, sizeof(SVertexTexCoord), 2, SVERTEXTEXCOORD_ATTRIBFORMAT);
        gpu3dsAllocVertexList(&GPU3DSExt.tileVertexes, REAL3DS_TILE_BUFFER_SIZE, sizeof(SVertexTexCoord), 2, SVERTEXTEXCOORD_ATTRIBFORMAT);
    }
    else
    {
        gpu3dsAllocVertexList(&GPU3DSExt.rectangleVertexes, RECTANGLE_BUFFER_SIZE, sizeof(SVertexColor), 2, SVERTEXCOLOR_ATTRIBFORMAT);
        gpu3dsAllocVertexList(&GPU3DSExt.quadVertexes, CITRA_VERTEX_BUFFER_SIZE, sizeof(SVertexTexCoord), 2, SVERTEXTEXCOORD_ATTRIBFORMAT);
        gpu3dsAllocVertexList(&GPU3DSExt.tileVertexes, CITRA_TILE_BUFFER_SIZE, sizeof(SVertexTexCoord), 2, SVERTEXTEXCOORD_ATTRIBFORMAT);
    }

    if (GPU3DSExt.quadVertexes.ListBase == NULL ||
        GPU3DSExt.tileVertexes.ListBase == NULL ||
        GPU3DSExt.rectangleVertexes.ListBase == NULL)
    {
        printf ("Unable to allocate vertex list buffers \n");
        return false;
    }

	gpu3dsUseShader(0);
    return true;
}


//---------------------------------------------------------
// Finalizes and frees up any resources.
//---------------------------------------------------------
void impl3dsFinalize()
{
    video3dsFinalize();

	if (nes) delete nes;
}


int soundSamplesPerGeneration = 0;
int soundSamplesPerSecond = 0;
short soundSamples[1000];

//---------------------------------------------------------
// Mix sound samples into a temporary buffer.
//
// This gives time for the sound generation to execute
// from the 2nd core before copying it to the actual
// output buffer.
//---------------------------------------------------------
void impl3dsGenerateSoundSamples(int numberOfSamples)
{
	if (nes && soundSamplesPerGeneration)
	{
		nes->apu->Process((unsigned char *)soundSamples, soundSamplesPerGeneration * 2, emulator.fastForwarding);
	}
}


//---------------------------------------------------------
// Mix sound samples into a temporary buffer.
//
// This gives time for the sound generation to execute
// from the 2nd core before copying it to the actual
// output buffer.
//
// For a console with only MONO output, simply copy
// the samples into the leftSamples buffer.
//---------------------------------------------------------
void impl3dsOutputSoundSamples(int numberOfSamples, short *leftSamples, short *rightSamples)
{
	for (int i = 0; i < soundSamplesPerGeneration; i++)
	{
		leftSamples[i] = soundSamples[i];
	}
}


//---------------------------------------------------------
// This is called when a ROM needs to be loaded and the
// emulator engine initialized.
//---------------------------------------------------------
bool impl3dsLoadROM(char *romFilePath)
{
	if (nes)
    {
		delete nes;
        nes = NULL;
    }

	nes = new NES(romFilePath);
	if (nes->error)
		return false;

	//nes->ppu->SetScreenPtr( NULL, linecolor );
	nes->ppu->SetScreenRGBAPtr((WORD*) video3dsGetCurrentSoftwareBuffer(), linecolor );

	// compute a sample rate closes to 32000 kHz.
	//
    int nesSampleRate = 32000;
    bool new3DS = false;
    APT_CheckNew3DS(&new3DS);

    // Lagrange Point and Old 3DS, we need to use a lower sample rate
    // because the 2nd core is not fast enough to generate VRC7 sounds.
    //
    if (nes->rom->GetMapperNo() == 85 && !new3DS)
        nesSampleRate = 20000;

    int numberOfGenerationsPerSecond = nes->nescfg->FrameRate * 2;
    soundSamplesPerGeneration = snd3dsComputeSamplesPerLoop(nesSampleRate, numberOfGenerationsPerSecond);
	soundSamplesPerSecond = snd3dsComputeSampleRate(nesSampleRate, numberOfGenerationsPerSecond);
	snd3dsSetSampleRate(
		false,
		nesSampleRate,
		numberOfGenerationsPerSecond,
		true,
        1, 4);

	Config.sound.nRate = soundSamplesPerSecond;
	Config.sound.nBits = 16;
	Config.sound.nFilterType = 1;
	Config.sound.nVolume[0] = 200;
    Config.graphics.bAllSprite = 0;

	nes->Reset();

    // If this is a FDS game, enable the FDS menu.
    //
    int fdsDiskNo = nes->rom->GetDiskNo();

    for (int i = 0; ; i++)
    {
        if (emulatorMenu[i].Type == MENUITEM_LASTITEM)
            break;
        if (emulatorMenu[i].ID == 30000)
        {
            if (fdsDiskNo > 0)
                emulatorMenu[i].Type = MENUITEM_PICKER2;
            else
                emulatorMenu[i].Type = MENUITEM_DISABLED;
            break;
        }
    }
    for (int i = 1; i <= 8; i++)
        optionsForDisk[i].Type = (fdsDiskNo >= i) ? MENUITEM_ACTION : MENUITEM_DISABLED;


	//svcSleepThread((long)10000000000);

	return true;
}


//---------------------------------------------------------
// This is called to determine what the frame rate of the
// game based on the ROM's region.
//---------------------------------------------------------
int impl3dsGetROMFrameRate()
{
	if (nes)
		return nes->nescfg->FrameRate;
	return 60;
}



//---------------------------------------------------------
// This is called when the user chooses to reset the
// console
//---------------------------------------------------------
void impl3dsResetConsole()
{
	if (nes)
		nes->SoftReset();
}


//---------------------------------------------------------
// This is called when preparing to start emulating
// a new frame. Use this to do any preparation of data,
// the hardware, swap any vertex list buffers, etc,
// before the frame is emulated
//---------------------------------------------------------
void impl3dsPrepareForNewFrame()
{
	gpu3dsSwapVertexListForNextFrame(&GPU3DSExt.quadVertexes);
    gpu3dsSwapVertexListForNextFrame(&GPU3DSExt.tileVertexes);
    gpu3dsSwapVertexListForNextFrame(&GPU3DSExt.rectangleVertexes);

    video3dsStartNewSoftwareRenderedFrame();
}




bool isOddFrame = false;
bool skipDrawingPreviousFrame = true;

uint32 			*bufferToTransfer = 0;
SGPUTexture 	*screenTexture = 0;


//---------------------------------------------------------
// Initialize any variables or state of the GPU
// before the emulation loop begins.
//---------------------------------------------------------
void impl3dsEmulationBegin()
{
	bufferToTransfer = 0;
	screenTexture = 0;
	skipDrawingPreviousFrame = true;

	gpu3dsUseShader(0);
	gpu3dsDisableAlphaBlending();
	gpu3dsDisableDepthTest();
	gpu3dsDisableAlphaTest();
	gpu3dsDisableStencilTest();
	gpu3dsSetTextureEnvironmentReplaceTexture0();
	gpu3dsSetRenderTargetToFrameBuffer(screenSettings.GameScreen);
	gpu3dsFlush();
	//if (emulator.isReal3DS)
	//	gpu3dsWaitForPreviousFlush();
}




//---------------------------------------------------------
// Polls and get the emulated console's joy pad.
//---------------------------------------------------------
u32 insertCoin1 = 0;
u32 insertCoin2 = 0;

void impl3dsEmulationPollInput()
{
    u32 keysHeld3ds = input3dsGetCurrentKeysHeld();
    u32 consoleJoyPad = input3dsProcess3dsKeys();

    u16 keysHeld2P = input3dsGetCurrentKeysHeld2P();
    u8 *ptrHeld2P = (u8 *)&keysHeld2P;
    u8 *ptrJoyPad = (u8 *)&consoleJoyPad;
    ptrJoyPad[1] |= (ptrHeld2P[0] & 0x0F);
    if (keysHeld2P & IRED_LEFT) ptrJoyPad[1] |= 0x40;
    if (keysHeld2P & IRED_RIGHT) ptrJoyPad[1] |= 0x80;
    if (keysHeld2P & IRED_UP) ptrJoyPad[1] |= 0x10;
    if (keysHeld2P & IRED_DOWN) ptrJoyPad[1] |= 0x20;

    if (nes)
		nes->pad->SetSyncData(consoleJoyPad);

    if (settings3DS.UseGlobalEmuControlKeys)
    {
        insertCoin1 = (keysHeld3ds & settings3DS.OtherOptions[SETTINGS_GLOBALINSERTCOIN1]) > 0;
        insertCoin2 = (keysHeld3ds & settings3DS.OtherOptions[SETTINGS_GLOBALINSERTCOIN2]) > 0;
    }
    else
    {
        insertCoin1 = (keysHeld3ds & settings3DS.OtherOptions[SETTINGS_INSERTCOIN1]) > 0;
        insertCoin2 = (keysHeld3ds & settings3DS.OtherOptions[SETTINGS_INSERTCOIN2]) > 0;
    }
}


//---------------------------------------------------------
// The following pipeline is used if the
// emulation engine does software rendering.
//
// You can potentially 'hide' the wait latencies by
// waiting only after some work on the main thread
// is complete.
//---------------------------------------------------------

int lastWait = 0;
#define WAIT_PPF		1
#define WAIT_P3D		2


void impl3dsRenderDrawTextureToFrameBuffer()
{
	t3dsStartTiming(14, "Draw Texture");
    int widthAdjust = screenSettings.GameScreen == GFX_TOP ? 0 : 40;

    // Draw a black colored rectangle covering the entire screen.
    //
	switch (settings3DS.ScreenStretch)
	{
		case 0:
            gpu3dsSetTextureEnvironmentReplaceColor();
            gpu3dsDrawRectangle(0, 0, 72 - widthAdjust, 240, 0, 0x000000ff);
            gpu3dsDrawRectangle(328 - widthAdjust, 0, screenSettings.GameScreenWidth, 240, 0, 0x000000ff);

            gpu3dsSetTextureEnvironmentReplaceTexture0();
            gpu3dsBindTextureMainScreen(video3dsGetPreviousScreenTexture(), GPU_TEXUNIT0);
			gpu3dsAddQuadVertexes(72 - widthAdjust, 0, 328 - widthAdjust, 240, 8, 0, 264, 240, 0);
			break;
		case 1:
            if (screenSettings.GameScreen == GFX_TOP)
            {
                gpu3dsSetTextureEnvironmentReplaceColor();
                gpu3dsDrawRectangle(0, 0, 40, 240, 0, 0x000000ff);
                gpu3dsDrawRectangle(360, 0, screenSettings.GameScreenWidth, 240, 0, 0x000000ff);
            }

            gpu3dsSetTextureEnvironmentReplaceTexture0();
            gpu3dsBindTextureMainScreen(video3dsGetPreviousScreenTexture(), GPU_TEXUNIT0);
			gpu3dsAddQuadVertexes(40 - widthAdjust, 0, 360 - widthAdjust, 240, 8.2, 0, 263.8, 240, 0);
			break;
		case 2:
            gpu3dsSetTextureEnvironmentReplaceTexture0();
            gpu3dsBindTextureMainScreen(video3dsGetPreviousScreenTexture(), GPU_TEXUNIT0);
			gpu3dsAddQuadVertexes(0, 0, screenSettings.GameScreenWidth, 240, 8.2, 0, 263.8, 240, 0);
			break;
		case 3:
            if (screenSettings.GameScreen == GFX_TOP)
            {
                gpu3dsSetTextureEnvironmentReplaceColor();
                gpu3dsDrawRectangle(0, 0, 40, 240, 0, 0x000000ff);
                gpu3dsDrawRectangle(360, 0, screenSettings.GameScreenWidth, 240, 0, 0x000000ff);
            }

            gpu3dsSetTextureEnvironmentReplaceTexture0();
            gpu3dsBindTextureMainScreen(video3dsGetPreviousScreenTexture(), GPU_TEXUNIT0);
			gpu3dsAddQuadVertexes(40 - widthAdjust, 0, 360 - widthAdjust, 240, 8.2 + 8, 0 + 8, 263.8 - 8, 240 - 8, 0);
			break;
		case 4:
            gpu3dsSetTextureEnvironmentReplaceTexture0();
            gpu3dsBindTextureMainScreen(video3dsGetPreviousScreenTexture(), GPU_TEXUNIT0);
			gpu3dsAddQuadVertexes(0, 0, screenSettings.GameScreenWidth, 240, 8.2 + 8, 0 + 8, 263.8 - 8, 240 - 8, 0);
			break;
	}
    gpu3dsDrawVertexes();
	t3dsEndTiming(14);

	t3dsStartTiming(15, "Flush");
	gpu3dsFlush();
	t3dsEndTiming(15);
}


//---------------------------------------------------------
// Executes one frame and draw to the screen.
//
// Note: TRUE will be passed in the firstFrame if this
// frame is to be run just after the emulator has booted
// up or returned from the menu.
//---------------------------------------------------------
extern int frameCount60;
void impl3dsEmulationRunOneFrame(bool firstFrame, bool skipDrawingFrame)
{
	t3dsStartTiming(1, "RunOneFrame");

#ifndef EMU_RELEASE
if (frameCount60 == 59)
{
    printf ("control1: %d\n", nes->rom->GetNesHeader()->control1);
    printf ("Mapper  : %d\n", nes->rom->GetMapperNo());
    printf ("PROM CRC: %08X\n", nes->rom->GetPROM_CRC());
    printf ("CRC     : %08X\n", nes->rom->GetROM_CRC());
}
#endif

	if (!skipDrawingPreviousFrame)
        video3dsTransferFrameBufferToScreenAndSwap();

	t3dsStartTiming(10, "EmulateFrame");
	if (nes)
	{
		impl3dsEmulationPollInput();

		if (skipDrawingFrame)
		{
			nes->EmulateFrame(false);
		}
		else
		{
            nes->ppu->SetScreenRGBAPtr((WORD*) video3dsGetCurrentSoftwareBuffer(), linecolor );
			nes->EmulateFrame(true);
		}
	}
	t3dsEndTiming(10);

	if (!skipDrawingFrame)
        video3dsCopySoftwareBufferToTexture();

	if (!skipDrawingPreviousFrame)
		impl3dsRenderDrawTextureToFrameBuffer();

	skipDrawingPreviousFrame = skipDrawingFrame;
	t3dsEndTiming(1);

}


//---------------------------------------------------------
// Finalize any variables or state of the GPU
// before the emulation loop ends and control
// goes into the menu.
//---------------------------------------------------------
void impl3dsEmulationEnd()
{
	// We have to do this to clear the wait event
	//
	/*if (lastWait != 0 && emulator.isReal3DS)
	{
		if (lastWait == WAIT_PPF)
			gspWaitForPPF();
		else
		if (lastWait == WAIT_P3D)
			gpu3dsWaitForPreviousFlush();
	}*/
}



//---------------------------------------------------------
// This is called when the bottom screen is touched
// during emulation, and the emulation engine is ready
// to display the pause menu.
//
// Use this to save the SRAM to SD card, if applicable.
//---------------------------------------------------------
void impl3dsEmulationPaused()
{
    if (nes)
    {
        int widthAdjust = screenSettings.GameScreen == GFX_TOP ? 0 : 40;

        ui3dsDrawRect(50 + widthAdjust, 140, 270 + widthAdjust, 154, 0x000000);
        ui3dsDrawStringWithNoWrapping(50 + widthAdjust, 140, 270 + widthAdjust, 154, 0x3f7fff, HALIGN_CENTER, "正在保存SRAM到SD卡...");

        nes->SaveSRAM();
    }
}


//---------------------------------------------------------
// This is called when the user chooses to save the state.
// This function should save the state into a file whose
// name contains the slot number. This will return
// true if the state is saved successfully.
//
// The slotNumbers passed in start from 1.
//---------------------------------------------------------
bool impl3dsSaveState(int slotNumber)
{
	char ext[_MAX_PATH];
    if (slotNumber == 0)
	    sprintf(ext, ".sta");
    else
	    sprintf(ext, ".st%d", slotNumber - 1);

	if (nes)
	{
		nes->SaveState(file3dsReplaceFilenameExtension(romFileNameFullPath, ext));
		return true;
	}
	else
		return false;
}


//---------------------------------------------------------
// This is called when the user chooses to load the state.
// This function should save the state into a file whose
// name contains the slot number. This will return
// true if the state is loaded successfully.
//
// The slotNumbers passed in start from 1.
//---------------------------------------------------------
bool impl3dsLoadState(int slotNumber)
{
	char ext[_MAX_PATH];
    if (slotNumber == 0)
	    sprintf(ext, ".sta");
    else
	    sprintf(ext, ".st%d", slotNumber - 1);

	if (nes)
	{
		nes->LoadState(file3dsReplaceFilenameExtension(romFileNameFullPath, ext));
		return true;
	}
	else
		return false;
}


//---------------------------------------------------------
// This function will be called everytime the user
// selects an action on the menu.
//
// Returns true if the menu should close and the game
// should resume
//---------------------------------------------------------
bool impl3dsOnMenuSelected(int ID)
{
    return false;
}



//---------------------------------------------------------
// This function will be called everytime the user
// changes the value in the specified menu item.
//
// Returns true if the menu should close and the game
// should resume
//---------------------------------------------------------
bool impl3dsOnMenuSelectedChanged(int ID, int value)
{
    if (ID == 18000)
    {
        ui3dsSetFont(value);
        return false;
    }
    if (ID == 30000)
    {
        switch (value)
        {
            case 0:
                if( nes->rom->GetDiskNo() > 0 )
                    nes->Command( NES::NESCMD_DISK_EJECT );
                return true;
                break;
            case 1:
                if( nes->rom->GetDiskNo() > 0 )
                    nes->Command( NES::NESCMD_DISK_0A );
                return true;
                break;
            case 2:
                if( nes->rom->GetDiskNo() > 1 )
                    nes->Command( NES::NESCMD_DISK_0B );
                return true;
                break;
            case 3:
                if( nes->rom->GetDiskNo() > 2 )
                    nes->Command( NES::NESCMD_DISK_1A );
                return true;
                break;
            case 4:
                if( nes->rom->GetDiskNo() > 3 )
                    nes->Command( NES::NESCMD_DISK_1B );
                return true;
                break;
            case 5:
                if( nes->rom->GetDiskNo() > 4 )
                    nes->Command( NES::NESCMD_DISK_2A );
                return true;
                break;
            case 6:
                if( nes->rom->GetDiskNo() > 5 )
                    nes->Command( NES::NESCMD_DISK_2B );
                return true;
                break;
            case 7:
                if( nes->rom->GetDiskNo() > 6 )
                    nes->Command( NES::NESCMD_DISK_3A );
                return true;
                break;
            case 8:
                if( nes->rom->GetDiskNo() > 7 )
                    nes->Command( NES::NESCMD_DISK_3B );
                return true;
                break;
        }
    }
    return false;
}


//---------------------------------------------------------
// Initializes the default global settings.
// This method is called everytime if the global settings
// file does not exist.
//---------------------------------------------------------
void impl3dsInitializeDefaultSettingsGlobal()
{
	settings3DS.GlobalVolume = 4;
	settings3DS.OtherOptions[SETTINGS_GLOBALINSERTCOIN1] = 0;
	settings3DS.OtherOptions[SETTINGS_GLOBALINSERTCOIN2] = 0;
}

//---------------------------------------------------------
// Initializes the default game-specific
// settings. This method is called everytime a game is
// loaded, but the configuration file does not exist.
//---------------------------------------------------------
void impl3dsInitializeDefaultSettingsByGame()
{
	settings3DS.MaxFrameSkips = 1;
	settings3DS.ForceFrameRate = 0;
	settings3DS.Volume = 4;

	settings3DS.OtherOptions[SETTINGS_INSERTCOIN1] = 0;
	settings3DS.OtherOptions[SETTINGS_INSERTCOIN2] = 0;
}




//----------------------------------------------------------------------
// Read/write all possible game specific settings into a file
// created in this method.
//
// This must return true if the settings file exist.
//----------------------------------------------------------------------
bool impl3dsReadWriteSettingsByGame(bool writeMode)
{
    bool success = config3dsOpenFile(file3dsReplaceFilenameExtension(romFileNameFullPath, ".cfg"), writeMode);
    if (!success)
        return false;

    config3dsReadWriteInt32("#v1\n", NULL, 0, 0);
    config3dsReadWriteInt32("# Do not modify this file or risk losing your settings.\n", NULL, 0, 0);

    // set default values first.
    if (!writeMode)
    {
        settings3DS.PaletteFix = 0;
        settings3DS.SRAMSaveInterval = 0;
    }

    int deprecated = 0;
    config3dsReadWriteInt32("Frameskips=%d\n", &settings3DS.MaxFrameSkips, 0, 4);
    config3dsReadWriteInt32("Framerate=%d\n", &settings3DS.ForceFrameRate, 0, 2);
    config3dsReadWriteInt32("TurboA=%d\n", &settings3DS.Turbo[0], 0, 10);
    config3dsReadWriteInt32("TurboB=%d\n", &settings3DS.Turbo[1], 0, 10);
    config3dsReadWriteInt32("TurboX=%d\n", &settings3DS.Turbo[2], 0, 10);
    config3dsReadWriteInt32("TurboY=%d\n", &settings3DS.Turbo[3], 0, 10);
    config3dsReadWriteInt32("TurboL=%d\n", &settings3DS.Turbo[4], 0, 10);
    config3dsReadWriteInt32("TurboR=%d\n", &settings3DS.Turbo[5], 0, 10);
    config3dsReadWriteInt32("Vol=%d\n", &settings3DS.Volume, 0, 8);
    config3dsReadWriteInt32("SRAMInterval=%d\n", &settings3DS.SRAMSaveInterval, 0, 4);
    config3dsReadWriteInt32("ButtonMapA=%d\n", &deprecated, 0, 0xffff);
    config3dsReadWriteInt32("ButtonMapB=%d\n", &deprecated, 0, 0xffff);
    config3dsReadWriteInt32("ButtonMapX=%d\n", &deprecated, 0, 0xffff);
    config3dsReadWriteInt32("ButtonMapY=%d\n", &deprecated, 0, 0xffff);
    config3dsReadWriteInt32("ButtonMapL=%d\n", &deprecated, 0, 0xffff);
    config3dsReadWriteInt32("ButtonMapR=%d\n", &deprecated, 0, 0xffff);
    config3dsReadWriteInt32("AllSprites=%d\n", &settings3DS.OtherOptions[SETTINGS_ALLSPRITES], 0, 1);

    // v1.00 options
    //
    config3dsReadWriteInt32("TurboZL=%d\n", &settings3DS.Turbo[6], 0, 10);
    config3dsReadWriteInt32("TurboZR=%d\n", &settings3DS.Turbo[7], 0, 10);
    static char const* buttonName[10] = {"A", "B", "X", "Y", "L", "R", "ZL", "ZR", "SELECT","START"};
    char buttonNameFormat[50];
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 2; ++j) {
            sprintf(buttonNameFormat, "ButtonMap%s_%d=%%d\n", buttonName[i], j);
            config3dsReadWriteInt32(buttonNameFormat, &settings3DS.ButtonMapping[i][j]);
        }
    }
    config3dsReadWriteInt32("ButtonMappingDisableFramelimitHold=%d\n", &settings3DS.ButtonHotkeyDisableFramelimit);
    config3dsReadWriteInt32("ButtonMappingOpenEmulatorMenu=%d\n", &settings3DS.ButtonHotkeyOpenMenu);
    config3dsReadWriteInt32("ButtonMappingInsertCoin1=%d\n", &settings3DS.OtherOptions[SETTINGS_INSERTCOIN1]);
    config3dsReadWriteInt32("ButtonMappingInsertCoin2=%d\n", &settings3DS.OtherOptions[SETTINGS_INSERTCOIN2]);
    config3dsReadWriteInt32("PalFix=%d\n", &settings3DS.PaletteFix, 0, 1);

    // All new options should come here!

    config3dsCloseFile();
    return true;
}


//----------------------------------------------------------------------
// Read/write all possible global specific settings into a file
// created in this method.
//
// This must return true if the settings file exist.
//----------------------------------------------------------------------
bool impl3dsReadWriteSettingsGlobal(bool writeMode)
{
    bool success = config3dsOpenFile("/emus3ds/virtuanes_3ds.cfg", writeMode);
    if (!success)
        return false;

    int deprecated = 0;

    config3dsReadWriteInt32("#v1\n", NULL, 0, 0);
    config3dsReadWriteInt32("# Do not modify this file or risk losing your settings.\n", NULL, 0, 0);

    config3dsReadWriteInt32("ScreenStretch=%d\n", &settings3DS.ScreenStretch, 0, 7);
    config3dsReadWriteInt32("HideUnnecessaryBottomScrText=%d\n", &settings3DS.HideUnnecessaryBottomScrText, 0, 1);
    config3dsReadWriteInt32("Font=%d\n", &settings3DS.Font, 0, 2);
    config3dsReadWriteInt32("NESPalette=%d\n", &settings3DS.NESPalette, 0, 7);
    config3dsReadWriteInt32("UseGlobalButtonMappings=%d\n", &settings3DS.UseGlobalButtonMappings, 0, 1);
    config3dsReadWriteInt32("UseGlobalTurbo=%d\n", &settings3DS.UseGlobalTurbo, 0, 1);
    config3dsReadWriteInt32("UseGlobalVolume=%d\n", &settings3DS.UseGlobalVolume, 0, 1);
    config3dsReadWriteInt32("TurboA=%d\n", &settings3DS.GlobalTurbo[0], 0, 10);
    config3dsReadWriteInt32("TurboB=%d\n", &settings3DS.GlobalTurbo[1], 0, 10);
    config3dsReadWriteInt32("TurboX=%d\n", &settings3DS.GlobalTurbo[2], 0, 10);
    config3dsReadWriteInt32("TurboY=%d\n", &settings3DS.GlobalTurbo[3], 0, 10);
    config3dsReadWriteInt32("TurboL=%d\n", &settings3DS.GlobalTurbo[4], 0, 10);
    config3dsReadWriteInt32("TurboR=%d\n", &settings3DS.GlobalTurbo[5], 0, 10);
    config3dsReadWriteInt32("Vol=%d\n", &settings3DS.GlobalVolume, 0, 8);
    config3dsReadWriteInt32("ButtonMapA=%d\n", &deprecated, 0, 0xffff);
    config3dsReadWriteInt32("ButtonMapB=%d\n", &deprecated, 0, 0xffff);
    config3dsReadWriteInt32("ButtonMapX=%d\n", &deprecated, 0, 0xffff);
    config3dsReadWriteInt32("ButtonMapY=%d\n", &deprecated, 0, 0xffff);
    config3dsReadWriteInt32("ButtonMapL=%d\n", &deprecated, 0, 0xffff);
    config3dsReadWriteInt32("ButtonMapR=%d\n", &deprecated, 0, 0xffff);

    // Fixes the bug where we have spaces in the directory name
    config3dsReadWriteString("Dir=%s\n", "Dir=%1000[^\n]s\n", file3dsGetCurrentDir());
    config3dsReadWriteString("ROM=%s\n", "ROM=%1000[^\n]s\n", romFileNameLastSelected);

    // v1.00 options
    //
    config3dsReadWriteInt32("AutoSavestate=%d\n", &settings3DS.AutoSavestate, 0, 1);
    config3dsReadWriteInt32("TurboZL=%d\n", &settings3DS.GlobalTurbo[6], 0, 10);
    config3dsReadWriteInt32("TurboZR=%d\n", &settings3DS.GlobalTurbo[7], 0, 10);
    static char const* buttonName[10] = {"A", "B", "X", "Y", "L", "R", "ZL", "ZR", "SELECT","START"};
    char buttonNameFormat[50];
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 2; ++j) {
            sprintf(buttonNameFormat, "ButtonMap%s_%d=%%d\n", buttonName[i], j);
            config3dsReadWriteInt32(buttonNameFormat, &settings3DS.GlobalButtonMapping[i][j]);
        }
    }
    config3dsReadWriteInt32("UseGlobalEmuControlKeys=%d\n", &settings3DS.UseGlobalEmuControlKeys, 0, 1);
    config3dsReadWriteInt32("ButtonMappingDisableFramelimitHold_0=%d\n", &settings3DS.GlobalButtonHotkeyDisableFramelimit);
    config3dsReadWriteInt32("ButtonMappingOpenEmulatorMenu_0=%d\n", &settings3DS.GlobalButtonHotkeyOpenMenu);
    config3dsReadWriteInt32("ButtonMappingInsertCoin1=%d\n", &settings3DS.OtherOptions[SETTINGS_GLOBALINSERTCOIN1]);
    config3dsReadWriteInt32("ButtonMappingInsertCoin2=%d\n", &settings3DS.OtherOptions[SETTINGS_GLOBALINSERTCOIN2]);

    // All new options should come here!
    config3dsReadWriteInt32("Disable3DSlider=%d\n", &settings3DS.Disable3DSlider, 0, 1);
    config3dsReadWriteInt32("GameScreen=%d\n", &settings3DS.GameScreen, 0, 1);

    config3dsCloseFile();
    return true;
}



//----------------------------------------------------------------------
// Apply settings into the emulator.
//
// This method normally copies settings from the settings3DS struct
// and updates the emulator's core's configuration.
//
// This must return true if any settings were modified.
//----------------------------------------------------------------------
bool impl3dsApplyAllSettings(bool updateGameSettings)
{
    bool settingsChanged = false;

    // Update color palette
    nespalInitialize(settings3DS.NESPalette);
    // Must also update this for DuckHunt, Castlevania, MegaMan2, DrMario,...
    PAL_Changed = true;

    // update screen stretch
    //
    if (settings3DS.ScreenStretch == 0)
    {
        settings3DS.StretchWidth = 256;
        settings3DS.StretchHeight = 240;    // Actual height
        settings3DS.CropPixels = 0;
    }
    else if (settings3DS.ScreenStretch == 1)
    {
        // Added support for 320x240 (4:3) screen ratio
        settings3DS.StretchWidth = 320;
        settings3DS.StretchHeight = 240;
        settings3DS.CropPixels = 0;
    }
    else if (settings3DS.ScreenStretch == 2)
    {
        settings3DS.StretchWidth = 400;
        settings3DS.StretchHeight = 240;
        settings3DS.CropPixels = 0;
    }

    // Update the screen font
    //
    ui3dsSetFont(settings3DS.Font);

    // update global volume
    //
    if (settings3DS.Volume < 0)
        settings3DS.Volume = 0;
    if (settings3DS.Volume > 8)
        settings3DS.Volume = 8;
    if (settings3DS.GlobalVolume < 0)
        settings3DS.GlobalVolume = 0;
    if (settings3DS.GlobalVolume > 8)
        settings3DS.GlobalVolume = 8;

    int vol[9] = { 100, 125, 150, 175, 200, 250, 300, 350, 400 };
    Config.sound.nVolume[0] = vol[settings3DS.Volume];
    if (settings3DS.UseGlobalVolume)
        Config.sound.nVolume[0] = vol[settings3DS.GlobalVolume];

    if (updateGameSettings)
    {
        if (settings3DS.ForceFrameRate == 0)
            settings3DS.TicksPerFrame = TICKS_PER_SEC / impl3dsGetROMFrameRate();

        if (settings3DS.ForceFrameRate == 1)
            settings3DS.TicksPerFrame = TICKS_PER_FRAME_PAL;

        else if (settings3DS.ForceFrameRate == 2)
            settings3DS.TicksPerFrame = TICKS_PER_FRAME_NTSC;

        Config.graphics.bAllSprite = settings3DS.OtherOptions[SETTINGS_ALLSPRITES];
    }

    return settingsChanged;
}


//----------------------------------------------------------------------
// Copy values from menu to settings3DS structure,
// or from settings3DS structure to the menu, depending on the
// copyMenuToSettings parameter.
//
// This must return return if any of the settings were changed.
//----------------------------------------------------------------------
bool impl3dsCopyMenuToOrFromSettings(bool copyMenuToSettings)
{
#define UPDATE_SETTINGS(var, tabIndex, ID)  \
    { \
    if (copyMenuToSettings && (var) != menu3dsGetValueByID(tabIndex, ID)) \
    { \
        var = menu3dsGetValueByID(tabIndex, (ID)); \
        settingsUpdated = true; \
    } \
    if (!copyMenuToSettings) \
    { \
        menu3dsSetValueByID(tabIndex, (ID), (var)); \
    } \
    }

    bool settingsUpdated = false;
    UPDATE_SETTINGS(settings3DS.Font, -1, 18000);
    UPDATE_SETTINGS(settings3DS.ScreenStretch, -1, 11000);
    UPDATE_SETTINGS(settings3DS.GameScreen, -1, 15000);
    UPDATE_SETTINGS(settings3DS.HideUnnecessaryBottomScrText, -1, 15001);
    UPDATE_SETTINGS(settings3DS.MaxFrameSkips, -1, 10000);
    UPDATE_SETTINGS(settings3DS.ForceFrameRate, -1, 12000);
    UPDATE_SETTINGS(settings3DS.UseGlobalButtonMappings, -1, 20000);
    UPDATE_SETTINGS(settings3DS.UseGlobalTurbo, -1, 20001);
    UPDATE_SETTINGS(settings3DS.UseGlobalVolume, -1, 20002);
    UPDATE_SETTINGS(settings3DS.AutoSavestate, -1, 21000);
    UPDATE_SETTINGS(settings3DS.Disable3DSlider, -1, 21001);
    UPDATE_SETTINGS(settings3DS.NESPalette, -1, 69696);

    UPDATE_SETTINGS(settings3DS.UseGlobalEmuControlKeys, -1, 50003);
    if (settings3DS.UseGlobalButtonMappings || copyMenuToSettings)
    {
        for (int i = 0; i < 2; i++)
            for (int b = 0; b < 10; b++)
                UPDATE_SETTINGS(settings3DS.GlobalButtonMapping[b][i], -1, 13010 + b + (i * 10));
    }
    if (!settings3DS.UseGlobalButtonMappings || copyMenuToSettings)
    {
        for (int i = 0; i < 2; i++)
            for (int b = 0; b < 10; b++)
                UPDATE_SETTINGS(settings3DS.ButtonMapping[b][i], -1, 13010 + b + (i * 10));
    }
    if (settings3DS.UseGlobalTurbo || copyMenuToSettings)
    {
        for (int b = 0; b < 8; b++)
            UPDATE_SETTINGS(settings3DS.GlobalTurbo[b], -1, 13000 + b);
    }
    if (!settings3DS.UseGlobalTurbo || copyMenuToSettings)
    {
        for (int b = 0; b < 8; b++)
            UPDATE_SETTINGS(settings3DS.Turbo[b], -1, 13000 + b);
    }
    if (settings3DS.UseGlobalVolume || copyMenuToSettings)
    {
        UPDATE_SETTINGS(settings3DS.GlobalVolume, -1, 14000);
    }
    if (!settings3DS.UseGlobalVolume || copyMenuToSettings)
    {
        UPDATE_SETTINGS(settings3DS.Volume, -1, 14000);
    }
    if (settings3DS.UseGlobalEmuControlKeys || copyMenuToSettings)
    {
        UPDATE_SETTINGS(settings3DS.GlobalButtonHotkeyOpenMenu, -1, 23001);
        UPDATE_SETTINGS(settings3DS.GlobalButtonHotkeyDisableFramelimit, -1, 23002);
        UPDATE_SETTINGS(settings3DS.OtherOptions[SETTINGS_GLOBALINSERTCOIN1], -1, 23003);
        UPDATE_SETTINGS(settings3DS.OtherOptions[SETTINGS_GLOBALINSERTCOIN2], -1, 23004);
    }
    if (!settings3DS.UseGlobalEmuControlKeys || copyMenuToSettings)
    {
        UPDATE_SETTINGS(settings3DS.ButtonHotkeyOpenMenu, -1, 23001);
        UPDATE_SETTINGS(settings3DS.ButtonHotkeyDisableFramelimit, -1, 23002);
        UPDATE_SETTINGS(settings3DS.OtherOptions[SETTINGS_INSERTCOIN1], -1, 23003);
        UPDATE_SETTINGS(settings3DS.OtherOptions[SETTINGS_INSERTCOIN1], -1, 23004);
    }

    UPDATE_SETTINGS(settings3DS.OtherOptions[SETTINGS_ALLSPRITES], -1, 19000);     // sprite flicker

    return settingsUpdated;
}



//----------------------------------------------------------------------
// Clears all cheats from the core.
//
// This method is called only when cheats are loaded.
// This only happens after a new ROM is loaded.
//----------------------------------------------------------------------
void impl3dsClearAllCheats()
{
    if (nes)
        nes->GenieInitial();
}


//----------------------------------------------------------------------
// Adds cheats into the emulator core after being loaded up from
// the .CHX file.
//
// This method is called only when cheats are loaded.
// This only happens after a new ROM is loaded.
//
// This method must return true if the cheat code format is valid,
// and the cheat is added successfully into the core.
//----------------------------------------------------------------------
bool impl3dsAddCheat(bool cheatEnabled, char *name, char *code)
{
    return nes->GenieAdd(cheatEnabled, code);
}


//----------------------------------------------------------------------
// Enable/disables a cheat in the emulator core.
//
// This method will be triggered when the user enables/disables
// cheats in the cheat menu.
//----------------------------------------------------------------------
void impl3dsSetCheatEnabledFlag(int cheatIdx, bool enabled)
{
    nes->GenieSet(cheatIdx, enabled);
}
