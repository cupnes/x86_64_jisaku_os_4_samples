#include <intr.h>
#include <pic.h>
#include <fbcon.h>
#include <fs.h>
#include <proc.h>
#include <sched.h>

#define SYSCALL_INTR_NO	0x80

enum SYSCCALL_NO {
	SYSCALL_PUTC,
	SYSCALL_OPEN,
	SYSCALL_EXEC,
	SYSCALL_ENQ_TASK,
	MAX_SYSCALL_NUM
};

unsigned long long do_syscall_interrupt(
	unsigned long long syscall_id, unsigned long long arg1,
	unsigned long long arg2 __attribute__((unused)),
	unsigned long long arg3 __attribute__((unused)))
{
	unsigned long long ret_val = 0;

	switch (syscall_id) {
	case SYSCALL_PUTC:
		putc((char)arg1);
		break;

	case SYSCALL_OPEN:
		ret_val = (unsigned long long)open((char *)arg1);
		break;

	case SYSCALL_EXEC:
		exec((struct file *)arg1);
		break;

	case SYSCALL_ENQ_TASK:
		enq_task((struct file *)arg1);
		break;
	}

	/* PICへ割り込み処理終了を通知(EOI) */
	set_pic_eoi(SYSCALL_INTR_NO);

	return ret_val;
}

void syscall_handler(void);
void syscall_init(void)
{
	set_intr_desc(SYSCALL_INTR_NO, syscall_handler);
	enable_pic_intr(SYSCALL_INTR_NO);
}
