#include <hpet.h>
#include <pic.h>
#include <fbcon.h>
#include <fs.h>

#define SCHED_PERIOD	(5 * MS_TO_US)
#define MAX_TASKS		100
#define TASK_STASK_BYTES	4096

unsigned long long task_sp[MAX_TASKS];
volatile unsigned int current_task = 0;
unsigned char task_stack[MAX_TASKS - 1][TASK_STASK_BYTES];
unsigned int num_tasks = 1;

void schedule(unsigned long long current_rsp)
{
	task_sp[current_task] = current_rsp;
	current_task = (current_task + 1) % num_tasks;
	set_pic_eoi(HPET_INTR_NO);
	asm volatile ("mov %[sp], %%rsp"
		      :: [sp]"a"(task_sp[current_task]));
	asm volatile (
		"pop	%rdi\n"
		"pop	%rsi\n"
		"pop	%rbp\n"
		"pop	%rbx\n"
		"pop	%rdx\n"
		"pop	%rcx\n"
		"pop	%rax\n"
		"iretq\n");
}

void sched_init(void)
{
	/* 5ms周期の周期タイマー設定 */
	ptimer_setup(SCHED_PERIOD, schedule);
}

void enq_task(struct file *f)
{
	unsigned long long start_addr = (unsigned long long)f->data;

	/* 予めタスクのスタックを適切に積んでおき、スタックポインタを揃える */
	unsigned long long *sp =
		(unsigned long long *)((unsigned char *)task_stack[num_tasks]
				       + TASK_STASK_BYTES);
	unsigned long long old_sp = (unsigned long long)sp;

	/* push SS */
	--sp;
	*sp = 0x10;

	/* push old RSP */
	--sp;
	*sp = old_sp;

	/* push RFLAGS */
	--sp;
	*sp = 0x202;

	/* push CS */
	--sp;
	*sp = 8;

	/* push RIP */
	--sp;
	*sp = start_addr;

	/* push GR */
	unsigned char i;
	for (i = 0; i < 7; i++) {
		--sp;
		*sp = 0;
	}

	task_sp[num_tasks] = (unsigned long long)sp;

	num_tasks++;
}

void sched_start(void)
{
	/* 周期タイマースタート */
	ptimer_start();
}
