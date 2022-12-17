OUTPUT_ARCH( "riscv" )
ENTRY( _entry )

SECTIONS
{
	. = 0x80000000,
	.text.boot : { *(.text.boot) }
	
	.text : {
		*(.text)
	}

	.rodata : {
    . = ALIGN(16);
		*(.rodata) 
	}
    
	.data : {
    . = ALIGN(16);
    *(.data)
  }

	.bss : {
    . = ALIGN(16);
    *(.bss*)
  }
}
