#pragma once

#define NIC_BUS_NUM	0x00
#define NIC_DEV_NUM	0x19
#define NIC_FUNC_NUM	0x0

#define NIC_REG_IMS	0x00d0
#define NIC_REG_IMC	0x00d8
#define NIC_REG_RDBAL	0x2800
#define NIC_REG_RDBAH	0x2804
#define NIC_REG_RDLEN	0x2808
#define NIC_REG_RDH	0x2810
#define NIC_REG_RDT	0x2818

void nic_init(void);
unsigned int get_nic_reg_base(void);
unsigned int get_nic_reg(unsigned short reg);
void set_nic_reg(unsigned short reg, unsigned int val);
void dump_nic_ims(void);
