	.global default_handler
default_handler:
	jmp	default_handler

	.global	hpet_handler
hpet_handler:
	push	%rax
	push	%rcx
	push	%rdx
	push	%rbx
	push	%rbp
	push	%rsi
	push	%rdi
	mov	%rsp, %rdi
	call	do_hpet_interrupt
	pop	%rdi
	pop	%rsi
	pop	%rbp
	pop	%rbx
	pop	%rdx
	pop	%rcx
	pop	%rax
	iretq

	.global	kbc_handler
kbc_handler:
	push	%rax
	push	%rcx
	push	%rdx
	push	%rbx
	push	%rbp
	push	%rsi
	push	%rdi
	call	do_kbc_interrupt
	pop	%rdi
	pop	%rsi
	pop	%rbp
	pop	%rbx
	pop	%rdx
	pop	%rcx
	pop	%rax
	iretq

	.global	syscall_handler
syscall_handler:
	push	%rcx
	push	%rax
	push	%rdx
	push	%rbx
	push	%rbp
	push	%rsi
	push	%rdi
	call	do_syscall_interrupt
	pop	%rdi
	pop	%rsi
	pop	%rbp
	pop	%rbx
	pop	%rdx
	pop	%rcx  /* RAXは戻り値の受け渡しに使うため退避したものは捨てる */
	pop	%rcx
	iretq
