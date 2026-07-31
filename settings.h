#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdint.h>
#include "lang.h"

struct settings {
    enum lang language;
    uint32_t  fg;   /* ARGB framebuffer rengi */
    uint32_t  bg;
};

extern struct settings settings;   // tek global canli durum

void settings_init(void);          // config.h varsayilanlariyla doldurur

#endif