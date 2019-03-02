int main(void)
{
	unsigned long long softirq_ret;
	asm volatile ("movq $1, %%rdi\n"
		      "movq $2, %%rsi\n"
		      "movq $3, %%rdx\n"
		      "movq $4, %%rcx\n"
		      "int $0x80\n"
		      "movq %%rax, %[softirq_ret]"
		      : [softirq_ret]"=r"(softirq_ret):);

	while (1);

	return 0;
}
