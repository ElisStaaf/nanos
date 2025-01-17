// Virtual Memory Manager

#include "common.h"
#include "cpu.h"

extern phys_addr_t x86_pmm_new(size_t pgs);

#define PAGE_BIT_P_PRESENT (1 << 0)
#define PAGE_BIT_RW_WRITABLE (1 << 1)
#define PAGE_BIT_US_USER (1 << 2)
#define PAGE_XD_NX (1 << 63)

#define PAGE_ADDR_MASK 0x000FFFFFFFFFF000

#define PAGE_BIT_A_ACCESSED (1 << 5)
#define PAGE_BIT_D_DIRTY (1 << 6)

struct mapping_table {
	uint64_t ent[512];
} __attribute__((packed));

static struct mapping_table *pml4;

static void map_page(uint64_t log, uint64_t phys, int flags) {
	int pml4_idx = (log >> 39) & 0x1ff;
	int pdp_idx = (log >> 30) & 0x1ff;
	int pd_idx = (log >> 21) & 0x1ff;
	int pt_idx = (log >> 12) & 0x1ff;

	// putf(DL0, "pml4_idx: %d, pdp_idx: %d, pd_idx: %d, pt_idx: %d\n",
	// 	pml4_idx, pdp_idx, pd_idx, pt_idx);

#define PAGE_MAP(DIR, IDX) \
	if(!(DIR->ent[IDX] & PAGE_BIT_P_PRESENT)) { \
		uint64_t alloc = (uint64_t)x86_pmm_new(1); \
		memza64((uint64_t*)alloc, 4096 / 8); \
		DIR->ent[IDX] = (alloc & PAGE_ADDR_MASK) | flags; \
		map_page(alloc, phys, flags); \
	}

	PAGE_MAP(pml4, pml4_idx);

	struct mapping_table *pdpt =
		(struct mapping_table*)(pml4->ent[pml4_idx] & PAGE_ADDR_MASK);

	PAGE_MAP(pdpt, pdp_idx);

	struct mapping_table *pdt =
		(struct mapping_table*)(pdpt->ent[pdp_idx] & PAGE_ADDR_MASK);

	PAGE_MAP(pdt, pd_idx);

	struct mapping_table *pt =
		(struct mapping_table*)(pdt->ent[pd_idx] & PAGE_ADDR_MASK);

	pt->ent[pt_idx] = (phys & PAGE_ADDR_MASK) | flags;
}

/* map `count` physical pages starting from `page` to `count` virtual pages starting at `vpage`  */
void vmm_map_phys_pages(phys_addr_t page, size_t count, void *vpage, enum vmm_perms perms) {
	for(int i = 0; i < count; ++i) {
		int flags = PAGE_BIT_P_PRESENT
			| (perms & VMM_PERM_WRIT ? PAGE_BIT_RW_WRITABLE : 0)
			| (perms & VMM_PERM_USER ? PAGE_BIT_US_USER : 0);
		// putf(DL0, "mapping page %p -> %p\n", vpage + i * 4096, page + i * 4096);
		map_page((uint64_t)vpage + i * 4096, (uint64_t)page + i * 4096, flags);
	}
	putc(DL0, '\n');
	extern void x86_load_pml4(void *pml4);
	x86_load_pml4(pml4);
	asm volatile("invlpg [0]");
}

void x86_setup_vmm() {
    uint64_t pml4_addr;
    asm volatile("movq %%cr3, %0" : "=a" (pml4_addr) : );
    asm volatile("movq %0, %%cr3" :  : "a" (pml4_addr));

	pml4 = (void*)pml4_addr;
	putf(DL0, "PML4 location: %p\n", pml4);
}
