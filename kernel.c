#include "vga.h"
#include "keyboard.h"
#include "config.h"
#include "settings.h"
#include "lang.h"

static int streq(const char* a, const char* b) {
    while (*a && *b) { if (*a != *b) return 0; a++; b++; }
    return *a == *b;
}
static const char* starts_with(const char* s, const char* prefix) {
    while (*prefix) { if (*s++ != *prefix++) return 0; }
    return s;
}

static void cmd_lang(const char* arg) {
    if (streq(arg, "tr")) settings.language = LANG_TR;
    else if (streq(arg, "en")) settings.language = LANG_EN;
    else { print("?\n"); return; }
    print(msg(MSG_LANG_SET)); putchar('\n');
}

static void cmd_theme(const char* arg) {
    if (streq(arg, "dark")) { settings.fg = DARK_FG;  settings.bg = DARK_BG; }
    else if (streq(arg, "light")) { settings.fg = LIGHT_FG; settings.bg = LIGHT_BG; }
    else { print("?\n"); return; }
    set_color(settings.fg, settings.bg);   
    clear_screen();                        
    print(msg(MSG_THEME_SET)); putchar('\n');
}

static void run_command(const char* cmd) {
    const char* arg;
    if (cmd[0] == '\0')                          return;
    else if (streq(cmd, "help")) { print(msg(MSG_HELP_LIST)); putchar('\n'); }
    else if (streq(cmd, "clear"))                clear_screen();
    else if (streq(cmd, "about")) { print(msg(MSG_ABOUT)); putchar('\n'); }
    else if ((arg = starts_with(cmd, "lang ")))  cmd_lang(arg);
    else if ((arg = starts_with(cmd, "theme "))) cmd_theme(arg);
    else { print(msg(MSG_UNKNOWN_CMD)); print(cmd); putchar('\n'); }
}

void kmain(void) {
    settings_init();                       
    set_color(settings.fg, settings.bg);   
    clear_screen();

    print(msg(MSG_WELCOME));   putchar('\n');
    print(msg(MSG_HELP_HINT)); print("\n\n");

    char line[LINE_MAX];
    while (1) {
        print(PROMPT);
        read_line(line, LINE_MAX);
        run_command(line);
    }
}