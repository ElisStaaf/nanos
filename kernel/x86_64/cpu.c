// Generic Platform-Specific things

#include "common.h"
#include "../limine.h"
#include "cpu.h"

void panic(const char *msg, ...) {
  puts(DL1, "\033[31mPANIC: ");
  va_list va;
  va_start(va, msg);
  putfv(DL1, msg, va);
  va_end(va);
  putsn(DL1, "\033[m");
  halt();
}

int x86_get_scr(screen_t *scr) {
  struct limine_framebuffer_response *res = limine_framebuffer_req.response;
  if(res == NULL) return 1;
  if(res->framebuffer_count == 0) return 2;
  scr->buf = res->framebuffers[0]->address;
  scr->w = res->framebuffers[0]->width;
  scr->h = res->framebuffers[0]->height;
  scr->pitch = res->framebuffers[0]->pitch;
  return 0;
}

struct limine_framebuffer_request limine_framebuffer_req = {
  .id = LIMINE_FRAMEBUFFER_REQUEST,
  .revision = 0
};

struct limine_kernel_address_request limine_kernel_addr_req = {
  .id = LIMINE_KERNEL_ADDRESS_REQUEST,
  .revision = 0
};

struct limine_hhdm_request limine_hhdm_req = {
  .id = LIMINE_HHDM_REQUEST,
  .revision = 0
};

