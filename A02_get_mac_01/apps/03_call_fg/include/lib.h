#pragma once

#define FILE_NAME_LEN	28
struct __attribute__((packed)) file {
	char name[FILE_NAME_LEN];
	unsigned int size;
	unsigned char data[0];
};

void putc(char c);
struct file *open(char *file_name);
void exec(struct file *file);
