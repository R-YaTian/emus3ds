
#ifndef _3DSMAIN_H_
#define _3DSMAIN_H_


extern char romFileNameFullPath[];
extern char romFileNameLastSelected[];

//---------------------------------------------------------
// The different emulator states.
//---------------------------------------------------------
#define EMUSTATE_EMULATE        1
#define EMUSTATE_PAUSEMENU      2
#define EMUSTATE_END            3

//---------------------------------------------------------
// FPS behavior
//---------------------------------------------------------
#define FPS_WAIT_FULL 0
#define FPS_WAIT_HALF 1
#define FPS_WAIT_NONE 2



#endif
