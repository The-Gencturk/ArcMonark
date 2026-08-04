#include "keyboard.h"
#include "config.h"
#include "settings.h"
#include "lang.h"
#include "idt.h"
#include "io.h"
#include "fb.h"
#include "pic.h"
#include "timer.h"
#include "pmm.h"

//cd /mnt/c/Users/LENOVO/source/repos/ArcMonark

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

static void Close_system() {
outw(0x604, 0x2000);
outw(0xB004, 0x2000);
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
    else if (streq(cmd, "close")) Close_system();
    else { print(msg(MSG_UNKNOWN_CMD)); print(cmd); putchar('\n'); }
}

static void hcf(void) {
    for (;;) __asm__ volatile ("hlt");
}

void kmain(void) {
    settings_init();
    set_color(settings.fg, settings.bg);
    clear_screen();
    idt_init();
    pic_remap();
    pit_init(100);
    idt_set_gate(0x20, (void*)irq0_timer, IDT_INT_GATE);
    irq_clear_mask(0);
    keyboard_init();
    pmm_init();
    asm volatile("sti");

    print(msg(MSG_WELCOME));   putchar('\n');
    print(msg(MSG_HELP_HINT)); print("\n\n");

    char b[21];

    print("PMM: total="); print(u64_to_dec(pmm_total_frames(), b));
    print(" used=");       print(u64_to_dec(pmm_used_frames(), b));
    print(" free=");       print(u64_to_dec(pmm_free_frames(), b));
    print("\n");

    char line[LINE_MAX];
    while (1) {
        color(COLOR_RED);
        print(PROMPT);
        color(COLOR_WHITE);
        read_line(line, LINE_MAX);
        run_command(line);
    }
}

void _start(void) {
    if (!fb_init())   
        hcf();
    kmain();
    hcf();          
}


