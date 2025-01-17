#ifndef NANOS_X86_64_ARCH_H_
#define NANOS_X86_64_ARCH_H_
#include "../common.h"

static inline void halt() { for(;;) asm("hlt"); }

static inline unsigned int clzll(uint64_t n) { return __builtin_clzll(n); }
static inline unsigned int clz(uint32_t n) { return __builtin_clz(n); }

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1"
                :
                : "a"(val), "Nd"(port)
                );
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0"
                : "=a"(ret)
                : "Nd"(port)
                );
    return ret;
}

static inline void x86_wrmsr(uint64_t msr, uint64_t value) {
	uint32_t low = value & 0xffffffff;
	uint32_t high = value >> 32;
	asm volatile("wrmsr"
		        :
		        : "c"(msr), "a"(low), "d"(high)
	            );
}

static inline uint64_t x86_rdmsr(uint64_t msr) {
	uint32_t low, high;
	asm volatile("rdmsr"
		        : "=a"(low), "=d"(high)
		        : "c"(msr)
	            );
	return ((uint64_t)high << 32) | low;
}

void x86_pic_remap(uint32_t offset1, uint32_t offset2);
void *x86_pmm_new(size_t pgs);
void *x86_pmm_new_a(size_t sz);

struct x86_stack_frame {
  uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
  uint64_t rsi, rdi, rbp, rdx, rcx, rbx, rax;
  uint64_t err, rip, cs, rfl, rsp, ss;
};

typedef struct x86_stack_frame syscall_frame_t;

static inline uint64_t syscall_conv_arg(syscall_frame_t *sframe, int i) {
	static size_t syscall_conv_args[6] = {
		offsetof(struct x86_stack_frame, rdi),
		offsetof(struct x86_stack_frame, rsi),
		offsetof(struct x86_stack_frame, rdx),
		offsetof(struct x86_stack_frame, rcx),
		offsetof(struct x86_stack_frame, r8),
		offsetof(struct x86_stack_frame, r9),
	};
	return ((uint64_t*)sframe)[syscall_conv_args[i]];
}

static inline void syscall_conv_out(syscall_frame_t *sframe, uint64_t val) {
	sframe->rax = val;
}

typedef void *(*x86_int_handler_func_t)(struct x86_stack_frame *frame, uint64_t code, void *data);
void x86_set_handler(int idx, x86_int_handler_func_t func, void *data);

#endif
