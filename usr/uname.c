#include "common.h"
#include "lib/usersp.h"
#include "lib/kernel.h"

int main() {
    struct kernel_functions funcs;
    sys_funcs(&funcs);

#ifdef NANOS_PLATFORM_X86_64
    const char *arch = "x86_64";
#else
    const char *arch = "unknown";
#endif
    funcs.putfn(funcs.DL0, "nanos %s (%s) nanos-usersp 0.1: Fri, Jan 17 13:02:04 2025; %s",
        NANOS_VERSION, NANOS_CONTRIBS, arch);
    return 0;
}


