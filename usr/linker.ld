
ENTRY(_start);

SECTIONS
{
    . = 0x10000;

    .text : { *(.text.prologue); *(.text*); }
    .data : { *(.data*); }
    .rodata : { *(.rodata*); }
    .bss : {
        __bss_start = .;
        *(COMMON);
        *(.bss);
        . = ALIGN(4);
        __bss_end = .;
    }
    __bss_size = ((__bss_end)-(__bss_start));
    /DISCARD/ : {  *(.note); }
}



