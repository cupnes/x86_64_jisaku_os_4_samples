#include <x86.h>
#include <pci.h>
#include <nic.h>
#include <fbcon.h>
#include <common.h>

#define RXDESC_NUM	80
#define TXDESC_NUM	80
#define ALIGN_MARGIN	16

struct __attribute__((packed)) rxdesc {
	unsigned long long buffer_address;
	unsigned short length;
	unsigned short packet_checksum;
	unsigned char status;
	unsigned char errors;
	unsigned short special;
};

struct __attribute__((packed)) txdesc {
	unsigned long long buffer_address;
	unsigned short length;
	unsigned char cso;
	unsigned char cmd;
	unsigned char sta:4;
	unsigned char _rsv:4;
	unsigned char css;
	unsigned short special;
};

static unsigned int nic_reg_base;

static unsigned char rx_buffer[RXDESC_NUM][PACKET_BUFFER_SIZE];
static unsigned char rxdesc_data[
	(sizeof(struct rxdesc) * RXDESC_NUM) + ALIGN_MARGIN];
static struct rxdesc *rxdesc_base;
static unsigned short current_rx_idx;

static unsigned char tx_buffer[TXDESC_NUM][PACKET_BUFFER_SIZE];
static unsigned char txdesc_data[
	(sizeof(struct txdesc) * TXDESC_NUM) + ALIGN_MARGIN];
static struct txdesc *txdesc_base;
static unsigned short current_tx_idx;

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

	/* rxdescの先頭アドレスを16バイトの倍数となるようにする */
	unsigned long long rxdesc_addr = (unsigned long long)rxdesc_data;
	rxdesc_addr = (rxdesc_addr + ALIGN_MARGIN) & 0xfffffffffffffff0;

	/* rxdescの初期化 */
	rxdesc_base = (struct rxdesc *)rxdesc_addr;
	struct rxdesc *cur_rxdesc = rxdesc_base;
	for (i = 0; i < RXDESC_NUM; i++) {
		cur_rxdesc->buffer_address = (unsigned long long)rx_buffer[i];
		cur_rxdesc->status = 0;
		cur_rxdesc->errors = 0;
		cur_rxdesc++;
	}

	/* rxdescの先頭アドレスとサイズをNICレジスタへ設定 */
	set_nic_reg(NIC_REG_RDBAH, rxdesc_addr >> 32);
	set_nic_reg(NIC_REG_RDBAL, rxdesc_addr & 0x00000000ffffffff);
	set_nic_reg(NIC_REG_RDLEN, sizeof(struct rxdesc) * RXDESC_NUM);

	/* 先頭と末尾のインデックスをNICレジスタへ設定 */
	current_rx_idx = 0;
	set_nic_reg(NIC_REG_RDH, current_rx_idx);
	set_nic_reg(NIC_REG_RDT, RXDESC_NUM - 1);

	/* NICの受信動作設定 */
	set_nic_reg(NIC_REG_RCTL, PACKET_BUFFER_BIT | NIC_RCTL_BAM
		    | NIC_RCTL_MPE | NIC_RCTL_UPE | NIC_RCTL_SBP | NIC_RCTL_EN);
}

static void tx_init(void)
{
	unsigned int i;

	/* txdescの先頭アドレスを16バイトの倍数となるようにする */
	unsigned long long txdesc_addr = (unsigned long long)txdesc_data;
	txdesc_addr = (txdesc_addr + ALIGN_MARGIN) & 0xfffffffffffffff0;

	/* txdescの初期化 */
	txdesc_base = (struct txdesc *)txdesc_addr;
	struct txdesc *cur_txdesc = txdesc_base;
	for (i = 0; i < TXDESC_NUM; i++) {
		cur_txdesc->buffer_address = 0;
		cur_txdesc->length = 0;
		cur_txdesc->cso = 0;
		cur_txdesc->cmd = NIC_TDESC_CMD_RS;
		cur_txdesc->sta = 0;
		cur_txdesc->_rsv = 0;
		cur_txdesc->css = 0;
		cur_txdesc->special = 0;
		cur_txdesc++;
	}

	/* txdescの先頭アドレスとサイズをNICレジスタへ設定 */
	set_nic_reg(NIC_REG_TDBAH, txdesc_addr >> 32);
	set_nic_reg(NIC_REG_TDBAL, txdesc_addr & 0x00000000ffffffff);
	set_nic_reg(NIC_REG_TDLEN, sizeof(struct txdesc) * TXDESC_NUM);

	/* 先頭と末尾のインデックスをNICレジスタへ設定 */
	current_tx_idx = 0;
	set_nic_reg(NIC_REG_TDH, current_tx_idx);
	set_nic_reg(NIC_REG_TDT, current_tx_idx);

	/* NICの送信動作設定 */
	set_nic_reg(NIC_REG_TCTL, PACKET_BUFFER_BIT | NIC_RCTL_BAM
		    | NIC_RCTL_MPE | NIC_RCTL_UPE | NIC_RCTL_SBP | NIC_RCTL_EN);
}

void nic_init(void)
{
	/* NICのレジスタのベースアドレスを取得しておく */
	nic_reg_base = get_nic_reg_base();

	/* NICの割り込みを全て無効にする */
	disable_nic_interrupt();

	/* 受信の初期化処理 */
	rx_init();

	/* 送信の初期化処理 */
	tx_init();
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
	if (cur_rxdesc->status & NIC_RDESC_STAT_DD) {
		len = cur_rxdesc->length;
		memcpy(buf, (void *)cur_rxdesc->buffer_address,
		       cur_rxdesc->length);

		cur_rxdesc->status = 0;

		set_nic_reg(NIC_REG_RDT, current_rx_idx);

		current_rx_idx = (current_rx_idx + 1) % RXDESC_NUM;
	}

	return len;
}

unsigned short dump_packet(void)
{
	unsigned char buf[PACKET_BUFFER_SIZE];
	unsigned short len;
	len = receive_packet(buf);

	unsigned short i;
	for (i = 0; i < len; i++) {
		puth(buf[i], 2);
		putc(' ');

		if (((i + 1) % 24) == 0)
			puts("\r\n");
		else if (((i + 1) % 4) == 0)
			putc(' ');
	}
	if (len > 0)
		puts("\r\n");

	return len;
}
