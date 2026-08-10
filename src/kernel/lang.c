#include "lang.h"
#include "settings.h"

static const char* table[LANG_COUNT][MSG_COUNT] = {
    {
        [MSG_WELCOME] = "ArcMonark 64-bit long mode",
        [MSG_HELP_HINT] = "'help' yaz.",
        [MSG_HELP_LIST] = "komutlar: help, clear, about, mem, mouse, lang, theme, close",
        [MSG_ABOUT] = "ArcMonark v0.1 - 64-bit hobi cekirdegi",
        [MSG_UNKNOWN_CMD] = "bilinmeyen komut: ",
        [MSG_LANG_SET] = "dil degistirildi",
        [MSG_THEME_SET] = "tema degistirildi",
    },

    {
        [MSG_WELCOME] = "ArcMonark 64-bit long mode",
        [MSG_HELP_HINT] = "type 'help'.",
        [MSG_HELP_LIST] = "commands: help, clear, about, mem, mouse, lang, theme, close",
        [MSG_ABOUT] = "ArcMonark v0.1 - 64-bit hobby kernel",
        [MSG_UNKNOWN_CMD] = "unknown command: ",
        [MSG_LANG_SET] = "language changed",
        [MSG_THEME_SET] = "theme changed",
    },
};

const char* msg(enum msg_id id) {
    enum lang l = settings.language;
    const char* s = table[l][id];
    return s ? s : table[LANG_TR][id];   
}