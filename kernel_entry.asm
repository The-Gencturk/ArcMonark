bits 16
; boot.asm bizi buraya (0x1000) real mode'da bırakıyor
section .entry    

kernel_entry:
    ; --- 1) A20 line'ı aç (fast A20, port 0x92) ---
    in al, 0x92
    or al, 2
    out 0x92, al

    ; --- 2) GDT'yi yükle, interrupt'ları kapat ---
    cli
    lgdt [gdt_descriptor]

    ; --- 3) cr0'ın PE (protected enable) bitini set et ---
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; --- 4) far jump → CS'i code segment'e set eder + pipeline'ı temizler ---
    jmp CODE_SEG:init_pm

; =========================================================
bits 32                     ; buradan sonrası 32-bit
init_pm:
    ; artık real mode değil → tüm data segment'leri DATA_SEG yap
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov ebp, 0x90000        ; stack'i güvenli, yüksek bir yere kur
    mov esp, ebp

    extern kmain            ; C tarafındaki kmain fonksiyonu
    call kmain              ; işletim sistemine gir

.hang:
    cli
    hlt
    jmp .hang

; =========================================================
; GDT: 3 giriş → null, code, data (flat memory model, 0-4GB)
gdt_start:
    dq 0x0000000000000000   ; null descriptor (zorunlu)

gdt_code:                   ; base=0, limit=0xFFFFF, 4KB granularity, 32-bit
    dw 0xFFFF               ; limit (0-15)
    dw 0x0000               ; base  (0-15)
    db 0x00                 ; base  (16-23)
    db 10011010b            ; access: present, ring0, code, executable, readable
    db 11001111b            ; flags + limit(16-19): 4KB gran, 32-bit
    db 0x00                 ; base  (24-31)

gdt_data:                   ; code ile aynı, sadece "data" tipi
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b            ; access: present, ring0, data, writable
    db 11001111b
    db 0x00
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1   ; GDT boyutu - 1
    dd gdt_start                 ; GDT'nin adresi

CODE_SEG equ gdt_code - gdt_start   ; = 0x08
DATA_SEG equ gdt_data - gdt_start   ; = 0x10