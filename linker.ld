OUTPUT_FORMAT("binary")     /* çıktı: ham binary, ELF header yok */
ENTRY(kernel_entry)         /* giriş noktası bilgisi (binary'de sembolik) */

SECTIONS
{
    /* boot.asm bizi 0x1000'e atlıyor → her şey buradan başlar */
    . = 0x1000;

    /* --- KOD --- */
    .text :
    {
        *(.entry)           /* kernel_entry EN BAŞTA olsun */
        *(.text)            /* geri kalan tüm kod */
        *(.text.*)
    }

    /* --- SALT-OKUNUR VERİ --- (string sabitleri vs.) */
    .rodata :
    {
        *(.rodata)
        *(.rodata.*)
    }

    /* --- YAZILABİLİR VERİ --- (init edilmiş global'ler) */
    .data :
    {
        *(.data)
        *(.data.*)
    }

    /* --- SIFIR VERİ --- (init edilmemiş global'ler: cursor_row vs.) */
    .bss :
    {
        *(.bss)
        *(.bss.*)
        *(COMMON)
    }

    /* --- ÇÖP TENEKESİ --- linker'ın ürettiği gereksiz section'lar */
    /DISCARD/ :
    {
        *(.comment)
        *(.note*)
        *(.eh_frame)
    }
}