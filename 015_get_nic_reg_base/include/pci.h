#pragma once

#define PCI_CONF_DID_VID	0x00
#define PCI_CONF_STATUS_COMMAND	0x04

#define PCI_COM_IO_EN	(1U << 0)
#define PCI_COM_MEM_EN	(1U << 1)
#define PCI_COM_BUS_MASTER_EN	(1U << 2)
#define PCI_COM_SPECIAL_CYCLE	(1U << 3)
#define PCI_COM_MEMW_INV_EN	(1U << 4)
#define PCI_COM_VGA_PAL_SNP	(1U << 5)
#define PCI_COM_PARITY_ERR_RES	(1U << 6)
#define PCI_COM_SERR_EN	(1U << 8)
#define PCI_COM_FAST_BACK2BACK_EN	(1U << 9)
#define PCI_COM_INTR_DIS	(1U << 10)

#define PCI_STAT_INTR	(1U << 3)
#define PCI_STAT_MULT_FUNC	(1U << 4)
#define PCI_STAT_66MHZ	(1U << 5)
#define PCI_STAT_FAST_BACK2BACK	(1U << 7)
#define PCI_STAT_DATA_PARITY_ERR	(1U << 8)
#define PCI_STAT_DEVSEL_MASK	(3U << 9)
#define PCI_STAT_DEVSEL_FAST	(0b00 << 9)
#define PCI_STAT_DEVSEL_MID	(0b01 << 9)
#define PCI_STAT_DEVSEL_LOW	(0b10 << 9)
#define PCI_STAT_SND_TARGET_ABORT	(1U << 11)
#define PCI_STAT_RCV_TARGET_ABORT	(1U << 12)
#define PCI_STAT_RCV_MASTER_ABORT	(1U << 13)
#define PCI_STAT_SYS_ERR	(1U << 14)
#define PCI_STAT_PARITY_ERR	(1U << 15)

#define PCI_CONF_BAR	0x10

#define PCI_BAR_MASK_IO	0x00000001
#define PCI_BAR_MASK_MEM_TYPE	0x00000006
#define PCI_BAR_MEM_TYPE_32BIT	0x00000000
#define PCI_BAR_MEM_TYPE_1M	0x00000002
#define PCI_BAR_MEM_TYPE_64BIT	0x00000004
#define PCI_BAR_MASK_MEM_PREFETCHABLE	0x00000008
#define PCI_BAR_MASK_MEM_ADDR	0xfffffff0
#define PCI_BAR_MASK_IO_ADDR	0xfffffffc

unsigned int get_pci_conf_reg(
	unsigned char bus, unsigned char dev, unsigned char func,
	unsigned char reg);
void set_pci_conf_reg(unsigned char bus, unsigned char dev, unsigned char func,
		      unsigned char reg, unsigned int val);
void dump_vid_did(unsigned char bus, unsigned char dev, unsigned char func);
void dump_command_status(
	unsigned char bus, unsigned char dev, unsigned char func);
void dump_bar(unsigned char bus, unsigned char dev, unsigned char func);
