// Physical Memory Manager
#include "common.h"
#include "cpu.h"

static size_t phys_page_addr, phys_page_addr_start;
static size_t avail_memory, used_memory;

phys_addr_t x86_pmm_new_a(size_t sz) {
	// putfn(DL0, "allocating %ld bytes", sz);
	return x86_pmm_new(((sz + 0x1000 - 1) & -0x1000) / 0x1000);
}

phys_addr_t x86_pmm_new(size_t pgs) {
	// putfn(DL0, "allocating %ld pages", pgs);
	pgs *= 0x1000;
	used_memory += pgs;
	if(used_memory >= avail_memory) {
		panic("OUT OF MEMORY");
		return NULL;
	}
	void *addr = (addr_t)phys_page_addr;
	phys_page_addr += pgs;
	return addr;
}

phys_addr_t pmm_alloc_pages(size_t amt) {
	return x86_pmm_new(amt);
}

void x86_setup_pmm() {
	extern addr_t kernel_end;

	size_t kernel_phys_base = limine_kernel_addr_req.response->physical_base;
	size_t kernel_virt_base = limine_kernel_addr_req.response->virtual_base;
	size_t kernel_size = (size_t)&kernel_end - kernel_virt_base;


	avail_memory = 0xFFFFFFFFFFFF; /* TODO: available physical memory */
	used_memory = 0;

	phys_page_addr = phys_page_addr_start =
		(kernel_phys_base + kernel_size + 0x1000 - 1) & -0x1000;

	putfn(DL0, "phys_base = 0x%p", (void*)kernel_phys_base);
	putfn(DL0, "page_addr = 0x%p", (void*)phys_page_addr);
}
