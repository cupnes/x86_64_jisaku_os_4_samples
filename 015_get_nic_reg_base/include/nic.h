#pragma once

#define NIC_BUS_NUM	0x00
#define NIC_DEV_NUM	0x19
#define NIC_FUNC_NUM	0x0

void nic_init(void);
unsigned int get_nic_reg_base(void);
