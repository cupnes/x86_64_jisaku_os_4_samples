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



	/* PCIコンフィグレーション空間からBARを取得 */
	unsigned int bar = get_pci_conf_reg(
		NIC_BUS_NUM, NIC_DEV_NUM, NIC_FUNC_NUM, PCI_CONF_BAR);
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
				NIC_BUS_NUM, NIC_DEV_NUM, NIC_FUNC_NUM,
				PCI_CONF_BAR + 4);
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
