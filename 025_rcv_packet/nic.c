#include <x86.h>
#include <pci.h>
#include <nic.h>
#include <fbcon.h>
#include <common.h>

#define RXDESC_NUM	80
#define ALIGN_MARGIN	16

struct __attribute__((packed)) rxdesc {
	unsigned long long buffer_address;
	unsigned short length;
	unsigned short packet_checksum;
	unsigned char status;
	unsigned char errors;
	unsigned short special;
};

static unsigned int nic_reg_base;
static unsigned char rx_buffer[RXDESC_NUM][PACKET_BUFFER_SIZE];
static unsigned char rxdesc_data[
	(sizeof(struct rxdesc) * RXDESC_NUM) + ALIGN_MARGIN];
static struct rxdesc *rxdesc_base;
static unsigned short current_rx_idx;

static void disable_nic_interrupt(void)
{
	/* 一旦、コマンドとステータスを読み出す */
	unsigned int conf_data = get_pci_conf_reg(
		NIC_BUS_NUM, NIC_DEV_NUM, NIC_FUNC_NUM,
		PCI_CONF_STATUS_COMMAND);

	/* ステータス(上位16ビット)をクリア */
	conf_data &= 0x0000ffff;
	/* コマンドに割り込み無効設定 */
	conf_data |= PCI_COM_INTR_DIS;

	/* コマンドとステータスに書き戻す */
	set_pci_conf_reg(NIC_BUS_NUM, NIC_DEV_NUM, NIC_FUNC_NUM,
			 PCI_CONF_STATUS_COMMAND, conf_data);

	/* NICの割り込みをIMC(Interrupt Mask Clear Register)で全て無効化 */
	set_nic_reg(NIC_REG_IMC, 0xffffffff);
}

static void rx_init(void)
{
	unsigned int i;

	unsigned long long rxdesc_addr = (unsigned long long)rxdesc_data;
	/* puts("RXDESC ADDR "); */
	/* puth(rxdesc_addr, 16); */
	/* puts("\r\n"); */
	rxdesc_addr = (rxdesc_addr + ALIGN_MARGIN) & 0xfffffffffffffff0;
	/* puts("RXDESC ADDR "); */
	/* puth(rxdesc_addr, 16); */
	/* puts("\r\n"); */
	/* return; */

	/* rxdescの初期化 */
	rxdesc_base = (struct rxdesc *)rxdesc_addr;
	puts("RXDESC_BASE ");
	puth((unsigned long long)rxdesc_base, 16);
	puts("\r\n");
	struct rxdesc *cur_rxdesc = rxdesc_base;
	for (i = 0; i < RXDESC_NUM; i++) {
		/* puts("RXDESC "); */
		/* puth((unsigned long long)cur_rxdesc, 16); */
		/* puts("\r\n"); */
		cur_rxdesc->buffer_address = (unsigned long long)rx_buffer[i];
		/* puts("BUFFER "); */
		/* puth(cur_rxdesc->buffer_address, 16); */
		/* puts("\r\n"); */
		cur_rxdesc->status = 0;
		cur_rxdesc->errors = 0;
		cur_rxdesc++;
	}

	set_nic_reg(NIC_REG_RDBAH, rxdesc_addr >> 32);
	set_nic_reg(NIC_REG_RDBAL, rxdesc_addr & 0x00000000ffffffff);

	/* puts("RDBAH "); */
	/* puth(get_nic_reg(NIC_REG_RDBAH), 8); */
	/* puts("\r\n"); */
	/* puts("RDBAL "); */
	/* puth(get_nic_reg(NIC_REG_RDBAL), 8); */
	/* puts("\r\n"); */

	set_nic_reg(NIC_REG_RDLEN, sizeof(struct rxdesc) * RXDESC_NUM);

	/* puts("RDLEN "); */
	/* puth(get_nic_reg(NIC_REG_RDLEN), 8); */
	/* puts("\r\n"); */

	current_rx_idx = 0;
	set_nic_reg(NIC_REG_RDH, current_rx_idx);
	set_nic_reg(NIC_REG_RDT, RXDESC_NUM - 1);

	/* puts("RDH "); */
	/* puth(get_nic_reg(NIC_REG_RDH), 8); */
	/* puts("\r\n"); */
	/* puts("RDT "); */
	/* puth(get_nic_reg(NIC_REG_RDT), 8); */
	/* puts("\r\n"); */

	set_nic_reg(NIC_REG_RCTL, PACKET_BUFFER_BIT | NIC_RCTL_BAM
		    | NIC_RCTL_MPE | NIC_RCTL_UPE | NIC_RCTL_SBP | NIC_RCTL_EN);
	/* puts("RCTL "); */
	/* puth(get_nic_reg(NIC_REG_RCTL), 8); */
	/* puts("\r\n"); */

	/* DEBUG */
	rx_buffer[0][0] = 0x01;
	rx_buffer[1][0] = 0x02;
	rx_buffer[2][0] = 0x03;
}

void nic_init(void)
{
	/* NICのレジスタのベースアドレスを取得しておく */
	nic_reg_base = get_nic_reg_base();

	/* NICの割り込みを全て無効にする */
	disable_nic_interrupt();

	/* 受信の初期化処理 */
	rx_init();
}

unsigned int get_nic_reg_base(void)
{
	/* PCIコンフィグレーション空間からBARを取得 */
	unsigned int bar = get_pci_conf_reg(
		NIC_BUS_NUM, NIC_DEV_NUM, NIC_FUNC_NUM, PCI_CONF_BAR);

	/* メモリ空間用ベースアドレス(32ビット)を返す */
	return bar & PCI_BAR_MASK_MEM_ADDR;
}

unsigned int get_nic_reg(unsigned short reg)
{
	unsigned long long addr = nic_reg_base + reg;
	return *(volatile unsigned int *)addr;
}

void set_nic_reg(unsigned short reg, unsigned int val)
{
	unsigned long long addr = nic_reg_base + reg;
	*(volatile unsigned int *)addr = val;
}

void dump_nic_ims(void)
{
	unsigned int ims = get_nic_reg(NIC_REG_IMS);

	puts("IMS ");
	puth(ims, 8);
	puts("\r\n");
}

unsigned short receive_packet(void *buf)
{
	unsigned short len = 0;

	struct rxdesc *cur_rxdesc = rxdesc_base + current_rx_idx;
	puts("CUR_RXDESC ");
	puth((unsigned long long)cur_rxdesc, 16);
	puts("\r\n");
	if (1 || cur_rxdesc->status & NIC_RDESC_STAT_DD) {
		puts("CUR BUFFER_ADDRESS ");
		puth(cur_rxdesc->buffer_address, 16);
		puts("\r\n");

		len = cur_rxdesc->length;
		/* memcpy(buf, (void *)cur_rxdesc->buffer_address, */
		/*        cur_rxdesc->length); */
		memcpy(buf, (void *)cur_rxdesc->buffer_address, 256);

		cur_rxdesc->status = 0;

		set_nic_reg(NIC_REG_RDT, current_rx_idx);

		current_rx_idx = (current_rx_idx + 1) % RXDESC_NUM;
	}

	return len;
}

void dump_packet(unsigned int rx_idx)
{
	current_rx_idx = rx_idx;

	unsigned char buf[PACKET_BUFFER_SIZE];
	unsigned short len;
	len = receive_packet(buf);
	len = 256;

	unsigned short i;
	for (i = 0; i < len; i++) {
		puth(buf[i], 2);
		putc(' ');

		if (((i + 1) % 24) == 0)
			puts("\r\n");
		else if (((i + 1) % 4) == 0)
			putc(' ');
	}
	puts("\r\n");
}
