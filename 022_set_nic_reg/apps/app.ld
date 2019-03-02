OUTPUT_FORMAT("binary");

SECTIONS
{
	.body : {
		*(.text)

		*(.rodata)

		*(.data)

		__bss_start = .;
		*(.bss)
		__bss_end = .;

		*(.eh_frame)
	}
}
