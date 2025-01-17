#include "asm.h"

#ifdef NANOS_X86_64_OPT_ASM
#define NMD_ASSEMBLY_IMPLEMENTATION
#include "nmd_assembly.h"

void opt_printdisasm(void *addr, void *buf, size_t len, size_t count) {
    static nmd_x86_instruction *instr = NULL; /* TODO: better memory management */
    if(instr == NULL) instr = pmm_alloc_pages(1);
    int first = 1;
    
#ifdef NANOS_PLATFORM_X86_64
    while(len != 0 && count != 0) {
        if(!nmd_x86_decode(buf, len, instr, NMD_X86_MODE_64, NMD_X86_DECODER_FLAGS_MINIMAL)) {
            if(!first) putfn(DL0, "%p (bad)", addr);
            len -= 1;
            count -= 1;
            addr += 1;
            buf += 1;
            continue;
        }

        first = 0;
        size_t insl = nmd_x86_ldisasm(buf, len, NMD_X86_MODE_64);
        if(insl >= len) break;
        len -= insl;
        addr += insl;
        buf += insl;
        count -= 1;

        char str[128];
        nmd_x86_format(instr, str, (uint64_t)addr, NMD_X86_FORMAT_FLAGS_DEFAULT);

        putfn(DL0, "%p %s", addr, str);
    }
#else
#error unknown platform
#endif
}

#else

#endif
