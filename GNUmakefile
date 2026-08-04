KERNEL := kernel.elf
ISO    := arcmonark.iso
LIMINE := limine

CC := clang
LD := ld.lld

CFLAGS := -g -O2 -pipe -Wall -Wextra -std=gnu11 \
          -ffreestanding -fno-stack-protector -fno-stack-check \
          -fno-lto -fno-PIC -m64 -march=x86-64 \
          -mno-80387 -mno-mmx -mno-sse -mno-sse2 \
          -mno-red-zone -mgeneral-regs-only -mcmodel=kernel -I.

LDFLAGS := -m elf_x86_64 -nostdlib -static -z max-page-size=0x1000 -T linker.ld

CFILES := kernel.c fb.c keyboard.c lang.c settings.c idt.c pic.c timer.c exceptions.c panic.c pmm.c
OBJ := $(CFILES:.c=.o)

all: $(KERNEL)

$(KERNEL): $(OBJ) linker.ld
	$(LD) $(OBJ) $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

iso: $(ISO)

$(ISO): $(KERNEL) limine.conf
	rm -rf iso_root
	mkdir -p iso_root/boot/limine iso_root/EFI/BOOT
	cp $(KERNEL) iso_root/boot/
	cp limine.conf iso_root/boot/limine/
	cp $(LIMINE)/limine-bios.sys $(LIMINE)/limine-bios-cd.bin $(LIMINE)/limine-uefi-cd.bin iso_root/boot/limine/
	cp $(LIMINE)/BOOTX64.EFI iso_root/EFI/BOOT/
	xorriso -as mkisofs -b boot/limine/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot boot/limine/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso_root -o $(ISO)
	$(LIMINE)/limine bios-install $(ISO)

run: $(ISO)
	qemu-system-x86_64 -cdrom $(ISO) -m 256M

clean:
	rm -f $(OBJ) $(KERNEL) $(ISO)
	rm -rf iso_root

.PHONY: all clean iso run