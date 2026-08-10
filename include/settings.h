#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdint.h>
#include "lang.h"

struct settings {
    enum lang language;
    uint32_t  fg;  
    uint32_t  bg;
};

extern struct settings settings;   

void settings_init(void);          

#endif