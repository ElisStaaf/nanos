#include "nanos.h"

void setup_memory() {
	logdev_beg(DL0, "setting up the GDT");
	extern void x86_setup_gdt();
	x86_setup_gdt();
	logdev_end(DL0, NULL);
}

void setup_physmm() {
	extern void x86_setup_pmm();
	x86_setup_pmm();
}

void setup_virtmm() {
	extern void x86_setup_vmm();
	x86_setup_vmm();
}
