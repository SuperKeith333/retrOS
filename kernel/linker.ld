ENTRY(kernel_main)

SECTIONS
{
	. = 0x200000;

    . = ALIGN(0x1000);
	.text :
	{
		*(.text)
		*(.text.*)
	}

    . = ALIGN(0x1000);
	.rodata :
	{
		*(.rodata)
		*(.rodata.*)
	}

    . = ALIGN(0x1000);
	.data :
	{
		*(.data)
		*(.data.*)
	}

    . = ALIGN(0x1000);
	.bss :
	{
		*(.bss)
		*(.bss.*)
		*(COMMON)
	}
}