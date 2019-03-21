#pragma once

#define FILE_NAME_LEN	28
struct __attribute__((packed)) file {
	char name[FILE_NAME_LEN];
	unsigned int size;
	unsigned char data[0];
};

void putc(char c);
void puts(char *s);
void puth(unsigned long long val, unsigned char num_digits);
struct file *open(char *file_name);
void exec(struct file *file);
void enq_task(struct file *file);
unsigned short receive_frame(void *buf);
unsigned char send_frame(void *buf, unsigned short len);
