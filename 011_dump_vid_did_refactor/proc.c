#include <fs.h>

void exec(struct file *f)
{
	void *start_addr = (void *)f->data;
	asm volatile ("call *%[start_addr]" :: [start_addr]"r"(start_addr));
}
