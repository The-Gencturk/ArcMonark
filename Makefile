ASM    = nasm
CC = i686-elf-gcc
LD = i686-elf-ld
QEMU   = qemu-system-i386

CFLAGS = -ffreestanding -fno-pie -fno-stack-protector \
         -nostdlib -fno-builtin -Wall -Wextra -Iinclude


LDFLAGS = -T linker.ld

BOOT_SRC   = boot.asm
ENTRY_SRC  = kernel_entry.asm
KERNEL_SRC = kernel.c

BOOT_BIN   = boot.bin
KERNEL_BIN = kernel.bin
OS_IMAGE   = arcmonark.img

all: $(OS_IMAGE)

$(BOOT_BIN): $(BOOT_SRC)
	$(ASM) -f bin $< -o $@

kernel_entry.o: $(ENTRY_SRC)
	$(ASM) -f elf32 $< -o $@

kernel.o: kernel.c
	$(CC) $(CFLAGS) -c $< -o $@

vga.o: vga.c
	$(CC) $(CFLAGS) -c $< -o $@

keyboard.o: keyboard.c
	$(CC) $(CFLAGS) -c $< -o $@

lang.o: lang.c
	$(CC) $(CFLAGS) -c $< -o $@

settings.o: settings.c
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL_BIN): kernel_entry.o kernel.o vga.o keyboard.o lang.o settings.o linker.ld
	$(LD) $(LDFLAGS) kernel_entry.o kernel.o vga.o keyboard.o lang.o settings.o -o $@

$(OS_IMAGE): $(BOOT_BIN) $(KERNEL_BIN)
	cat $(BOOT_BIN) $(KERNEL_BIN) > $@
	truncate -s 1440k $@

run: $(OS_IMAGE)
	$(QEMU) -drive format=raw,file=$(OS_IMAGE),if=floppy

clean:
	rm -f *.bin *.o $(OS_IMAGE)

.PHONY: all run clean