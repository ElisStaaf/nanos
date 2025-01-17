#include "../nanos.h"

void muldev_write(devp_t dev, void *x) {
    for(uint8_t i = 0; i < ((muldev_t*)dev)->amt; ++i)
        ((muldev_t*)dev)->ds[i]->write(((muldev_t*)dev)->ds[i], x);
}

void *muldev_read(devp_t dev) {
    panic("muldev doesn't support reading");
    return NULL;
}

void muldev_init(muldev_t *dev, uint8_t amt, devp_t *ds) {
    if(amt > MULDEV_LIM)
        panic("muldev only supports up to 4 devs for now. %d > %d", amt, MULDEV_LIM);
    dev->dev.p = &dev->dev;
    dev->dev.read = &muldev_read;
    dev->dev.write = &muldev_write;
    dev->amt = amt;
    for(uint8_t i = 0; i < amt; ++i) dev->ds[i] = ds[i];
}
