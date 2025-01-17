// GDT Setup

#include "common.h"

struct gdt_entry {
	uint16_t limit0;
	uint16_t base0;
	uint8_t base1;
	uint8_t access;
	uint8_t limit1 : 4;
	uint8_t flags : 4;
	uint8_t base2;
} __attribute__((packed));

static struct {
    uint32_t res0; uint64_t rsp0; uint64_t rsp1;
    uint64_t rsp2; uint64_t res1; uint64_t ist1;
    uint64_t ist2; uint64_t ist3; uint64_t ist4;
    uint64_t ist5; uint64_t ist6; uint64_t ist7;
    uint64_t res2; uint16_t res3; uint16_t iopb;
} __attribute__((packed)) tss = { 0 };

static struct {
	struct gdt_entry null;
	struct gdt_entry kcode;
	struct gdt_entry kdata;
	struct gdt_entry udata;
	struct gdt_entry ucode;
	struct gdt_entry tsslo;
	struct gdt_entry tsshi;
} __attribute__((packed)) gdt = {
	{ 0 },
	{ 0xFFFF, 0, 0, 0x9A, 0xF, 0xA, 0 },
	{ 0xFFFF, 0, 0, 0x92, 0xF, 0xC, 0 },
	{ 0xFFFF, 0, 0, 0xF2, 0xF, 0xC, 0 },
	{ 0xFFFF, 0, 0, 0xFA, 0xF, 0xA, 0 },
	{ 0x0000, 0, 0, 0x89, 0x0, 0x0, 0 },
	{ 0 },
};

static struct {
	uint16_t limit;
	uint64_t base;
} __attribute__((packed)) gdtptr;

void x86_setup_gdt() {
	uint64_t tssaddr = (uint64_t)&tss;
	gdt.tsslo.base0 = tssaddr >> 00;
	gdt.tsslo.base1 = tssaddr >> 16;
	gdt.tsslo.base2 = tssaddr >> 24;
	gdt.tsslo.limit0 = sizeof(tss) - 1;
	gdt.tsshi.limit0 = tssaddr >> 32;
	gdt.tsshi.base0 = tssaddr >> 48;

	gdtptr.base = (uint64_t)&gdt;
	gdtptr.limit = sizeof(gdt) - 1;

	puts(DL0, "loading the GDT... ");
	extern void x86_lgdt(void*);
	x86_lgdt(&gdtptr);
	putsn(DL0, "done!");

	puts(DL0, "loading the TSS... ");
	extern void x86_ltr(uint16_t);
	x86_ltr(0x28);
	putsn(DL0, "done!");
}
