#ifndef __NANOS_USERSPLIB_KERNEL_H
#define __NANOS_USERSPLIB_KERNEL_H
#include "usersp.h"

typedef struct dev dev_t;
typedef dev_t *devp_t;

typedef struct screen {
    uint32_t *buf;
    size_t w, h;
    size_t pitch;
} screen_t;

struct kernel_functions {
    screen_t *const (*scrdev_getscr)(devp_t);
    devp_t DSCR, DL0;
    void (*putf)(devp_t d, const char *fmt, ...);
    void (*putfn)(devp_t d, const char *fmt, ...);
    void (*vputf)(devp_t d, const char *fmt, ...);
    void (*puts)(devp_t d, const char *s);
    void (*putc)(devp_t d, char c);
};  


#endif
