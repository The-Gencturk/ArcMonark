#pragma once
#include <stdint.h>
#include "idt.h"

__attribute__((noreturn))
void panic(const char* msg, struct interrupt_frame* frame, uint64_t error_code);