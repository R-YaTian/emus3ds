#ifndef INPUTREDIRECT_H
#define INPUTREDIRECT_H

#include <sys/socket.h>

#define PORTIRED 4950

//#define IRDEBUG 1
//#define IRINITSCREEN 1
#define IRINITSOCKET 1
//#define IRFAILEXIT 1

// first byte
#define IRED_A 0x0001
#define IRED_B 0x0002
#define IRED_SELECT 0x0004
#define IRED_START 0x0008
#define IRED_RIGHT 0x0010
#define IRED_LEFT 0x0020
#define IRED_UP 0x0040
#define IRED_DOWN 0x0080

// second byte
#define IRED_R 0x0100
#define IRED_L 0x0200
#define IRED_X 0x0400
#define IRED_Y 0x0800
#define IRED_ZR 0x1000
#define IRED_ZL 0x2000

typedef struct {
  u16 buttons;
  u16 cPadAxis1;
  u16 cPadAxis2;
  u8 cStickAxis1;
  u8 cStickAxis2;
} ControlIRED;

void recvUDPPackIRED(int sock, ControlIRED *control);
int createUDPIRED();
void finishSysIRED();
int initSysIRED();

#endif

