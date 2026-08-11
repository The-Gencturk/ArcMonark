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
#include "event.h"

// --- kabuk komutlari --------------------------------------------------------
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

static void cmd_mem(void) {
    char buf[21];
    uint64_t free_mib = pmm_free_frames() * PAGE_SIZE / (1024 * 1024);
    print("total frames : "); print(u64_to_dec(pmm_total_frames(), buf)); putchar('\n');
    print("used  frames : "); print(u64_to_dec(pmm_used_frames(),  buf)); putchar('\n');
    print("free  frames : "); print(u64_to_dec(pmm_free_frames(),  buf)); putchar('\n');
    print("free  ram MiB: "); print(u64_to_dec(free_mib,           buf)); putchar('\n');
    print("heap  used  B: "); print(u64_to_dec(heap_used_bytes(),  buf)); putchar('\n');
}

static void cmd_close(void) {   // ACPI soft-off (QEMU)
    outw(0x604, 0x2000);
    outw(0xB004, 0x2000);
}

// --- fare (imleç) -----------------------------------------------------------
// Klasik ok imleci; 'X'=siyah kenar, 'o'=beyaz dolgu, ' '=saydam.
#define CUR_W 12
#define CUR_H 15

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

static void draw_btns(uint8_t b) {
    gfx_fill_rect(200, 2, 14, 14, (b & MOUSE_LEFT)   ? GFX_RGB(0, 220, 0) : GFX_RGB(90, 90, 90));
    gfx_fill_rect(218, 2, 14, 14, (b & MOUSE_MIDDLE) ? GFX_RGB(0, 220, 0) : GFX_RGB(90, 90, 90));
    gfx_fill_rect(236, 2, 14, 14, (b & MOUSE_RIGHT)  ? GFX_RGB(0, 220, 0) : GFX_RGB(90, 90, 90));
}

// Fareyi oynat -> imleç takip etsin. Titremesiz: statik sahne bir kez cizilir,
// her karede yalniz imlecin eski/yeni kucuk dikdortgeni guncellenir (save-under
// + kismi present). Bir tusa basinca cikar.
static void cmd_mouse(void) {
    uint32_t w = gfx_width(), h = gfx_height();
    static uint32_t under[CUR_W * CUR_H];

    gfx_clear(GFX_RGB(25, 25, 40));
    gfx_fill_rect(0, 0, w, 18, GFX_RGB(50, 50, 80));
    gfx_text("fare demosu - cikmak icin bir tusa bas", 6, 1,
             GFX_RGB(200, 200, 220), GFX_RGB(50, 50, 80));
    draw_btns(0);
    gfx_present();

    int ox = -1, oy = -1;
    uint8_t ob = 0;

    while (!keyboard_haskey()) {
        int mx = mouse_get_x(), my = mouse_get_y();
        uint8_t b = mouse_get_buttons();

        int nx = mx > (int)(w - CUR_W) ? (int)(w - CUR_W) : mx;
        int ny = my > (int)(h - CUR_H) ? (int)(h - CUR_H) : my;

        int moved = (nx != ox || ny != oy);
        int btn_changed = (b != ob);

        if (btn_changed) {
            draw_btns(b);
            gfx_present_rect(200, 2, 52, 14);
        }
        if (moved || btn_changed) {
            if (moved && ox >= 0) {
                gfx_blit(under, (uint32_t)ox, (uint32_t)oy, CUR_W, CUR_H);
                gfx_present_rect((uint32_t)ox, (uint32_t)oy, CUR_W, CUR_H);
            }
            if (moved || ox < 0)
                gfx_copy_out((uint32_t)nx, (uint32_t)ny, CUR_W, CUR_H, under);
            draw_cursor(nx, ny, b);
            gfx_present_rect((uint32_t)nx, (uint32_t)ny, CUR_W, CUR_H);
            ox = nx; oy = ny;
        }
        ob = b;
        sleep(10);
    }
    (void)keyboard_getchar();
    clear_screen();
    print("mouse demo bitti\n");
}

// --- arka plan is-parcasi demosu --------------------------------------------
// Preemptive zamanlayiciyi kanitlar: bu isciler zaman-dilimiyle donusumlu
// calisir, seri porta yazar, sonra doner (task_exit -> stack geri alinir).
static void demo_worker(void* arg) {
    uint64_t id = (uint64_t)arg;
    char buf[21];
    for (int i = 0; i < 5; i++) {
        serial_write("[task ");
        serial_write(u64_to_dec(id, buf));
        serial_write("] tick ");
        serial_write(u64_to_dec((uint64_t)i, buf));
        serial_write("\n");
        sleep(150);
    }
    // entry doner -> task_trampoline task_exit cagirir; zombie sonra reap edilir.
}

static void cmd_threads(void) {
    char buf[21];
    print("3 arka plan task baslatiliyor (cikti seri portta)\n");
    task_create(demo_worker, (void*)1);
    task_create(demo_worker, (void*)2);
    task_create(demo_worker, (void*)3);
    print("canli task: "); print(u64_to_dec(sched_task_count(), buf)); putchar('\n');

    // Isciler bitip reap edilene kadar bekle (preemption zaten calistiriyor).
    for (int i = 0; i < 40 && sched_task_count() > 1; i++)
        sleep(100);

    print("bitince canli task: "); print(u64_to_dec(sched_task_count(), buf)); putchar('\n');
}

// --- ortak olay kuyrugu demosu (GUI olay dongusu prototipi) -----------------
static void cmd_events(void) {
    print("olay dongusu - cikmak icin ESC\n");
    event_t e;
    char buf[21];
    for (;;) {
        if (!event_poll(&e)) { asm volatile("hlt"); continue; }

        if (e.type == EV_KEY_DOWN) {
            if (e.scancode == 0x01) break;             // ESC (make)
            print("tus  : '");
            if (e.ch) putchar(e.ch); else print("?");
            print("'  sc="); print(u64_to_dec(e.scancode, buf)); putchar('\n');
        } else if (e.type == EV_MOUSE_DOWN) {
            print("fare down btn="); print(u64_to_dec(e.button, buf));
            print(" @"); print(u64_to_dec((uint64_t)(uint16_t)e.x, buf));
            print(","); print(u64_to_dec((uint64_t)(uint16_t)e.y, buf)); putchar('\n');
        } else if (e.type == EV_MOUSE_UP) {
            print("fare up   btn="); print(u64_to_dec(e.button, buf)); putchar('\n');
        }
        // EV_KEY_UP / EV_MOUSE_MOVE: cok gurultulu, atla.
    }
    // Kabuk tamponuna dusen tuslari temizle ki sonraki komut satirina sizmasin.
    while (keyboard_haskey()) (void)keyboard_getchar();
    print("olay dongusu bitti\n");
}

// --- komut ayristirici ------------------------------------------------------
static void run_command(const char* cmd) {
    const char* arg;
    if (cmd[0] == '\0')                              return;
    else if (streq(cmd, "help"))  { print(msg(MSG_HELP_LIST)); putchar('\n'); }
    else if (streq(cmd, "clear"))                   clear_screen();
    else if (streq(cmd, "about")) { print(msg(MSG_ABOUT)); putchar('\n'); }
    else if (streq(cmd, "mem"))                     cmd_mem();
    else if (streq(cmd, "mouse"))                   cmd_mouse();
    else if (streq(cmd, "threads"))                 cmd_threads();
    else if (streq(cmd, "events"))                  cmd_events();
    else if ((arg = str_starts_with(cmd, "lang ")))  cmd_lang(arg);
    else if ((arg = str_starts_with(cmd, "theme "))) cmd_theme(arg);
    else if (streq(cmd, "close"))                   cmd_close();
    else { print(msg(MSG_UNKNOWN_CMD)); print(cmd); putchar('\n'); }
}

static void hcf(void) {
    for (;;) __asm__ volatile ("hlt");
}

void kmain(void) {
    // Dusuk seviye: kesmeler, zamanlayici, girdi
    idt_init();
    pic_remap();
    pit_init(100);
    idt_set_gate(0x20, (void*)irq0_timer, IDT_INT_GATE);
    irq_clear_mask(0);
    keyboard_init();
    mouse_init(gfx_width(), gfx_height());

    // Bellek + arka tampon (konsol bundan sonra cift tamponlu calisir)
    pmm_init();
    vmm_init();
    heap_init();
    gfx_backbuffer_init();

    event_init();
    sched_init();
    serial_init();
    serial_write("ArcMonark boot\n");
    asm volatile("sti");

    // Konsol
    settings_init();
    set_color(settings.fg, settings.bg);
    clear_screen();
    print(msg(MSG_WELCOME));   putchar('\n');
    print(msg(MSG_HELP_HINT)); print("\n\n");

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
    if (!gfx_init())
        hcf();
    kmain();
    hcf();
}
