#ifndef _ACPI_H_
#define _ACPI_H_

struct __attribute__((packed)) SDTH {
	char Signature[4];
	unsigned int Length;
	unsigned char Revision;
	unsigned char Checksum;
	char OEMID[6];
	char OEM_Table_ID[8];
	unsigned int OEM_Revision;
	unsigned int Creator_ID;
	unsigned int Creator_Revision;
};

struct __attribute__((packed)) ACPI_ADDRESS {
	unsigned char address_space_id;
	unsigned char register_bit_width;
	unsigned char register_bit_offset;
	unsigned char _reserved;
	unsigned long long address;
};

void acpi_init(void *rsdp);
void dump_sdth_sig(struct SDTH *h);
void dump_xsdt(void);
struct SDTH *get_sdt(char *sig);

#endif
