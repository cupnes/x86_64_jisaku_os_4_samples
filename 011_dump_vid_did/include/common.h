#ifndef _COMMON_H_
#define _COMMON_H_

#define NULL	(void *)0

int strcmp(char *s1, char *s2);
int strncmp(char *s1, char *s2, unsigned long long n);
void memcpy(void *dst, void *src, unsigned long long size);

#endif
