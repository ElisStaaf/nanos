/* monos userspace header */
#ifndef __NANOS_USERSPLIB_H
#include <stddef.h>
#include <stdint.h>

void *memset(void *dst, int v, size_t len);
void sys_funcs(void *funcs);

#endif
