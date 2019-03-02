#include <lib.h>

#define WAIT_CLK	1000000

int main(void)
{
	enq_task(open("putb"));

	while (1) {
		unsigned long long wait = WAIT_CLK;
		putc('A');
		while (wait--);
	}

	return 0;
}
