#include <stdlib.h>
#include <3ds.h>

#include "3dsexit.h"

aptHookCookie hookCookie;
int appExiting = 0;

void handleAptHook(APT_HookType hook, void* param)
{
    if (hook == APTHOOK_ONEXIT) {
        appExiting = 1;
    }
}

void enableAptHooks() {
    aptHook(&hookCookie, handleAptHook, NULL);
}

void disableAptHooks() {
    aptUnhook(&hookCookie);
}
