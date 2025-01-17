#ifndef NANOS_H_
#define NANOS_H_
#include "common.h"
#include <stdarg.h>

#include "x86_64/arch.h"

// #define SCRDEV_DOUBLE
#define SCRDEV_PAD_LEFT 8
#define SCRDEV_PAD_TOP 8

void *memcpy(void *restrict dest, const void *restrict src, size_t count);

typedef struct dev_tag dev_t;
struct dev_tag {
    dev_t *p;
    void (*write)(dev_t *p, void *d);
    void *(*read)(dev_t *p);
};

typedef dev_t *devp_t;
extern devp_t DS0, DS1,
              DL0, DL1,
              DSCR, DINIT;

#define __argvfmt(a, b)  __attribute__((__format__(__printf__, a, b)))

void putc(devp_t dev, char ch); /* char */
void puts(devp_t dev, const char *str); /* string */
void putsn(devp_t dev, const char *str); /* string + newline */
void putf(devp_t dev, const char *fmt, ...) __argvfmt(2, 3); /* printf-like */
void putfn(devp_t dev, const char *fmt, ...) __argvfmt(2, 3); /* printf-like + newline */
void putfv(devp_t dev, const char *fmt, va_list v); /* vprintf-like */
void putsua(devp_t dev, usize_t value, char base); /* string any-base unsigned int */
void putsia(devp_t dev, ssize_t value, char base); /* string any-base signed int */
void putsud(devp_t dev, usize_t value); /* string decimal unsigned int */
void putsid(devp_t dev, ssize_t value); /* string decimal signed int */
void putsuh(devp_t dev, usize_t value); /* string hexadecimal unsigned int */
void putsih(devp_t dev, ssize_t value); /* string hexadecimal signed int */
void putsub(devp_t dev, usize_t value); /* string binary unsigned int */
void putsib(devp_t dev, ssize_t value); /* string binary signed int */
void putsuo(devp_t dev, usize_t value); /* string octal unsigned int */
void putsio(devp_t dev, ssize_t value); /* string octal signed int */
void putp(devp_t dev, void *a); /* pointer */

typedef struct serdev_tag serdev_t;
void serdev_init(serdev_t *dev, uint16_t port);

typedef struct muldev_tag muldev_t;
struct muldev_tag {
    dev_t dev;
    uint8_t amt;
#define MULDEV_LIM 4
    devp_t ds[MULDEV_LIM];
};
void muldev_init(muldev_t *dev, uint8_t amt, devp_t *ds);

typedef struct screen {
    uint32_t *buf;
    size_t w, h;
    size_t pitch;
} screen_t;

typedef struct scrdev_tag scrdev_t;
void scrdev_init(scrdev_t *dev, screen_t *scr);
screen_t *const scrdev_getscr(scrdev_t *dev);

struct kernel_functions {
    screen_t *const (*scrdev_getscr)(scrdev_t *dev);
    devp_t DSCR, DL0;
    void (*putf)(devp_t dev, const char *fmt, ...);
    void (*putfn)(devp_t dev, const char *fmt, ...);
    void (*vputf)(devp_t dev, const char *fmt, ...);
    void (*puts)(devp_t dev, const char *s);
    void (*putc)(devp_t dev, char c);
};

enum logdev_feat {
    LOGDEV_NONE = 0,
    LOGDEV_ENABLE_COLOR = 1 << 0,
    LOGDEV_ENABLE_INDENT = 1 << 1,
};

typedef struct logdev_tag logdev_t;
void logdev_init(logdev_t *dev, devp_t tgt, enum logdev_feat feats);
void logdev_beg(devp_t dev, const char *fmt, ...); /* log begin */
void logdev_end(devp_t dev, const char *fmt, ...); /* log end */
void logdev_set_tgt(devp_t dev, devp_t tgt);

struct tardev_file {
    const char *name;
    uint16_t mode;
    size_t size;
    void *data;
};

typedef struct tardev_tag tardev_t;
void tardev_init(tardev_t *dev, void *data);
size_t tardev_count(devp_t dev);
void tardev_get(devp_t dev, const char *filename, struct tardev_file *file);

char serial_read(uint16_t port);
void serial_write(uint16_t port, char v);

void setup_syslog();
void setup_serial();
void setup_sysios();
void setup_scrlog();
void setup_memory();
void setup_intrps();
void setup_physmm();
void setup_virtmm();
void setup_usersp();
void setup_tardev(void *data);

static inline void memza64(uint64_t *addr, size_t amt) {
    while(amt--) *addr++ = 0;
}

static inline void memza32(uint32_t *addr, size_t amt) {
    while(amt--) *addr++ = 0;
}

static inline void memza16(uint16_t *addr, size_t amt) {
    while(amt--) *addr++ = 0;
}

static inline void memz(uint8_t *addr, size_t amt) {
    while(amt--) *addr++ = 0;
}

typedef void *phys_addr_t;
typedef void *virt_addr_t;
phys_addr_t pmm_alloc_pages(size_t amt);

enum vmm_perms {
    VMM_PERM_READ = (0 << 0),
    VMM_PERM_WRIT = (1 << 0),
    VMM_PERM_EXEC = (1 << 0),
    VMM_PERM_KERN = (0 << 1),
    VMM_PERM_USER = (1 << 1),
};

/* map `amt` physical pages starting from `paddr` to `amt` virtual pages starting at `vaddr`  */
void vmm_map_phys_pages(phys_addr_t paddr, size_t amt, virt_addr_t vaddr, enum vmm_perms perms);

typedef struct proc *procp_t;

typedef struct x86_stack_frame registers_t;
typedef struct {
  uint64_t rsp, rbp;
  registers_t regs;
  /* uint8_t fpu[512]; */
} save_frame_t;

void init_proc(procp_t p, size_t len, void *buf);
procp_t new_proc();
procp_t get_proc(size_t idx);

#endif
