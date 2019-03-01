#include <x86.h>
#include <intr.h>
#include <pic.h>
#include <acpi.h>
#include <fb.h>
#include <kbc.h>
#include <fbcon.h>
#include <fs.h>
#include <hpet.h>
#include <sched.h>
#include <common.h>
#include <syscall.h>
#include <proc.h>

struct __attribute__((packed)) platform_info {
	struct framebuffer fb;
	void *rsdp;
};

#define INIT_APP	"test"


/* PCIの定義 */
#define PCI_CONF_DID_VID	0x00

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


/* NICの定義 */
#define NIC_BUS_NUM	0x00
#define NIC_DEV_NUM	0x19
#define NIC_FUNC_NUM	0x0


void start_kernel(void *_t __attribute__((unused)), struct platform_info *pi,
		  void *_fs_start)
{
	/* フレームバッファ周りの初期化 */
	fb_init(&pi->fb);
	set_fg(255, 255, 255);
	set_bg(0, 70, 250);
	clear_screen();

	/* ACPIの初期化 */
	acpi_init(pi->rsdp);

	/* CPU周りの初期化 */
	gdt_init();
	intr_init();

	/* 周辺ICの初期化 */
	pic_init();
	hpet_init();
	kbc_init();



	/* NICのベンダーID・デバイスIDを取得 */

	/* CONFIG_ADDRESSを設定 */
	union pci_config_address conf_addr;
	conf_addr.raw = 0;
	conf_addr.bus_num = NIC_BUS_NUM;
	conf_addr.dev_num = NIC_DEV_NUM;
	conf_addr.func_num = NIC_FUNC_NUM;
	conf_addr.reg_addr = PCI_CONF_DID_VID;
	conf_addr.enable_bit = 1;
	io_write32(CONFIG_ADDRESS, conf_addr.raw);

	/* CONFIG_DATAを読み出す */
	unsigned int conf_data = io_read32(CONFIG_DATA);

	/* 読み出したデータからベンダーID・デバイスIDを取得 */
	unsigned short vendor_id = conf_data & 0x0000ffff;
	unsigned short device_id = conf_data >> 16;
	puth(vendor_id, 4);
	puts("\r\n");
	puth(device_id, 4);

	/* haltして待つ */
	while (1)
		cpu_halt();



	/* システムコールの初期化 */
	syscall_init();

	/* ファイルシステムの初期化 */
	fs_init(_fs_start);

	/* スケジューラの初期化 */
	sched_init();

	/* CPUの割り込み有効化 */
	enable_cpu_intr();

	/* スケジューラの開始 */
	sched_start();

	/* initアプリ起動 */
	exec(open(INIT_APP));

	/* haltして待つ */
	while (1)
		cpu_halt();
}
