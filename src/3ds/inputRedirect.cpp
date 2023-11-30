#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>

#include <fcntl.h>

#include <sys/types.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <3ds.h>

#include "inputRedirect.h"

#define SOC_ALIGN       0x1000
//#define SOC_BUFFERSIZE  0x100000
//#define SOC_BUFFERSIZE  (1024 * 128)
#define SOC_BUFFERSIZE  (1024 * 1024)

static u32 *SOC_buffer = NULL;

#ifdef IRFAILEXIT
__attribute__((format(printf,1,2)))
void failExit(const char *fmt, ...);

void failExit(const char *fmt, ...) {
        va_list ap;

#ifdef IRDEBUG
        printf(CONSOLE_RED);
#endif
        va_start(ap, fmt);
#ifdef IRDEBUG
        vprintf(fmt, ap);
#endif
        va_end(ap);
#ifdef IRDEBUG
        printf(CONSOLE_RESET);
        printf("\nPress B to exit\n");
#endif

        while (aptMainLoop()) {
                gspWaitForVBlank();
                hidScanInput();

                u32 kDown = hidKeysDown();
                if (kDown & KEY_B) exit(0);
        }
}
#else
#define failExit(...) return -1;
#endif

ControlIRED lastControl = {.buttons = 0, .cPadAxis1 = 0x7FF, .cPadAxis2 = 0x7FF, .cStickAxis1 = 0x80, .cStickAxis2 = 0x80};
void recvUDPPackIRED(int sock, ControlIRED *control) {
    struct sockaddr_in senderaddr;
    socklen_t len = sizeof(struct sockaddr_in);
    u8 pack[32];

    int n = recvfrom(sock, (void *)pack, 32,
            0, ( struct sockaddr *) &senderaddr,
            &len);

    while (n >= 20) {
      u8 tmp[32];
      int recv = recvfrom(sock, (void *)tmp, 32,
              MSG_PEEK, ( struct sockaddr *) &senderaddr,
              &len);

      if (recv != n) break;
      if (memcmp(pack, tmp, n) != 0) break;

      recvfrom(sock, (void *)tmp, 32,
              0, ( struct sockaddr *) &senderaddr,
              &len);
    }

    if (n >= 20) {
      control->buttons = (pack[0] ^ 0XFF) | ((pack[1] ^ 0x0F) << 8);

      u32 cPadData = pack[8] | (pack[9] << 8) | (pack[10] << 16);
      control->cPadAxis1 = cPadData & 0xFFF;
      control->cPadAxis2 = (cPadData >> 12) & 0xFFF;

      if (pack[13] & 0x02) control->buttons |= IRED_ZR;
      if (pack[13] & 0x04) control->buttons |= IRED_ZL;

      control->cStickAxis1 = pack[14];
      control->cStickAxis2 = pack[15];

#ifdef IRDEBUG
      if (control->buttons & IRED_UP) printf("UP ");
      if (control->buttons & IRED_DOWN) printf("DOWN ");
      if (control->buttons & IRED_LEFT) printf("LEFT ");
      if (control->buttons & IRED_RIGHT) printf("RIGHT ");
      if (control->buttons & IRED_B) printf("B ");
      if (control->buttons & IRED_A) printf("A ");
      if (control->buttons & IRED_START) printf("START ");
      if (control->buttons & IRED_SELECT) printf("SELECT ");
      if (control->buttons & IRED_Y) printf("Y ");
      if (control->buttons & IRED_X) printf("X ");
      if (control->buttons & IRED_L) printf("L ");
      if (control->buttons & IRED_R) printf("R ");
      if (control->buttons & IRED_ZL) printf("ZL ");
      if (control->buttons & IRED_ZR) printf("ZR ");
      if (control->buttons) printf("\n");
      if ((control->cPadAxis1 != 0x7FF) || (control->cPadAxis2 != 0x7FF)) printf("Axis1: %d    Axis2: %d\n", control->cPadAxis1, control->cPadAxis2);
      if ((control->cStickAxis1 != 0x80) || (control->cStickAxis2 != 0x80)) printf("cAxis1: %d    cAxis2: %d\n", control->cStickAxis1, control->cStickAxis2);
#endif
      lastControl = *control;
    } else {
      *control = lastControl;
    }
}

int createUDPIRED(void) {
    int sockfd;
    struct sockaddr_in myaddr;

    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        failExit("socket creation failed");
    }

    memset(&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family    = AF_INET; // IPv4
    myaddr.sin_addr.s_addr = INADDR_ANY;
    myaddr.sin_port = htons(PORTIRED);

    // Bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *)&myaddr,
         sizeof(myaddr)) < 0 )
    {
        close(sockfd);
        failExit("bind filed");
    }

    fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK);
    return sockfd;
}

static void initScreenIRED(void) {
        // Initialize services
        gfxInitDefault();

        //Initialize console on top screen. Using NULL as the second argument tells the console library to use the internal console structure as current one
        consoleInit(GFX_TOP, NULL);
}

static void finishScreenIRED(void) {
        gfxExit();
}

static int initSocketsIRED(void) {
        SOC_buffer = (u32*)memalign(SOC_ALIGN, SOC_BUFFERSIZE);

        if(SOC_buffer == NULL) {
                failExit("memalign: failed to allocate\n");
        }

        // Now intialise soc:u service
        int ret = socInit(SOC_buffer, SOC_BUFFERSIZE);
        if (ret != 0) {
                failExit("socInit: 0x%08X\n", (unsigned int)ret);
        }

	return 1;
}

static void finishSocketsIRED(void) {
#ifdef IRDEBUG
        printf("waiting for socExit...\n");
#endif
        socExit();
}

void finishSysIRED(void) {
#ifdef IRINITSOCKET
        finishSocketsIRED();
#endif
#ifdef IRINITSCREEN
        finishScreenIRED();
#endif
}

int initSysIRED(void) {
#ifdef IRFAILEXIT
        atexit(finishSysIRED);
#endif

#ifdef IRINITSCREEN
        initScreenIRED();
#endif
#ifdef IRINITSOCKET
        if (initSocketsIRED() == -1) return -1;
#endif

#ifdef IRDEBUG
//      printf("\x1b[1;1HPress Start to exit.");
        printf("Press Start to exit.\n");
#endif

  return 1;
}
