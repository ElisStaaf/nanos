#ifndef __NANOS_USERSPLIB_RNG_H
#define __NANOS_USERSPLIB_RNG_H
#include "usersp.h"
/* set rng seed */ void srng(uint64_t);
/* get p-random */ uint64_t grng(void);
/* get p-random double-prec float */ double grngdpf(void);
/* get p-random single-prec float */ float grngspf(void);
/* get rng maximum */ uint64_t rngmax(void);
#endif
