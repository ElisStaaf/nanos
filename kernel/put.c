#include "nanos.h"

void putc(devp_t d, char c) {
    d->write(d, (void*)(intptr_t)c);
}

void puts(devp_t d, const char *s) {
    if(!s) puts(d, "(null)");
    else while(*s) putc(d, *s++);
}

void putsn(devp_t d, const char *s) {
    if(!s) putsn(d, "(null)");
    else while(*s) putc(d, *s++);
    putc(d, '\n');
}

static inline char dchupr(int digit) {
    return digit < 10 ? digit + '0' : digit - 10 + 'A';
}

static inline char dchlwr(int digit) {
    return digit < 10 ? digit + '0' : digit - 10 + 'a';
}

void putsua(devp_t d, usize_t v, char base) {
    if(v >= base) putsua(d, v / base, base);
    putc(d, dchupr(v % base));
}

void putsia(devp_t d, ssize_t v, char base) {
    if(v < 0) {
        putc(d, '-');
        v = -v;
    }
    putsua(d, v, base);
}

void putsud(devp_t d, usize_t v) { putsua(d, v, 10); }
void putsid(devp_t d, ssize_t v) { putsia(d, v, 10); }
void putsuh(devp_t d, usize_t v) { putsua(d, v, 16); }
void putsih(devp_t d, ssize_t v) { putsia(d, v, 16); }
void putsub(devp_t d, usize_t v) { putsua(d, v, 2); }
void putsib(devp_t d, ssize_t v) { putsia(d, v, 2); }
void putsuo(devp_t d, usize_t v) { putsua(d, v, 8); }
void putsio(devp_t d, ssize_t v) { putsia(d, v, 8); }

void putp(devp_t d, void *a) {
    for(int i = 0; i < clzll((size_t)a) / 4; ++i)
        putc(d, '0');
    putsuh(d, (size_t)a);
}

void putd(devp_t d, double f) {
    long long ipart = (long long)f;
    long long fpart = (long long)((f - ipart) * 1000000.0f);
    putsid(d, ipart);
    putc(d, '.');
    putsid(d, fpart);
}

void putf(devp_t d, const char *f, ...) {
    va_list va;
    va_start(va, f);
    putfv(d, f, va);
    va_end(va);
}

void putfn(devp_t d, const char *f, ...) {
    va_list va;
    va_start(va, f);
    putfv(d, f, va);
    va_end(va);
    putc(d, '\n');
}

void putfv(devp_t d, const char *f, va_list v) {
    if(!f) return;
    while(*f) {
        if(*f != '%') {
            putc(d, *f++);
            continue;
        }
        f += 1; // skip '%'
        switch(*f++) {
        case 'l':
            switch(*f++) {
            case 'd': putsid(d, va_arg(v, signed long)); break;
            case 'x': putsuh(d, va_arg(v, unsigned long)); break;
            case 'u': putsud(d, va_arg(v, unsigned long)); break;
            default: break;
            }
            break;
        case 'd': putsid(d, va_arg(v, signed int)); break;
        case 'x': putsuh(d, va_arg(v, unsigned int)); break;
        case 'u': putsud(d, va_arg(v, unsigned int)); break;
        case 'p': putp(d, va_arg(v, void*)); break;
        case 's': puts(d, va_arg(v, const char *)); break;
        case 'c': putc(d, va_arg(v, int)); break;
        case 'f': putd(d, va_arg(v, double)); break;
        default: break;
        }
    }
}
