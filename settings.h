#ifndef SETTINGS_H
#define SETTINGS_H

#include "lang.h"

struct settings {
    enum lang     language;
    unsigned char fg;
    unsigned char bg;
};

extern struct settings settings;   // tek global canli durum

void settings_init(void);          // config.h varsayilanlariyla doldurur

#endif