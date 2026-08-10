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
#include "vmm.h"
#include "heap.h"
#include "serial.h"
#include "string.h"
#include "sched.h"
#include "gfx.h"
#include "mouse.h"
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

static void cmd_mem(void) {
    char buf[21];
    uint64_t free_mib = pmm_free_frames() * PAGE_SIZE / (1024 * 1024);

    print("total frames : "); print(u64_to_dec(pmm_total_frames(), buf)); putchar('\n');
    print("used  frames : "); print(u64_to_dec(pmm_used_frames(),  buf)); putchar('\n');
    print("free  frames : "); print(u64_to_dec(pmm_free_frames(),  buf)); putchar('\n');
    print("free  ram MiB: "); print(u64_to_dec(free_mib,           buf)); putchar('\n');
    print("heap  used  B: "); print(u64_to_dec(heap_used_bytes(),  buf)); putchar('\n');
}


static volatile int demo_finished;

static void demo_worker(void* arg) {
    uint64_t id = (uint64_t)arg;
    char buf[21];
    for (int i = 0; i < 5; i++) {
        print("  [task "); print(u64_to_dec(id, buf));
        print("] tick ");  print(u64_to_dec((uint64_t)i, buf)); putchar('\n');
        task_yield();
    }
    demo_finished++;
    task_exit();
}

static void cmd_threads(void) {
    demo_finished = 0;
    print("2 kooperatif task baslatiliyor...\n");
    task_create(demo_worker, (void*)1);
    task_create(demo_worker, (void*)2);
    while (demo_finished < 2)     // kabuk task'i round-robin'i surer
        task_yield();
    print("tum tasklar bitti\n");
}


static void cmd_gfxdemo(void) {
    uint32_t w = gfx_width(), h = gfx_height();
    uint32_t bw = 140, bh = 40;
    int x = 2, dx = 7;

    int y = (int)(h / 2 - bh / 2);

    for (int frame = 0; frame < 300; frame++) {
        gfx_clear(GFX_RGB(15, 15, 25));
        gfx_fill_rect((uint32_t)x, (uint32_t)y, bw, bh, GFX_RGB(40, 160, 220));
        gfx_rect((uint32_t)x, (uint32_t)y, bw, bh, GFX_RGB(255, 255, 255));
        gfx_text("cift tampon", (uint32_t)x + 10, (uint32_t)y + bh / 2 - 8,
                 GFX_RGB(255, 255, 255), GFX_RGB(40, 160, 220));
        gfx_present();
        x += dx;
        if (x <= 0 || x + (int)bw >= (int)w) dx = -dx;
        sleep(16);
    }
    print("gfxdemo bitti\n");
}

// --- fare demosu (Faz 3) ----------------------------------------------------
// Klasik ok imleci; 'X'=siyah kenar, 'o'=beyaz dolgu, ' '=saydam.
static const char* const cursor_glyph[] = {
    "X           ",
    "XX          ",
    "XoX         ",
    "XooX        ",
    "XoooX       ",
    "XooooX      ",
    "XoooooX     ",
    "XooooooX    ",
    "XoooooooX   ",
    "XoooooXXXX  ",
    "XooXooX     ",
    "XoX XooX    ",
    "XX   XooX   ",
    "X     XooX  ",
    "       XX   ",
    0
};

static void draw_cursor(int cx, int cy, uint8_t buttons) {
    uint32_t fill = (buttons & MOUSE_LEFT) ? GFX_RGB(255, 210, 0)
                                           : GFX_RGB(255, 255, 255);
    for (int r = 0; cursor_glyph[r]; r++) {
        const char* row = cursor_glyph[r];
        for (int c = 0; row[c]; c++) {
            if (row[c] == 'X')      gfx_pixel((uint32_t)(cx + c), (uint32_t)(cy + r), GFX_RGB(0, 0, 0));
            else if (row[c] == 'o') gfx_pixel((uint32_t)(cx + c), (uint32_t)(cy + r), fill);
        }
    }
}

// Fareyi oynat -> imleç takip etsin; tus göstergeleri + x/y. Bir tusa basinca cikar.
static void cmd_mouse(void) {
    uint32_t w = gfx_width();
    char buf[21];
    print("fareyi oynat; cikmak icin bir tusa bas\n");
    while (!keyboard_haskey()) {
        int mx = mouse_get_x(), my = mouse_get_y();
        uint8_t b = mouse_get_buttons();

        gfx_clear(GFX_RGB(25, 25, 40));
        gfx_fill_rect(0, 0, w, 18, GFX_RGB(50, 50, 80));          // ust bilgi cubugu
        gfx_text("x=", 6, 1, GFX_RGB(255, 255, 255), GFX_RGB(50, 50, 80));
        gfx_text(u64_to_dec((uint64_t)mx, buf), 24, 1, GFX_RGB(255, 255, 255), GFX_RGB(50, 50, 80));
        gfx_text("y=", 96, 1, GFX_RGB(255, 255, 255), GFX_RGB(50, 50, 80));
        gfx_text(u64_to_dec((uint64_t)my, buf), 114, 1, GFX_RGB(255, 255, 255), GFX_RGB(50, 50, 80));
        gfx_fill_rect(200, 2, 14, 14, (b & MOUSE_LEFT)   ? GFX_RGB(0, 220, 0) : GFX_RGB(90, 90, 90));
        gfx_fill_rect(218, 2, 14, 14, (b & MOUSE_MIDDLE) ? GFX_RGB(0, 220, 0) : GFX_RGB(90, 90, 90));
        gfx_fill_rect(236, 2, 14, 14, (b & MOUSE_RIGHT)  ? GFX_RGB(0, 220, 0) : GFX_RGB(90, 90, 90));

        draw_cursor(mx, my, b);
        gfx_present();
        sleep(10);
    }
    (void)keyboard_getchar();   // cikis tusunu yut
    print("mouse demo bitti\n");
}

static void run_command(const char* cmd) {
    const char* arg;
    if (cmd[0] == '\0')                          return;
    else if (streq(cmd, "help")) { print(msg(MSG_HELP_LIST)); putchar('\n'); }
    else if (streq(cmd, "clear"))                clear_screen();
    else if (streq(cmd, "about")) { print(msg(MSG_ABOUT)); putchar('\n'); }
    else if (streq(cmd, "mem"))                  cmd_mem();
    else if (streq(cmd, "threads"))              cmd_threads();
    else if (streq(cmd, "gfxdemo"))              cmd_gfxdemo();
    else if (streq(cmd, "mouse"))                cmd_mouse();
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
    mouse_init(fb_width(), fb_height());
    pmm_init();
    vmm_init();
    heap_init();

    sched_init();
    asm volatile("sti");

    print(msg(MSG_WELCOME));   putchar('\n');
    print(msg(MSG_HELP_HINT)); print("\n\n");

    serial_init();
    serial_write("[ArcMonark] seri port hazir\n");
    serial_write_hex(0xDEADBEEFCAFE);
    serial_putc('\n');

    gfx_init(fb_address(), fb_width(), fb_height(), fb_pitch(), fb_bpp());
    if (gfx_backbuffer_init() != 0)
        serial_write("[gfx] arka tampon ayrilamadi, tek tampon modu\n");
    gfx_clear(GFX_RGB(0, 0, 30));                     
    uint32_t cx = gfx_width() / 2, cy = gfx_height() / 2;
    gfx_fill_rect(cx - 100, cy - 40, 200, 80, GFX_RGB(40, 120, 5));
    gfx_rect(cx - 100, cy - 40, 200, 80, GFX_RGB(20, 255, 8));
    gfx_text("ArcMonark", cx - 50, cy - 8, GFX_RGB(255, 50, 255), GFX_RGB(40, 50, 200));
    gfx_present(); 


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


