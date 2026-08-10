#include <stdint.h>

int  serial_init(void);        
void serial_putc(char c);
void serial_write(const char* s);
void serial_write_hex(uint64_t v); 