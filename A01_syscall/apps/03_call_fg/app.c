#include <lib.h>

int main(void)
{
	putc('Z');
	exec(open("puta"));
	putc('B');

	return 0;
}
