#include "nanos.h"

#ifndef NANOS_USERSP_ENTRY
#define NANOS_USERSP_ENTRY 0x10000ULL
#endif

#ifndef NANOS_USERSP_STACK
#define NANOS_USERSP_STACK 0x2000ULL
#endif

struct proc {
  save_frame_t frame;
  void *ret;
  phys_addr_t buf;
  size_t page_amt;
};

#define PROC_SIZE "216"

static struct proc *procs;
static size_t proc_idx;
static size_t proc_amt;

#define IS_PROC_KERNEL(PROC) ((PROC) == &procs[0])

procp_t new_proc() { return get_proc(proc_amt++); }
procp_t get_proc(size_t idx) { return &procs[idx]; }

void init_proc(procp_t p, size_t len, void *buf) {
  memza64((uint64_t*)&p->frame.regs, sizeof(registers_t) / 8);
  p->ret = (void*)NANOS_USERSP_ENTRY;
  p->page_amt = (((len + 0x1000 - 1) & -0x1000) + NANOS_USERSP_STACK) / 0x1000;
  p->frame.rsp = p->frame.rbp = NANOS_USERSP_ENTRY + len + NANOS_USERSP_STACK;
  p->buf = pmm_alloc_pages(p->page_amt);
  memcpy(p->buf, buf, len);
}

void *usersp_hndl_(procp_t from, procp_t to, registers_t *t, void *data) {
  from->ret = data;
  memcpy(&from->frame.regs, t, sizeof(registers_t));
  if(!IS_PROC_KERNEL(from)) {
    vmm_map_phys_pages((phys_addr_t)NANOS_USERSP_ENTRY, from->page_amt, (virt_addr_t)NANOS_USERSP_ENTRY, VMM_PERM_EXEC);
  }
  memcpy(t, &to->frame.regs, sizeof(registers_t));
  if(!IS_PROC_KERNEL(to)) {
    vmm_map_phys_pages((phys_addr_t)NANOS_USERSP_ENTRY, from->page_amt, (virt_addr_t)from->buf, VMM_PERM_EXEC);
  }

  return to->ret;
}

void *timer_hndl_() {
	return NULL;
}

uint64_t should_ctx_switch() {
	static int tlast = 0;
	return tlast++ > 10;
}

/* BUGS: check sizeof(struct proc) */
asm(
"x86_timer_int_handler:\n"        // function x86_timer_int_handler(rdi, rsi, rdx) -- perform context switch between procedures
"   callq should_ctx_switch\n"    //     (rax ←) should_ctx_switch()
"   cmpq $0, %rax\n"              //     rax ↔ 0
"   je timer_hndl_\n"             //     tailcall timer_hndl_ if equal
"   movq %rdi, %rbx\n"            //     rbx ← rdi<old#1>
"   movq %rdx, %rcx\n"            //     rcx<#4> ← rdx<old#3>
"   movq proc_idx, %rdi\n"        //     rdi<#1> ← proc_idx -- ptr to cur proc
"   movq proc_idx, %rax\n"        //     rax ← proc_idx -- ptr to next proc
"   xorl %edx, %edx\n"            //     edx ← 0
"   addq $1, %rax\n"              //     rax ← rax + 1
"   movq %rax, proc_idx\n"        //     proc_idx ← rax
"   divq proc_amt\n"              //     rax<div>|rdx<rem> ← rax / proc_amt
"   movq %rdx, %rsi\n"            //     rsi<#2> ← rdx
"   testq %rsi, %rsi\n"           //     test rsi & rsi -- don't allow `to` to be 0
"   xorl %eax, %eax\n"            //     eax ← 0
"   addq %rax, %rsi\n"            //     rsi ← rsi + eax
"   movq %rbx, %rdx\n"            //     rdx<#3> ← rbx (← <old#1>)
"   movq %rsp, 0(%rsi)\n"         //     rsi:save_frame_t.rsp ← rsp -- save stack
"   movq %rbp, 8(%rsi)\n"         //     rsi:save_frame_t.rbp ← rbp
"   movq 0(%rdi), %rsp\n"         //     rsp ← rdi:save_frame_t.rsp -- restore other stack
"   movq 8(%rdi), %rbp\n"         //     rbp ← rdi:save_frame_t.rbp
"   callq usersp_hndl_\n"         //     (rax ←) usersp_hndl_(rdi, rsi, rdx, rcx)
".E:retq"                         // return
);
asm(
"x86_syscall_int_handler:\n"       // function x86_syscall_int_handler
"   movq %rsp, (usersp_state+0)\n" //     usersp_state.rsp ← rsp -- save userspace stack
"   movq %rbp, (usersp_state+8)\n" //     usersp_state.rbp ← rbp
"   movq (kernel_state+0), %rsp\n" //     rsp ← kernel_state.rsp -- restore kernel stack
"   movq (kernel_state+8), %rbp\n" //     rbp ← kernel_state.rbp
"   callq jump_usersp_int_\n"      //     (rax ←) jump_usersp_int_(rdi, rsi, rdx)
"   movq %rsp, (kernel_state+0)\n" //     kernel_state.rsp ← rsp -- save kernel stack
"   movq %rbp, (kernel_state+8)\n" //     kernel_state.rbp ← rbp
"   movq (usersp_state+0), %rsp\n" //     rsp ← usersp_state.rsp -- restore userspace stack
"   movq (usersp_state+8), %rbp\n" //     rbp ← usersp_state.rbp
"   retq"                          // return
);
asm(
"jump_to_usersp:\n"                // function jump_to_usersp
"   int $40\n"                     //     int 40
"   retq"                          // return
);

void setup_usersp() {
  procs = pmm_alloc_pages(1); // TODO
  memza64((uint64_t*)procs, 0x1000 / 8);
  proc_idx = 0;
  proc_amt = 1;
}
