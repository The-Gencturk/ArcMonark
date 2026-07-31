#include "settings.h"
#include "config.h"

struct settings settings;   

void settings_init(void) {
    settings.language = DEFAULT_LANG;
    settings.fg = THEME_FG;
    settings.bg = THEME_BG;
}