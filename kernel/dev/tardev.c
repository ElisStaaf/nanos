#include "../nanos.h"

struct tardev_hdr {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag[1];
};

size_t getvalue(size_t lim, const char *x) {
    size_t size = 0;
    size_t count = 1;
    for(size_t j = lim; j > 0; j--, count *= 8)
        size += ((x[j - 1] - '0') * count);
    return size;
}

static size_t parse(size_t max, struct tardev_hdr **hdrs, uintptr_t address) {
    logdev_beg(DL0, "parsing tar");
    for(size_t i = 0; i < max; ++i) {
        struct tardev_hdr *header = (struct tardev_hdr *)address;
        if (header->name[0] == '\0') {
            logdev_end(DL0, "parsed %lu", i);
            return i;
        }
 
        size_t size = getvalue(11, header->size);
        putfn(DL0, "file: %s, size: %lu", header->name, size);
        hdrs[i] = header;
        address += ((size / 512) + 1) * 512;
        if(size % 512) address += 512;
    }
    logdev_end(DL0, "parsed %lu", max - 1);
    return max - 1;
}

static void *tardev_read(dev_t *d) {
    panic("tardev doesn't support reading");
    return NULL;
}

static void tardev_write(dev_t *d, void *x) {
    panic("tardev doesn't support writing");
}

struct tardev_tag {
    dev_t dev;
    size_t amt;
    struct tardev_hdr *hdrs[16];
};

void tardev_init(tardev_t *dev, void *data) {
    dev->dev.p = &dev->dev;
    dev->dev.read = &tardev_read;
    dev->dev.write = &tardev_write;
    dev->amt = parse(16, dev->hdrs, (uintptr_t)data);
}

size_t tardev_count(devp_t d) { return ((tardev_t*)d)->amt; }

static size_t strcmp(const char *a, const char *b) {
    size_t d = 0;
    do d += *b++ - *a++; while(*a && *b);
    return d + (*a != *b); // tmp
}

void tardev_get(devp_t d, const char *filename, struct tardev_file *file) {
    if(file == NULL) return;
    
    for(size_t i = 0; i < ((tardev_t*)d)->amt; ++i) {
        struct tardev_hdr *hdr = ((tardev_t*)d)->hdrs[i];
        if(strcmp(hdr->name, filename) == 0) {
            file->name = hdr->name;
            file->mode = getvalue(7, hdr->mode);
            file->size = getvalue(11, hdr->size);
            file->data = (uint8_t*)hdr + 512;
            return;
        }
    }
    file->name = NULL;
    file->data = NULL;
    file->mode = 0;
    file->size = 0;
}

tardev_t initdev;
devp_t DINIT;
void setup_tardev(void *data) {
    tardev_init(&initdev, data);
    DINIT = &initdev.dev;
}
