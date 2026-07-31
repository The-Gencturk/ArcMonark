#ifndef CONFIG_H
#define CONFIG_H

#include "lang.h"


#define DEFAULT_LANG   LANG_TR
/* Framebuffer artik 0xAARRGGBB (ARGB) renk kullaniyor, VGA index degil. */
#define THEME_FG       0xFFFFFFFF   /* beyaz */
#define THEME_BG       0xFF000000   /* siyah */


#define DARK_FG  0xFFE0E0E0
#define DARK_BG  0xFF101010
#define LIGHT_FG 0xFF202020
#define LIGHT_BG 0xFFF0F0F0
#define COLOR_BLACK   0xFF000000
#define COLOR_WHITE   0xFFFFFFFF
#define COLOR_RED     0xFFFF0000
#define COLOR_GREEN   0xFF00FF00
#define COLOR_BLUE    0xFF0000FF
#define COLOR_YELLOW  0xFFFFFF00
#define COLOR_CYAN    0xFF00FFFF
#define COLOR_MAGENTA 0xFFFF00FF
#define COLOR_GRAY    0xFF808080

#define PROMPT     "Arc> "
#define LINE_MAX   128

#endif