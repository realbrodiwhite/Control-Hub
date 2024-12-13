ENTRY(_start)

MEMORY
{
    RAM : ORIGIN = 0x8000, LENGTH = 1M
}

SECTIONS
{
    .text : {
        KEEP(*(.vector_table))
        *(.text*)
        *(.rodata*)
    } > RAM

    .data : {
        *(.data*)
    } > RAM

    .bss : {
        *(.bss*)
        *(COMMON)
    } > RAM

    /DISCARD/ : {
        *(.comment)
        *(.gnu*)
        *(.note*)
        *(.eh_frame*)
    }
}
