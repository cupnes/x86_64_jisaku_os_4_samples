#pragma once

#define PCI_CONF_DID_VID	0x00
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
void dump_vid_did(unsigned char bus, unsigned char dev, unsigned char func);
void dump_bar(unsigned char bus, unsigned char dev, unsigned char func);
