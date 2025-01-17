/* usersp library */
#include <stddef.h>
#include <stdint.h>
#include <x86intrin.h>

void *memset(void *ptr, int val, size_t num) {
   unsigned char *ptr_byte = (unsigned char*)ptr;
   for(size_t i = 0; i < num; ptr_byte[i] = (unsigned char)val, i++);
   return ptr;
}
