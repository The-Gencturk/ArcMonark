#pragma once
#include <stdint.h>

#define MOUSE_LEFT   0x01
#define MOUSE_RIGHT  0x02
#define MOUSE_MIDDLE 0x04

// PS/2 fare surucusu. screen_w/h imleç konumunu ekran icinde tutmak (clamp) icin.
void    mouse_init(uint32_t screen_w, uint32_t screen_h);

int     mouse_get_x(void);
int     mouse_get_y(void);
uint8_t mouse_get_buttons(void);   // bit0=sol, bit1=sag, bit2=orta
