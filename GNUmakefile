KERNEL := build/kernel.elf
ISO    := arcmonark.iso
LIMINE := limine
BUILD  := build

CC := clang
LD := ld.lld
AS := nasm

CFLAGS := -g -O2 -pipe -Wall -Wextra -std=gnu11 \
          -ffreestanding -fno-builtin -fno-stack-protector -fno-stack-check \
          -fno-lto -fno-PIC -m64 -march=x86-64 \
          -mno-80387 -mno-mmx -mno-sse -mno-sse2 \
          -mno-red-zone -mgeneral-regs-only -mcmodel=kernel \
          -Iinclude -MMD -MP

ASFLAGS := -f elf64

LDFLAGS := -m elf_x86_64 -nostdlib -static -z max-page-size=0x1000 -T linker.ld

# Kaynaklar alt sistem klasorlerinden otomatik toplanir (src/**/*.c, src/**/*.asm).
CFILES  := $(shell find src -name '*.c')
ASFILES := $(shell find src -name '*.asm')

# Objeler build/ altinda src agacini aynalar: build/src/mm/heap.o gibi.
OBJ  := $(CFILES:%.c=$(BUILD)/%.o) $(ASFILES:%.asm=$(BUILD)/%.o)
DEPS := $(CFILES:%.c=$(BUILD)/%.d)

all: $(KERNEL)

$(KERNEL): $(OBJ) linker.ld
	$(LD) $(OBJ) $(LDFLAGS) -o $@

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: %.asm
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

# Otomatik uretilen .h bagimlilik dosyalari (bir baslik degisince ilgili .o yeniden derlenir).
-include $(DEPS)

iso: $(ISO)

$(ISO): $(KERNEL) limine.conf
	rm -rf iso_root
	mkdir -p iso_root/boot/limine iso_root/EFI/BOOT
	cp $(KERNEL) iso_root/boot/kernel.elf
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
	qemu-system-x86_64 -cdrom $(ISO) -m 256M -serial stdio

clean:
	rm -rf $(BUILD) iso_root $(ISO)

.PHONY: all clean iso run
