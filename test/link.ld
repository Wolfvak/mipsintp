OUTPUT_FORMAT("elf32-littlemips")
OUTPUT_ARCH(mips)
ENTRY(_start)

MEMORY
{
    MEM (RWX) : ORIGIN = 0, LENGTH = 4096
}

SECTIONS
{
    .text : ALIGN(16) {
        __text_s = ABSOLUTE(.);
        KEEP(*(.text.start));
        KEEP(*(.text*));
        . = ALIGN(16);
        __text_e = ABSOLUTE(.);
    } >MEM

    .data : ALIGN(16) {
        KEEP(*(.data*));
        *(.rodata*);
        . = ALIGN(16);
    } >MEM

    .bss : ALIGN(16) {
        __bss_start = .;
        KEEP(*(.bss*));
        KEEP(*(.sbss*));
        . = ALIGN(16);
        __bss_end = .;
    } >MEM

    __stack_top = ORIGIN(MEM) + LENGTH(MEM);

    __end__ = ABSOLUTE(.);
}
