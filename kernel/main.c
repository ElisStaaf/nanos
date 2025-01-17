#include "nanos.h"
#include "limine.h"

struct limine_bootloader_info_request bootloader_info = {
  .id = LIMINE_BOOTLOADER_INFO_REQUEST,
  .revision = 0
};

struct limine_module_request limine_module_req = {
  .id = LIMINE_MODULE_REQUEST,
  .revision = 0
};

void log_kerninfo(devp_t d) {
  putfn(d, "version: %s", NANOS_VERSION);
  putfn(d, "author: %s", "Elis Staaf");
  putfn(d, "contribs: %s", NANOS_CONTRIBS);
  putfn(d, "bootloader: %s %s", bootloader_info.response->name,
                                bootloader_info.response->version);
}

static size_t strstart(const char *a, const char *b) {
  size_t d = 0;
  do d += *b++ - *a++; while(*a && *b);
  return d;
}

void *memcpy(void *restrict dest, const void *restrict src, size_t count) {
  asm volatile("cld\n rep movsb" : "+c" (count), "+S" (src), "+D" (dest) :: "memory");
  return dest;
}

struct kernel_state {
  uint64_t rsp, rbp;
  int retcode;
} kernel_state;

struct usersp_state {
  uint64_t rsp, rbp;
} usersp_state;

struct kernel_functions kernel_funcs;

struct syscall {
  int argc;
  void *(*func)(void *data, ...);
};

const char *syscall_names[] = {
  "sys_exit",
  "sys_funcs"
};

void *jump_usersp_int_(struct x86_stack_frame *frame, uint64_t code, void *data, int *val) {
  putfn(DL0, "syscall: %lu (%s)", frame->rax, syscall_names[frame->rax]);
  switch(frame->rax) {
  case 0:
    kernel_state.retcode = frame->rdi;
    putfn(DL0, "exiting back to %p", data);
    return data;
  case 1:
    putfn(DL0, "copying functions to %p", (void*)frame->rdi);
    memcpy((void*)frame->rdi, &kernel_funcs, sizeof(kernel_funcs));
    break;
  case 2:
    putsn(DL0, "debug");
    break;
  default:
    putfn(DL0, "unknown syscall: %lu", frame->rax);
    break;
  }
  return NULL;
}

/*
asm (
"x86_jump_to_usersp:\n"               // x86_jump_to_usersp:
"   movq %rdi, %r8\n"                 // r8 ← rdi(code)
"   movq %rsi, %r9\n"                 // r9 ← rsi(stack)
"   movq $1, %rdi\n"                  // rdi ← 1
"   movq $x86_usersp_handler, %rsi\n" // rsi ← jump_usersp_int_
"   movq $.end_is_near, %rdx\n"       // rdx ← .end_is_near
"   callq x86_set_handler\n"          // x86_set_handler(rdi, rsi, rdx)
"   movq %rsp, (kernel_state+0)\n"    // kernel_state.rsp ← rsp
"   movq %rbp, (kernel_state+8)\n"    // kernel_state.rbp ← rbp
"   movq %r9, %rsp\n"                 // rsp ← r9(stack)
"   movq %r9, %rbp\n"                 // rbp ← r9(stack)
"   jmpq *%r8\n"                      // jmp → r8(code)
".end_is_near:\n"                     // .end_is_near:
"   movl (kernel_state+16), %eax\n"   // rax ← kernel_state.retcode
"   movq (kernel_state+0), %rsp\n"    // rsp ← kernel_state.rsp -- restore kernel stack
"   movq (kernel_state+8), %rbp\n"    // rbp ← kernel_state.rbp
"   retq\n"                           // return
); */

int x86_jump_to_usersp(void *code, void *stack);

static int jump_usersp(void *code, void *stack) {
  kernel_funcs.scrdev_getscr = &scrdev_getscr;
  kernel_funcs.DSCR = DSCR;
  kernel_funcs.DL0 = DL0;
  kernel_funcs.putf = &putf;
  kernel_funcs.putfn = &putfn;
  kernel_funcs.vputf = &putf;
  kernel_funcs.puts = &puts;
  kernel_funcs.putc = &putc;

  return x86_jump_to_usersp(code, stack);
}

void entry() {
  extern void x86_enable_sse();
  x86_enable_sse();

  setup_serial();
  setup_syslog();
  setup_scrlog();

  int stack_check = 0;
  putfn(DL0, "stack begin: %p", &stack_check);

  muldev_t logdev_muldev;
  if(DSCR != NULL) {
    devp_t logdev_mul_ds[] = { DS0, DSCR };
    muldev_init(&logdev_muldev, 2, logdev_mul_ds);
    logdev_set_tgt(DL0, &logdev_muldev.dev);
    logdev_set_tgt(DL1, &logdev_muldev.dev);
  }

  void *inittar = NULL;
  const char *initbin = "hello.c.bin";
  if(limine_module_req.response != NULL) {
    logdev_beg(DL0, "%lu module%s:", limine_module_req.response->module_count,
      limine_module_req.response->module_count == 1 ? "" : "s");
    for(int i = 0; i < limine_module_req.response->module_count; ++i) {
      struct limine_file *files = *limine_module_req.response->modules;
      struct limine_file *file = &files[i];
      if(file == NULL) continue;
      logdev_beg(DL0, file->path);
      putf(DL0,
        "address = %p, size = %lu, path = '%s', cmdline = '%s'",
        file->address,
        file->size,
        file->path,
        file->cmdline
      );
      if(strstart(file->cmdline, "init;") == 0) {
        putsn(DL0, " - init file");
        inittar = file->address;
        initbin = file->cmdline + 5;
      } else {
        putsn(DL0, " - unknown");
      }
      logdev_end(DL0, NULL);
    }
  }
  logdev_end(DL0, NULL);

  logdev_beg(DL0, "nanos :)");
  log_kerninfo(DL0);
  logdev_end(DL0, NULL);

  logdev_beg(DL0, "setting up physical memory management:");
  setup_physmm();
  logdev_end(DL0, NULL);

  logdev_beg(DL0, "setting up virtual memory management:");
  setup_virtmm();
  logdev_end(DL0, NULL);

  logdev_beg(DL0, "setting up memory:");
  setup_memory();
  logdev_end(DL0, NULL);

  logdev_beg(DL0, "setting up interrupts:");
  setup_intrps();
  logdev_end(DL0, NULL);

  logdev_beg(DL0, "setting up userspace");
  setup_usersp();
  logdev_end(DL0, NULL);

  if(inittar != NULL) {
    putsn(DL0, "loading init");
    setup_tardev(inittar);
    struct tardev_file file;
    tardev_get(DINIT, initbin, &file);
    putfn(DL0, "name = %s, mode = %u,\nsize = %lu, data = %p", file.name, file.mode, file.size, file.data);
    procp_t p = new_proc();
    putfn(DL0, "\n[exited with code %d]", inittar);
  }

  forever asm("hlt");
}
