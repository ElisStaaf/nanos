#include "common.h"
#include "lib/usersp.h"
#include "lib/kernel.h"

int main() {
    struct kernel_functions funcs;
    sys_funcs(&funcs);

#ifdef NANOS_PLATFORM_X86_64
    const char *arch = "x86_64";
#else
    const char *arch = "(unknown)";
#endif
    funcs.putfn(funcs.DL0, "nanos %s (%s) nanos-usersp 0.0.2a: Tue, Aug 7 13:22:30 MSK 2022; %s",
        NANOS_VERSION, NANOS_CONTRIBS, arch);
    return 0;
}


