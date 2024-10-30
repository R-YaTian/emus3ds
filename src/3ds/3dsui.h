#ifndef _3DSUI_H_
#define _3DSUI_H_

#include "3dstypes.h"
#include "3dsthemes.h"

//---------------------------------------------------------------
// Initialize this library
//---------------------------------------------------------------
void ui3dsInitialize();


//---------------------------------------------------------------
// Get the target screen width
//---------------------------------------------------------------
int ui3dsGetScreenWidth(gfxScreen_t targetScreen);


//---------------------------------------------------------------
// this is called on init and if game screen has swapped
//---------------------------------------------------------------
void ui3dsUpdateScreenSettings(gfxScreen_t gameScreen);


//---------------------------------------------------------------
// Sets the font to be used to display text to the screen.
//---------------------------------------------------------------
void ui3dsSetFont(int fontIndex);


//---------------------------------------------------------------
// Sets the menu theme.
//---------------------------------------------------------------
void ui3dsSetTheme(int themeIndex);


//---------------------------------------------------------------
// Sets the global viewport for all drawing
//---------------------------------------------------------------
void ui3dsSetViewport(int x1, int y1, int x2, int y2);


//---------------------------------------------------------------
// Push the global viewport for all drawing
//---------------------------------------------------------------
void ui3dsPushViewport(int x1, int y1, int x2, int y2);


//---------------------------------------------------------------
// Pop the global viewport
//---------------------------------------------------------------
void ui3dsPopViewport();


//---------------------------------------------------------------
// Sets the global translate for all drawing.
//---------------------------------------------------------------
void ui3dsSetTranslate(int tx, int ty);


//---------------------------------------------------------------
// Colors are in the 888 (RGB) format.
//---------------------------------------------------------------
void ui3dsSetColor(int newForeColor, int newBackColor);


//---------------------------------------------------------------
// Multiplies the alpha to the color in RGBA8 format,
// and returns the results.
//---------------------------------------------------------------
int ui3dsApplyAlphaToColor(int color, float alpha);


//---------------------------------------------------------------
// overlay blending mode: returns a color in RGB888 format
// may have more performance impact than simple blending mode
//---------------------------------------------------------------
int ui3dsOverlayBlendColor(int backgroundColor, int foregroundColor);


//---------------------------------------------------------------
// Draws a rectangle with the back colour
//
// Note: x0,y0 are inclusive. x1,y1 are exclusive.
//---------------------------------------------------------------
void ui3dsDrawRect(int x0, int y0, int x1, int y1);


//---------------------------------------------------------------
// Draws a rectangle with the colour (in RGB888 format).
//
// Note: x0,y0 are inclusive. x1,y1 are exclusive.
//---------------------------------------------------------------
void ui3dsDrawRect(int x0, int y0, int x1, int y1, int color, float alpha = 1.0f);


//---------------------------------------------------------------
// Draws a string with the forecolor, with wrapping
//---------------------------------------------------------------
void ui3dsDrawStringWithWrapping(int x0, int y0, int x1, int y1, int color, int horizontalAlignment, const char *buffer);


//---------------------------------------------------------------
// Draws a string with the forecolor, with no wrapping
//---------------------------------------------------------------
void ui3dsDrawStringWithNoWrapping(int x0, int y0, int x1, int y1, int color, int horizontalAlignment, const char *buffer);


//---------------------------------------------------------------
// Copies pixel data from the frame buffer to another buffer
//---------------------------------------------------------------
void ui3dsCopyFromFrameBuffer(uint16 *destBuffer);


//---------------------------------------------------------------
// Copies pixel data from the buffer to the framebuffer
//---------------------------------------------------------------
void ui3dsBlitToFrameBuffer(uint16 *srcBuffer, float alpha = 1.0f);


#define HALIGN_LEFT     -1
#define HALIGN_CENTER   0
#define HALIGN_RIGHT    1


#endif
