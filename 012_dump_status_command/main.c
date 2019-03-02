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
#include <pci.h>
#include <nic.h>

struct __attribute__((packed)) platform_info {
	struct framebuffer fb;
	void *rsdp;
};

#define INIT_APP	"test"

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



	/* PCIコンフィグレーション空間のレジスタを読み出す */
	unsigned int conf_data = get_pci_conf_reg(
		NIC_BUS_NUM, NIC_DEV_NUM, NIC_FUNC_NUM,
		PCI_CONF_STATUS_COMMAND);

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
