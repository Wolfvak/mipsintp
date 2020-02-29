OUTPUT_FORMAT("elf32-littlemips")
OUTPUT_ARCH(mips)
ENTRY(_start)

MEMORY
{
    MEM (RWX) : ORIGIN = 0, LENGTH = 384
}

SECTIONS
{
    .text : ALIGN(4) {
        __text_s = ABSOLUTE(.);
        *(.text.start);
        *(.text*);
        . = ALIGN(4);
        __text_e = ABSOLUTE(.);
    } >MEM

    .data : ALIGN(4) {
        *(.data*);
        *(.rodata*);
        . = ALIGN(4);
    } >MEM

    .bss : ALIGN(4) {
        __bss_start = .;
        *(.bss*);
        . = ALIGN(4);
        __bss_end = .;
    } >MEM

    __end__ = ABSOLUTE(.);
}
