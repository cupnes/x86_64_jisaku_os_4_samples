#include <lib.h>

#define WAIT_CLK	1000000

int main(void)
{
	while (1) {
		unsigned long long wait = WAIT_CLK;
		putc('B');
		while (wait--);
	}

	return 0;
}
