#include <pci.h>
#include <nic.h>

unsigned int get_nic_reg_base(void)
{
	/* PCIコンフィグレーション空間からBARを取得 */
	unsigned int bar = get_pci_conf_reg(
		NIC_BUS_NUM, NIC_DEV_NUM, NIC_FUNC_NUM, PCI_CONF_BAR);

	/* メモリ空間用ベースアドレス(32ビット)を返す */
	return bar & PCI_BAR_MASK_MEM_ADDR;
}
