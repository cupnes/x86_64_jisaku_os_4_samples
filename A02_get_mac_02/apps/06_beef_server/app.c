#include <lib.h>

#define PACKET_BUFFER_SIZE	1024

void dump_frame(unsigned char buf[], unsigned short len);

int main(void)
{
	while (1) {
		unsigned char buf[PACKET_BUFFER_SIZE];
		unsigned char buf_s[PACKET_BUFFER_SIZE] = { 0 };
		unsigned short len;

		/* 受信 */
		while (!(len = receive_frame(buf)));
		dump_frame(buf, len);
		puts("__RECEIVED\r\n");

		/* 送信 */
		buf_s[0] = 0xbe;
		buf_s[1] = 0xef;
		buf_s[2] = 0xbe;
		buf_s[3] = 0xef;
		len = 4;
		dump_frame(buf_s, len);
		unsigned char stat = send_frame(buf_s, len);
		puts("__SENDED ");
		puth(stat, 2);
		puts("\r\n");
	}

	return 0;
}

void dump_frame(unsigned char buf[], unsigned short len)
{
	unsigned short i;
	for (i = 0; i < len; i++) {
		puth(buf[i], 2);
		putc(' ');

		if (((i + 1) % 24) == 0)
			puts("\r\n");
		else if (((i + 1) % 4) == 0)
			putc(' ');
	}

	putc('_');
	puth(len, 4);

	if (len > 0)
		puts("\r\n");
}
