#include "../nanos.h"

static void init_com(int port) {
    outb(port + 1, 0x00);
    outb(port + 3, 0x80);
    outb(port + 0, 0x03);
    outb(port + 1, 0x00);
    outb(port + 3, 0x03);
    outb(port + 2, 0xC7);
    outb(port + 4, 0x0B);
    outb(port + 4, 0x1E);
    outb(port + 0, 0xAE);

    if(inb(port + 0) != 0xAE) return;

    outb(port + 4, 0x0F);
}

enum line_status {
    LINE_STATUS_DR = 1 << 0,
    LINE_STATUS_OE = 1 << 1,
    LINE_STATUS_PE = 1 << 2,
    LINE_STATUS_FE = 1 << 3,
    LINE_STATUS_BI = 1 << 4,
    LINE_STATUS_TR = 1 << 5,
    LINE_STATUS_TE = 1 << 6,
    LINE_STATUS_ER = 1 << 7,
};

int serial_line_status_reg(uint16_t port, enum line_status s) {
    return inb(port + 5) & s;
}

char serial_read(uint16_t port) {
   while(!serial_line_status_reg(port, LINE_STATUS_DR));
   return inb(port);
}

void serial_write(uint16_t port, char v) {
    while(!serial_line_status_reg(port, LINE_STATUS_TE));
    outb(port, v);
}

struct serdev_tag {
    dev_t dev;
    uint16_t port;
};

static void *serdev_read(dev_t *d) {
    return (void*)(intptr_t)serial_read(((serdev_t*)d)->port);
}

static void serdev_write(dev_t *d, void *x) {
    serial_write(((serdev_t*)d)->port, (char)(intptr_t)x);
}

void serdev_init(serdev_t *dev, uint16_t port) {
    dev->dev.p = &dev->dev;
    dev->port = port;
    init_com(dev->port);
    dev->dev.read = &serdev_read;
    dev->dev.write = &serdev_write;
}

static serdev_t serdev1;
devp_t DS0, DS1;
void setup_serial() {
    serdev_init(&serdev1, 0x3F8);
    // serdev_init(&serdev1, 0x2F8);
    DS0 = &serdev1.dev;
    // DS1 = &serdev2.dev;
}
