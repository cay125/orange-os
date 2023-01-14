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

  .init_array : {
    PROVIDE (init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE (init_array_end = .);
  }

	.bss : {
    . = ALIGN(16);
    *(.bss*)
  }
	PROVIDE(kernel_end = .);
	. = ALIGN(4096);
	PROVIDE(memory_beg = .);
}
