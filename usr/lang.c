#include "lib/usersp.h"
#include "lib/kernel.h"
#include "lib/bool.h"
#include "lib/math.h"
#include "lib/rng.h"

struct msg {
    size_t tgt; /* target address */
    size_t ret; /* return address */
    size_t msz; /* memory size */
    void  *mem; /* memory pointer */
};

struct ctl {
    size_t amt; /* message count */
    struct msg *msg; /* messages */
};

size_t counter = 0;
size_t magic = 10;
void msg(struct ctl *ctl) {
    for(size_t i = 0; i < ctl->amt; ++i) {
        if(ctl->msg[0].msz == 0) {
            if((uintptr_t)ctl->msg[0].mem == 0x00) // increment
                counter += 1;
            if((uintptr_t)ctl->msg[0].mem == 0x01) // decrement
                counter -= 1;
        }
    }
    if(counter == magic) {
        ctl->amt = 1;
        ctl->msg[0].msz = 0;
        ctl->msg[0].mem = 0;
    }
}

int main() {
    return 0;
}
