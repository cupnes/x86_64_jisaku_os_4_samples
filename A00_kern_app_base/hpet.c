#include <intr.h>
#include <pic.h>
#include <acpi.h>
#include <fbcon.h>
#include <hpet.h>
#include <common.h>

#define TIMER_N		0	/* 使用するタイマー番号 */

#define US_TO_FS	1000000000

struct __attribute__((packed)) HPET_TABLE {
	struct SDTH header;
	unsigned int event_timer_block_id;
	struct ACPI_ADDRESS base_address;
	unsigned char hpet_number;
	unsigned short minimum_tick;
	unsigned char flags;
};

unsigned long long reg_base;
unsigned int counter_clk_period;
unsigned long long cmpr_clk_counts;
unsigned char is_oneshot = 0;

/* General Capabilities and ID Register */
#define GCIDR_ADDR	(reg_base)
#define GCIDR	(*(volatile unsigned long long *)GCIDR_ADDR)
union gcidr {
	unsigned long long raw;
	struct __attribute__((packed)) {
		unsigned long long rev_id:8;
		unsigned long long num_tim_cap:5;
		unsigned long long count_size_cap:1;
		unsigned long long _reserved:1;
		unsigned long long leg_rt_cap:1;
		unsigned long long vendor_id:16;
		unsigned long long counter_clk_period:32;
	};
};

/* General Configuration Register */
#define GCR_ADDR	(reg_base + 0x10)
#define GCR	(*(volatile unsigned long long *)GCR_ADDR)
union gcr {
	unsigned long long raw;
	struct __attribute__((packed)) {
		unsigned long long enable_cnf:1;
		unsigned long long leg_rt_cnf:1;
		unsigned long long _reserved:62;
	};
};

/* Main Counter Register */
#define MCR_ADDR	(reg_base + 0xf0)
#define MCR	(*(volatile unsigned long long *)MCR_ADDR)

/* Timer N Configuration and Capabilities Register */
#define TNCCR_ADDR(n)	(reg_base + (0x20 * (n)) + 0x100)
#define TNCCR(n)	(*(volatile unsigned long long *)(TNCCR_ADDR(n)))
#define TNCCR_INT_TYPE_EDGE	0
#define TNCCR_INT_TYPE_LEVEL	1
#define TNCCR_TYPE_NON_PERIODIC	0
#define TNCCR_TYPE_PERIODIC	1
union tnccr {
	unsigned long long raw;
	struct __attribute__((packed)) {
		unsigned long long _reserved1:1;
		unsigned long long int_type_cnf:1;
		unsigned long long int_enb_cnf:1;
		unsigned long long type_cnf:1;
		unsigned long long per_int_cap:1;
		unsigned long long size_cap:1;
		unsigned long long val_set_cnf:1;
		unsigned long long _reserved2:1;
		unsigned long long mode32_cnf:1;
		unsigned long long int_route_cnf:5;
		unsigned long long fsb_en_cnf:1;
		unsigned long long fsb_int_del_cap:1;
		unsigned long long _reserved3:16;
		unsigned long long int_route_cap:32;
	};
};

/* Timer N Comparator Register */
#define TNCR_ADDR(n)	(reg_base + (0x20 * (n)) + 0x108)
#define TNCR(n)	(*(volatile unsigned long long *)(TNCR_ADDR(n)))

void hpet_handler(void);
void (*user_handler)(unsigned long long current_rsp) = NULL;

void hpet_init(void)
{
	/* HPET tableを取得 */
	struct HPET_TABLE *hpet_table = (struct HPET_TABLE *)get_sdt("HPET");

	/* レジスタの先頭アドレスを取得 */
	reg_base = hpet_table->base_address.address;

	/* 使うまでHPETは止めておく
	 * 併せてLegacy Replacement Route有効化 */
	union gcr gcr;
	gcr.raw = GCR;
	gcr.enable_cnf = 0;
	gcr.leg_rt_cnf = 1;
	GCR = gcr.raw;

	/* カウント周期を取得 */
	union gcidr gcidr;
	gcidr.raw = GCIDR;
	counter_clk_period = gcidr.counter_clk_period;

	/* 割り込み設定初期化 */
	union tnccr tnccr;
	tnccr.raw = TNCCR(TIMER_N);
	tnccr.int_type_cnf = TNCCR_INT_TYPE_EDGE;
	tnccr.int_enb_cnf = 0;
	tnccr.type_cnf = TNCCR_TYPE_NON_PERIODIC;
	tnccr.val_set_cnf = 0;
	tnccr.mode32_cnf = 0;
	tnccr.fsb_en_cnf = 0;
	tnccr._reserved1 = 0;
	tnccr._reserved2 = 0;
	tnccr._reserved3 = 0;
	TNCCR(TIMER_N) = tnccr.raw;

	/* IDTへHPET割り込みのハンドラ登録 */
	set_intr_desc(HPET_INTR_NO, hpet_handler);

	/* PICの割り込みマスク解除 */
	enable_pic_intr(HPET_INTR_NO);
}

void dump_gcidr(void)
{
	puts("GCIDR\r\n");

	union gcidr r;
	r.raw = GCIDR;

	puts("REV ID             ");
	putd(r.rev_id, 3);
	puts("\r\n");

	puts("NUM TIM CAP        ");
	putd(r.num_tim_cap, 2);
	puts("\r\n");

	puts("COUNT SIZE CAP     ");
	putd(r.count_size_cap, 1);
	puts("\r\n");

	puts("LEG RT CAP         ");
	putd(r.leg_rt_cap, 1);
	puts("\r\n");

	puts("COUNTER CLK PERIOD ");
	putd(r.counter_clk_period, 10);
	puts("\r\n");
}

void dump_gcr(void)
{
	puts("GCR\r\n");

	union gcr r;
	r.raw = GCR;

	puts("ENABLE CNF ");
	putd(r.enable_cnf, 1);
	puts("\r\n");

	puts("LEG RT CNF ");
	putd(r.leg_rt_cnf, 1);
	puts("\r\n");
}

void dump_mcr(void)
{
	puts("MCR ");
	puth(MCR, 16);
	puts("\r\n");
}

void sleep(unsigned long long us)
{
	/* 現在のmain counterのカウント値を取得 */
	unsigned long long mc_now = MCR;

	/* usマイクロ秒後のmain counterのカウント値を算出 */
	unsigned long long fs = us * US_TO_FS;
	union gcidr gcidr;
	gcidr.raw = GCIDR;
	unsigned long long mc_duration = fs / gcidr.counter_clk_period;
	unsigned long long mc_after = mc_now + mc_duration;

	/* HPETが無効であれば有効化する */
	union gcr gcr;
	gcr.raw = GCR;
	unsigned char to_disable = 0;
	if (!gcr.enable_cnf) {
		gcr.enable_cnf = 1;
		GCR = gcr.raw;

		/* sleep()を抜ける際に元に戻す(disableする) */
		to_disable = 1;
	}

	/* usマイクロ秒の経過を待つ */
	while (MCR < mc_after);

	/* 元々無効であった場合は無効に戻しておく */
	if (to_disable) {
		gcr.raw = GCR;
		gcr.enable_cnf = 0;
		GCR = gcr.raw;
	}
}

void do_hpet_interrupt(unsigned long long current_rsp)
{
	if (is_oneshot == 1) {
		/* HPET無効化 */
		union gcr gcr;
		gcr.raw = GCR;
		gcr.enable_cnf = 0;
		GCR = gcr.raw;

		/* 割り込みを無効化 */
		union tnccr tnccr;
		tnccr.raw = TNCCR(TIMER_N);
		tnccr.int_enb_cnf = 0;
		tnccr._reserved1 = 0;
		tnccr._reserved2 = 0;
		tnccr._reserved3 = 0;
		TNCCR(TIMER_N) = tnccr.raw;

		/* ワンショットタイマー設定を解除 */
		is_oneshot = 0;
	}

	/* ユーザーハンドラを呼び出す */
	if (user_handler)
		user_handler(current_rsp);

	/* PICへ割り込み処理終了を通知(EOI) */
	set_pic_eoi(HPET_INTR_NO);
}

void alert(unsigned long long us, void *handler)
{
	/* ユーザーハンドラ設定 */
	user_handler = handler;

	/* 非周期割り込みで割り込み有効化 */
	union tnccr tnccr;
	tnccr.raw = TNCCR(TIMER_N);
	tnccr.int_enb_cnf = 1;
	tnccr.type_cnf = TNCCR_TYPE_NON_PERIODIC;
	tnccr._reserved1 = 0;
	tnccr._reserved2 = 0;
	tnccr._reserved3 = 0;
	TNCCR(TIMER_N) = tnccr.raw;

	/* main counterをゼロクリア */
	MCR = (unsigned long long)0;

	/* コンパレータ設定 */
	unsigned long long femt_sec = us * US_TO_FS;
	unsigned long long clk_counts = femt_sec / counter_clk_period;
	TNCR(TIMER_N) = clk_counts;

	/* ワンショットタイマー設定 */
	is_oneshot = 1;

	/* HPET有効化 */
	union gcr gcr;
	gcr.raw = GCR;
	gcr.enable_cnf = 1;
	GCR = gcr.raw;
}

void ptimer_setup(unsigned long long us, void *handler)
{
	/* HPET無効化 */
	union gcr gcr;
	gcr.raw = GCR;
	gcr.enable_cnf = 0;
	GCR = gcr.raw;

	/* ユーザーハンドラ設定 */
	user_handler = handler;

	/* 周期割り込みで割り込み有効化 */
	union tnccr tnccr;
	tnccr.raw = TNCCR(TIMER_N);
	tnccr.int_enb_cnf = 1;
	tnccr.type_cnf = TNCCR_TYPE_PERIODIC;
	tnccr._reserved1 = 0;
	tnccr._reserved2 = 0;
	tnccr._reserved3 = 0;
	TNCCR(TIMER_N) = tnccr.raw;

	/* コンパレータ設定値を計算しておく */
	unsigned long long femt_sec = us * US_TO_FS;
	cmpr_clk_counts = femt_sec / counter_clk_period;
}

void ptimer_start(void)
{
	/* コンパレータ初期化 */
	union tnccr tnccr;
	tnccr.raw = TNCCR(TIMER_N);
	tnccr.val_set_cnf = 1;
	TNCCR(TIMER_N) = tnccr.raw;
	TNCR(TIMER_N) = cmpr_clk_counts;

	/* main counter初期化 */
	MCR = (unsigned long long)0;

	/* HPET有効化 */
	union gcr gcr;
	gcr.raw = GCR;
	gcr.enable_cnf = 1;
	GCR = gcr.raw;
}

void ptimer_stop(void)
{
	/* HPET無効化 */
	union gcr gcr;
	gcr.raw = GCR;
	gcr.enable_cnf = 0;
	GCR = gcr.raw;

	/* 割り込みを無効化 */
	union tnccr tnccr;
	tnccr.raw = TNCCR(TIMER_N);
	tnccr.int_enb_cnf = 0;
	tnccr._reserved1 = 0;
	tnccr._reserved2 = 0;
	tnccr._reserved3 = 0;
	TNCCR(TIMER_N) = tnccr.raw;
}
