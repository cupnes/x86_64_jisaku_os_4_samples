#include <x86.h>
#include <pci.h>
#include <nic.h>
#include <fbcon.h>

static unsigned int nic_reg_base;

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

void nic_init(void)
{
	/* NICのレジスタのベースアドレスを取得しておく */
	nic_reg_base = get_nic_reg_base();

	/* NICの割り込みを全て無効にする */
	disable_nic_interrupt();
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
