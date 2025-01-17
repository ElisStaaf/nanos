#ifndef NANOS_COMMON_H_
#define NANOS_COMMON_H_

#include <stdint.h>
#include <stddef.h>

#define NANOS_VERSION "0.1"
#define NANOS_CONTRIBS "*"

typedef ptrdiff_t ssize_t;
typedef size_t usize_t;
typedef void *addr_t;
#define forever for(;;)

void panic(const char *msg, ...);

#define likely(x) (__builtin_expect((x), 1))
#define unlikely(x) (__builtin_expect((x), 0))

#endif
