#ifndef _3DSTHEMES_H_
#define _3DSTHEMES_H_

#include <stdint.h>

typedef struct 
{
    uint32_t menuTopBarColor;
    uint32_t selectedTabTextColor;
    uint32_t tabTextColor;
    uint32_t selectedTabIndicatorColor;
    uint32_t menuBottomBarColor;
    uint32_t menuBottomBarTextColor;
    uint32_t menuBackColor;
    uint32_t selectedItemBackColor;
    uint32_t selectedItemTextColor;
    uint32_t selectedItemDescriptionTextColor;
    uint32_t normalItemTextColor;
    uint32_t normalItemDescriptionTextColor;
    uint32_t disabledItemTextColor;
    uint32_t headerItemTextColor;
    uint32_t subtitleTextColor;
    uint32_t accentColor;
    uint32_t accentUnselectedColor;
    int dialogColorInfo; // DIALOGCOLOR_CYAN
    int dialogColorWarn; // DIALOGCOLOR_RED
    int dialogColorSuccess; // DIALOGCOLOR_GREEN
    float dialogTextAlpha;
    float dialogSelectedItemBackAlpha;
} Theme3ds;

#define THEME_ORIGINAL 0
#define THEME_DARK_MODE 1

#define TOTALTHEMECOUNT 2
extern Theme3ds Themes[TOTALTHEMECOUNT];

#endif
