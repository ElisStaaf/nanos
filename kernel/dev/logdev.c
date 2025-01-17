#include "../nanos.h"

#define INDENT_SIZE 2

struct logdev_tag {
    dev_t dev, *tgt;
    int feats, ind, nl;
};

static inline void log_common(devp_t dev) {
    if(((logdev_t*)dev)->feats & LOGDEV_ENABLE_INDENT)
        for(int i = 0; i < ((logdev_t*)dev)->ind * INDENT_SIZE; ++i)
            putc(dev, ' ');
}

static inline void log_ind(devp_t dev, int d) {
    if(((logdev_t*)dev)->feats & LOGDEV_ENABLE_INDENT)
        ((logdev_t*)dev)->ind += d;
}

static inline void log_color(devp_t d, int c) {
    if(((logdev_t*)d)->feats & LOGDEV_ENABLE_COLOR)
        switch(c) {
        case 0: puts(d, "\033[m"); break;
        case 1: puts(d, "\033[31m"); break;
        case 2: puts(d, "\033[32m"); break;
        case 3: puts(d, "\033[33m"); break;
        case 4: puts(d, "\033[34m"); break;
        case 5: puts(d, "\033[35m"); break;
        default: break;
        }
}

void logdev_write(devp_t dev, void *x) {
    if(((logdev_t*)dev)->nl) {
        ((logdev_t*)dev)->nl = 0;
        log_common(dev);
    }
    ((logdev_t*)dev)->tgt->write(((logdev_t*)dev)->tgt, x);
    if((char)(intptr_t)x == '\n') ((logdev_t*)dev)->nl = 1;
}

void *logdev_read(devp_t dev) {
    panic("logdev doesn't support reading");
    return NULL;
}

void logdev_init(logdev_t *dev, devp_t tgt, enum logdev_feat feats) {
    dev->dev.p = &dev->dev;
    dev->dev.write = &logdev_write;
    dev->dev.read = &logdev_read;
    dev->feats = feats;
    dev->tgt = tgt;
    dev->ind = 0;
}

void logdev_beg(devp_t d, const char *f, ...) {
    log_color(d, 2);
    log_ind(d, +1);
    va_list va;
    va_start(va, f);
    putfv(d, f, va);
    va_end(va);
    putc(d, '\n');
    log_color(d, 0);
}

void logdev_end(devp_t d, const char *f, ...) {
    log_color(d, 2);
    log_ind(d, -1);
    putc(d, '\r');
    log_common(d);
    va_list va;
    va_start(va, f);
    putfv(d, f, va);
    va_end(va);
    if(f) putc(d, '\n');
    log_color(d, 0);
}

static logdev_t logdev1, logdev2;
devp_t DL0, DL1;
void setup_syslog() {
    logdev_init(&logdev1, DS0, LOGDEV_ENABLE_INDENT | LOGDEV_ENABLE_COLOR);
    logdev_init(&logdev2, DS0, LOGDEV_ENABLE_INDENT | LOGDEV_ENABLE_COLOR);
    DL0 = &logdev1.dev;
    DL1 = &logdev2.dev;
}

void logdev_set_tgt(devp_t d, devp_t tgt) {
    // void *old = ((logdev_t*)d)->tgt;
    ((logdev_t*)d)->tgt = tgt;
    // ((logdev_t*)d)->tgt = old;
}
