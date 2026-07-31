#ifndef LANG_H
#define LANG_H

enum lang { LANG_TR = 0, LANG_EN = 1, LANG_COUNT };

enum msg_id {
    MSG_WELCOME = 0,
    MSG_HELP_HINT,
    MSG_HELP_LIST,
    MSG_ABOUT,
    MSG_UNKNOWN_CMD,
    MSG_LANG_SET,
    MSG_THEME_SET,
    MSG_COUNT
};

const char* msg(enum msg_id id);

#endif