#include <x86.h>
#include <pci.h>
#include <fbcon.h>

#define CONFIG_ADDRESS	0x0cf8
#define CONFIG_DATA	0x0cfc

union pci_config_address {
	unsigned int raw;
	struct __attribute__((packed)) {
		unsigned int reg_addr:8;
		unsigned int func_num:3;
		unsigned int dev_num:5;
		unsigned int bus_num:8;
		unsigned int _reserved:7;
		unsigned int enable_bit:1;
	};
};

unsigned int get_pci_conf_reg(
	unsigned char bus, unsigned char dev, unsigned char func,
	unsigned char reg)
{
	/* CONFIG_ADDRESSを設定 */
	union pci_config_address conf_addr;
	conf_addr.raw = 0;
	conf_addr.bus_num = bus;
	conf_addr.dev_num = dev;
	conf_addr.func_num = func;
	conf_addr.reg_addr = reg;
	conf_addr.enable_bit = 1;
	io_write32(CONFIG_ADDRESS, conf_addr.raw);

	/* CONFIG_DATAを読み出す */
	return io_read32(CONFIG_DATA);
}

void set_pci_conf_reg(unsigned char bus, unsigned char dev, unsigned char func,
		      unsigned char reg, unsigned int val)
{
	/* CONFIG_ADDRESSを設定 */
	union pci_config_address conf_addr;
	conf_addr.raw = 0;
	conf_addr.bus_num = bus;
	conf_addr.dev_num = dev;
	conf_addr.func_num = func;
	conf_addr.reg_addr = reg;
	conf_addr.enable_bit = 1;
	io_write32(CONFIG_ADDRESS, conf_addr.raw);

	/* CONFIG_DATAへvalを書き込む */
	io_write32(CONFIG_DATA, val);
}

void dump_vid_did(unsigned char bus, unsigned char dev, unsigned char func)
{
	/* PCIコンフィグレーション空間のレジスタを読み出す */
	unsigned int conf_data = get_pci_conf_reg(
		bus, dev, func, PCI_CONF_DID_VID);

	/* 読み出したデータからベンダーID・デバイスIDを取得 */
	unsigned short vendor_id = conf_data & 0x0000ffff;
	unsigned short device_id = conf_data >> 16;

	/* 表示 */
	puts("VENDOR ID ");
	puth(vendor_id, 4);
	puts("\r\n");
	puts("DEVICE ID ");
	puth(device_id, 4);
	puts("\r\n");
}

void dump_command_status(
	unsigned char bus, unsigned char dev, unsigned char func)
{
	/* PCIコンフィグレーション空間のレジスタを読み出す */
	unsigned int conf_data = get_pci_conf_reg(
		bus, dev, func, PCI_CONF_STATUS_COMMAND);

	/* 読み出したデータからステータスとコマンド値を取得 */
	unsigned short command = conf_data & 0x0000ffff;
	unsigned short status = conf_data >> 16;

	/* 表示 */
	puts("COMMAND ");
	puth(command, 4);
	puts("\r\n");

	if (command & PCI_COM_IO_EN)
		puts("IO_EN ");
	if (command & PCI_COM_MEM_EN)
		puts("MEM_EN ");
	if (command & PCI_COM_BUS_MASTER_EN)
		puts("BUS_MASTER_EN ");
	if (command & PCI_COM_SPECIAL_CYCLE)
		puts("SPECIAL_CYCLE ");
	if (command & PCI_COM_MEMW_INV_EN)
		puts("MEMW_INV_EN ");
	if (command & PCI_COM_VGA_PAL_SNP)
		puts("VGA_PAL_SNP ");
	if (command & PCI_COM_PARITY_ERR_RES)
		puts("PARITY_ERR_RES ");
	if (command & PCI_COM_SERR_EN)
		puts("SERR_EN ");
	if (command & PCI_COM_FAST_BACK2BACK_EN)
		puts("FAST_BACK2BACK_EN ");
	if (command & PCI_COM_INTR_DIS)
		puts("INTR_DIS");
	puts("\r\n");

	puts("STATUS ");
	puth(status, 4);
	puts("\r\n");

	if (status & PCI_STAT_INTR)
		puts("INTR ");
	if (status & PCI_STAT_MULT_FUNC)
		puts("MULT_FUNC ");
	if (status & PCI_STAT_66MHZ)
		puts("66MHZ ");
	if (status & PCI_STAT_FAST_BACK2BACK)
		puts("FAST_BACK2BACK ");
	if (status & PCI_STAT_DATA_PARITY_ERR)
		puts("DATA_PARITY_ERR ");

	switch (status & PCI_STAT_DEVSEL_MASK) {
	case PCI_STAT_DEVSEL_FAST:
		puts("DEVSEL_FAST ");
		break;
	case PCI_STAT_DEVSEL_MID:
		puts("DEVSEL_MID ");
		break;
	case PCI_STAT_DEVSEL_LOW:
		puts("DEVSEL_LOW ");
		break;
	}

	if (status & PCI_STAT_SND_TARGET_ABORT)
		puts("SND_TARGET_ABORT ");
	if (status & PCI_STAT_RCV_TARGET_ABORT)
		puts("RCV_TARGET_ABORT ");
	if (status & PCI_STAT_RCV_MASTER_ABORT)
		puts("RCV_MASTER_ABORT ");
	if (status & PCI_STAT_SYS_ERR)
		puts("SYS_ERR ");
	if (status & PCI_STAT_PARITY_ERR)
		puts("PARITY_ERR");
	puts("\r\n");
}

void dump_bar(unsigned char bus, unsigned char dev, unsigned char func)
{
	/* PCIコンフィグレーション空間からBARを取得 */
	unsigned int bar = get_pci_conf_reg(bus, dev, func, PCI_CONF_BAR);
	puts("BAR ");
	puth(bar, 8);
	puts("\r\n");

	/* BARのタイプを確認しNICのレジスタのベースアドレスを取得 */
	if (bar & PCI_BAR_MASK_IO) {
		/* IO空間用ベースアドレス */
		puts("IO BASE ");
		puth(bar & PCI_BAR_MASK_IO_ADDR, 8);
		puts("\r\n");
	} else {
		/* メモリ空間用ベースアドレス */
		unsigned int bar_32;
		unsigned long long bar_upper;
		unsigned long long bar_64;
		switch (bar & PCI_BAR_MASK_MEM_TYPE) {
		case PCI_BAR_MEM_TYPE_32BIT:
			puts("MEM BASE 32BIT ");
			bar_32 = bar & PCI_BAR_MASK_MEM_ADDR;
			puth(bar_32, 8);
			puts("\r\n");
			break;

		case PCI_BAR_MEM_TYPE_1M:
			puts("MEM BASE 1M ");
			bar_32 = bar & PCI_BAR_MASK_MEM_ADDR;
			puth(bar_32, 8);
			puts("\r\n");
			break;

		case PCI_BAR_MEM_TYPE_64BIT:
			bar_upper = get_pci_conf_reg(
				bus, dev, func, PCI_CONF_BAR + 4);
			bar_64 = (bar_upper << 32)
				+ (bar & PCI_BAR_MASK_MEM_ADDR);
			puts("MEM BASE 64BIT ");
			puth(bar_64, 16);
			puts("\r\n");
			break;
		}
		if (bar & PCI_BAR_MASK_MEM_PREFETCHABLE)
			puts("PREFETCHABLE\r\n");
		else
			puts("NON PREFETCHABLE\r\n");
	}
}
