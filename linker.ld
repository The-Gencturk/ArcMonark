OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)

PHDRS {
    text    PT_LOAD FLAGS((1 << 0) | (1 << 2));
    rodata  PT_LOAD FLAGS((1 << 2));
    data    PT_LOAD FLAGS((1 << 1) | (1 << 2));
    dynamic PT_DYNAMIC FLAGS((1 << 1) | (1 << 2));
}

SECTIONS {
    . = 0xffffffff80000000;

    .text : { *(.text .text.*) } :text

    . += CONSTANT(MAXPAGESIZE);
    .rodata : { *(.rodata .rodata.*) } :rodata
    .limine_requests : {
        KEEP(*(.limine_requests_start))
        KEEP(*(.limine_requests))
        KEEP(*(.limine_requests_end))
    } :rodata

    . += CONSTANT(MAXPAGESIZE);
    .data : { *(.data .data.*) } :data
    .dynamic : { *(.dynamic) } :data :dynamic
    .bss : { *(.bss .bss.*) *(COMMON) } :data

    /DISCARD/ : { *(.note .note.*) *(.eh_frame*) }
}