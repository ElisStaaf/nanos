#include "../nanos.h"

#define TAB_SZ 8

struct scrdev_tag {
    dev_t dev;
    screen_t *scr;
    uint8_t x, y;
    uint32_t fg, bg;
    int esc, escidx;
    char escstr[4];
};

#ifdef SCRDEV_DOUBLE
#define SCRDEV_SCALE 2
#else
#define SCRDEV_SCALE 1
#endif

#define VMUL 2
void scrdev_out(scrdev_t *dev, char c) {
    extern unsigned char scrfont0[128][8];
    unsigned char *ltr = scrfont0[(size_t)c];
    uint32_t off = SCRDEV_PAD_TOP * dev->scr->w + SCRDEV_PAD_LEFT
        + (dev->x * 8) + ((dev->y * 8*VMUL + 2) * dev->scr->w);
    size_t sz = dev->scr->w * dev->scr->h;
    for(int8_t i = 0; i < 8; ++i) {
        unsigned char row = ltr[i];
        for(int8_t j = 0; j < 8; ++j) {
            size_t addr = off + i * VMUL * dev->scr->w + j;
            if(addr >= sz) continue;
            uint32_t c = ((row >> j) & 1) ? dev->fg : dev->bg;
            for(int i = 0; i < VMUL; ++i)
           		dev->scr->buf[addr + i * dev->scr->w] = c;
        }
    }
}

static int cmp_str(char *a, char *b, int lena, int lenb) {
    lena = lena < lenb ? lena : lenb;
    int s = 0;
    for(int i = 0; i < lena; ++i)
        s += a[i] - b[i];
    return s;
}

static void handle_esc_color(scrdev_t *dev, char *str, int len) {
    if(len == 0) {
        dev->fg = 0xc6cfd5;
        dev->bg = 0x050912;
    }

    else if(cmp_str(str, "31", len, 2) == 0) dev->fg = 0xdfa6a6;
    else if(cmp_str(str, "32", len, 2) == 0) dev->fg = 0xa6dfa3;
    else if(cmp_str(str, "33", len, 2) == 0) dev->fg = 0xdfd398;
    else if(cmp_str(str, "34", len, 2) == 0) dev->fg = 0x98a2d7;
    else if(cmp_str(str, "35", len, 2) == 0) dev->fg = 0xd799df;
    else if(cmp_str(str, "36", len, 2) == 0) dev->fg = 0x97e2d6;
}

void scrdev_put(scrdev_t *dev, char c) {
    switch(dev->esc) {
    case 0:
        switch(c) {
        case '\n': ++dev->y; /* fallthrough */
        case '\r': dev->x = 0; break;
        case '\t': dev->x = (dev->x + TAB_SZ - 1) & -TAB_SZ; break;
        case '\033': dev->esc = 1; break;
        default: if(c < 32) break; scrdev_out(dev, c); ++dev->x; break;
        } break;
    case 1:
        switch(c) {
        case '[': dev->esc = 2; dev->escidx = 0; break;
        default: dev->esc = 0; break;
        } break;
    case 2:
        switch(c) {
        case 'm': handle_esc_color(dev, dev->escstr, dev->escidx); dev->esc = 0; break;
        default: dev->escstr[dev->escidx++] = c; break;
        } break;
    default: dev->esc = 0; break;
    }
}

void scrdev_write(devp_t dev, void *x) {
    scrdev_put((scrdev_t*)dev, (intptr_t)x);
}

void *scrdev_read(devp_t dev) {
    panic("scrdev doesn't support reading");
    return NULL;
}

void scrdev_init(scrdev_t *dev, screen_t *scr) {
    dev->dev.p = &dev->dev;
    dev->dev.write = &scrdev_write;
    dev->dev.read = &scrdev_read;
    dev->scr = scr;
    dev->x = dev->y = 0;
    dev->fg = 0xFFFFFF;
    dev->bg = 0x000000;
    dev->esc = 0;
    dev->escidx = 0;
    for(uint32_t i = 0; i < scr->w * scr->h; ++i)
    	scr->buf[i] = 0x050912;
}

screen_t *const scrdev_getscr(scrdev_t *dev) {
    return dev->scr;
}

static scrdev_t scrdev;
static screen_t scr;
devp_t DSCR = NULL;

void setup_scrlog() {
    logdev_beg(DL0, "setting up scrlog:");
    extern int x86_get_scr(screen_t *scr);
    if(x86_get_scr(&scr) != 0) {
        putsn(DL0, "no screen!");
        return;
    }

    putfn(DL0, "screen at 0x%p %lux%lu, %lu", scr.buf, scr.w, scr.h, scr.pitch);

    scrdev_init(&scrdev, &scr);
    DSCR = &scrdev.dev;

    logdev_end(DL0, "done");
}
