#include "3dsexit.h"
#include "3dsmain.h"
#include "3dsemu.h"
#include "3dssound.h"
#include "3dsinterface.h"

aptHookCookie hookCookie;
int appExiting = 0;
int appSuspended = 0;

void handleAptHook(APT_HookType hook, void* param)
{
    switch(hook) {
        case APTHOOK_ONEXIT:
            appExiting = 1;
            break;
        case APTHOOK_ONSUSPEND:
        case APTHOOK_ONSLEEP:
            appSuspended = 1;
            if (emulator.emulatorState == EMUSTATE_EMULATE) {
                impl3dsEmulationPaused();
                snd3dsStopPlaying();
                snd3dsFinalize();
            }
            break;
        case APTHOOK_ONRESTORE:
        case APTHOOK_ONWAKEUP:
            appSuspended = 1;
            break;
    }
}

void enableAptHooks() {
    aptHook(&hookCookie, handleAptHook, NULL);
}

void disableAptHooks() {
    aptUnhook(&hookCookie);
}
