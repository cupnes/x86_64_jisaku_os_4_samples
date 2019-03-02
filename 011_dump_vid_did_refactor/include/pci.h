#pragma once

#define PCI_CONF_DID_VID	0x00

unsigned int get_pci_conf_reg(
	unsigned char bus, unsigned char dev, unsigned char func,
	unsigned char reg);
void dump_vid_did(unsigned char bus, unsigned char dev, unsigned char func);
