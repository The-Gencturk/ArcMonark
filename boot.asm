bits 16
org 0x7c00

KERNEL_OFFSET   equ 0x1000      ; kernel'i buraya yükleyeceğiz
KERNEL_SECTORS  equ 15          ; kaç sektör okunacak (kernel boyutuna göre ayarla)

start:
    ; --- 1) Segment'ler + stack güvenli hale ---
    cli                     ; kurulum bitene kadar interrupt kapalı
    xor ax, ax
    mov ds, ax              ; DS = 0
    mov es, ax              ; ES = 0
    mov ss, ax              ; SS = 0
    mov sp, 0x7c00          ; stack 0x7c00'ün altına doğru büyür
    jmp 0x0000:.init        ; far jump → CS'i de 0'a normalize et

.init:
    sti
    mov [boot_drive], dl    ; BIOS boot drive no'sunu DL'de verir, saklıyoruz
    mov byte [retry], 4     ; disk okuma için deneme hakkı

.read:
    mov ah, 0x02            ; fonksiyon: sektör oku
    mov al, KERNEL_SECTORS  ; kaç sektör
    mov ch, 0               ; cylinder 0
    mov cl, 2               ; sektör 2'den başla (1 = boot sector'ün kendisi)
    mov dh, 0               ; head 0
    mov dl, [boot_drive]
    xor bx, bx
    mov es, bx              ; ES = 0
    mov bx, KERNEL_OFFSET   ; ES:BX = 0x0000:0x1000 hedef
    int 0x13
    jnc .ok                 ; carry yoksa okuma başarılı

    ; hata → diski resetle, tekrar dene
    dec byte [retry]
    jz .halt
    xor ah, ah
    mov dl, [boot_drive]
    int 0x13                ; disk reset
    jmp .read

.ok:
    mov dl, [boot_drive]    ; kernel'e drive no'sunu bırak (işine yarar)
    jmp 0x0000:KERNEL_OFFSET ; kontrolü kernel'e devret

.halt:
    cli
    hlt
    jmp .halt

boot_drive db 0
retry      db 0

times 510-($-$$) db 0
dw 0xaa55