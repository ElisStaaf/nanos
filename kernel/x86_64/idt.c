// IDT Management and Handlers

#include "common.h"
#include "../opt/asm.h"

typedef struct {
  uint16_t off1;
  uint16_t sel;
  uint8_t zero1;
  uint8_t type;
  uint16_t off2;
  uint32_t off3;
  uint32_t zero2;
} idt_entry_t;

static idt_entry_t *idt;

static void init_idt_entry(idt_entry_t *e, uint64_t isr) {
  e->off1 = isr;
  e->off2 = isr >> 16;
  e->off3 = isr >> 32;
  e->sel = 0x08;
  e->type = 0x8E;
  e->zero1 = 0;
  e->zero2 = 0;
}

static int received_test = 0;

int x86_interrupt_test() {
  received_test = 0;
  asm("int $1");
  return received_test;
}

struct x86_int_handler { x86_int_handler_func_t func; void *data; };
static struct x86_int_handler *handlers = NULL;
void x86_set_handler(int idx, x86_int_handler_func_t func, void *data) {
  if(handlers == NULL) {
    handlers = pmm_alloc_pages(1);
    memza64((uint64_t*)handlers, 512);
  }
  handlers[idx].func = func;
  handlers[idx].data = data;
}

const char *get_fatal_int_name(int i) {
  switch(i) {
  case 14: return "Page Fault";
  case 13: return "General Protection Fault";
  case 6: return "Invalid Opcode";
  }
  return "See Intel Manual";
}

void *x86_int_handler(struct x86_stack_frame *frame, uint64_t code) {
  switch(code)
  {
  case 1:
    received_test = 1;
    if(handlers && handlers[1].func) {
      return handlers[1].func(frame, code, handlers[1].data);
    }
    break;
  case 14:
    puts(DL0, "\033[31m");
    opt_printdisasm((void*)frame->rip, (void*)frame->rip, (size_t)-1, 3);
  case 13:
  case 6:
    panic("fatal interrupt %d (%s) @ %p", code, get_fatal_int_name(code), frame->rip);
  default:
    putfn(DL1, "int #%d", (int)code);
    if(handlers && handlers[code].func)
      return handlers[code].func(frame, code, handlers[code].data);
    break;
  }

  return NULL;
}

static struct {
  uint16_t size;
  uint64_t off;
} __attribute__((packed)) idt_ptr = {
  256 * sizeof(idt_entry_t) - 1, 0
};

void x86_setup_idt() {
  puts(DL0, "setting ISRs... ");

  idt = x86_pmm_new_a(256 * sizeof(idt_entry_t));
  idt_ptr.off = (uint64_t)idt;

#define X(N) { extern void x86_isr##N(); \
               init_idt_entry(&idt[N], (uint64_t)&x86_isr##N); }
  X(0);
  X(1);
  X(2);
  X(3);
  X(4);
  X(5);
  X(6);
  X(7);
  X(8);
  X(9);
  X(10);
  X(11);
  X(12);
  X(13);
  X(14);
  X(15);
  X(16);
  X(17);
  X(18);
  X(19);
  X(20);
  X(21);
  X(22);
  X(23);
  X(24);
  X(25);
  X(26);
  X(27);
  X(28);
  X(29);
  X(30);
  X(31);

  asm volatile("cli");

  putsn(DL0, "done");
  puts(DL0, "remapping the PIC... ");
  x86_pic_remap(32, 48);
  putsn(DL0, "done");

  puts(DL0, "loading the IDT... ");
  extern void x86_load_idt(void *ptr);
  x86_load_idt(&idt_ptr);
  putsn(DL0, "done");

  asm volatile("sti");
}
