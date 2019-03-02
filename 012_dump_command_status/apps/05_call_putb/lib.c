enum SYSCCALL_NO {
	SYSCALL_PUTC,
	SYSCALL_OPEN,
	SYSCALL_EXEC,
	SYSCALL_ENQ_TASK,
	MAX_SYSCALL_NUM
};

unsigned long long syscall(
	unsigned long long syscall_id __attribute__((unused)),
	unsigned long long arg1 __attribute__((unused)),
	unsigned long long arg2 __attribute__((unused)),
	unsigned long long arg3 __attribute__((unused)))
{
	unsigned long long ret_val;

	asm volatile ("int $0x80\n"
		      "movq %%rax, %[ret_val]"
		      : [ret_val]"=r"(ret_val) :);

	return ret_val;
}

void putc(char c)
{
	syscall(SYSCALL_PUTC, c, 0, 0);
}

struct file *open(char *file_name)
{
	return (struct file *)syscall(
		SYSCALL_OPEN, (unsigned long long)file_name, 0, 0);
}

void exec(struct file *file)
{
	syscall(SYSCALL_EXEC, (unsigned long long)file, 0, 0);
}

void enq_task(struct file *file)
{
	syscall(SYSCALL_ENQ_TASK, (unsigned long long)file, 0, 0);
}
