SECTIONS {
  .text : { *(.text); *(.text.*) }
  .bss : { *(.bss); *(.bss.*) }
  .data : { *(.data); *(.data.*) }
  .rodata : { *(.rodata); *(.rodata.*) }
  .rela : { *(.rela); *(.rela.*) }
}