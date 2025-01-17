// PIC Interface

#include "arch.h"

#define PIC1 0x20            /* IO base address for master PIC */
#define PIC2 0xA0            /* IO base address for slave PIC */
#define PIC1_CMD (PIC1 + 0)
#define PIC1_DATA (PIC1 + 1)
#define PIC2_CMD (PIC2 + 0)
#define PIC2_DATA (PIC2 + 1)
#define ICW1_ICW4 0x01		   /* ICW4 (not) needed */
#define ICW1_SINGLE 0x02		 /* Single (cascade) mode */
#define ICW1_INTERVAL4 0x04  /* Call address interval 4 (8) */
#define ICW1_LEVEL 0x08		   /* Level triggered (edge) mode */
#define ICW1_INIT 0x10		   /* Initialization - required! */

#define ICW4_8086 0x01       /* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO 0x02       /* Auto (normal) EOI */
#define ICW4_BUF_SLAVE 0x08  /* Buffered mode/slave */
#define ICW4_BUF_MASTER 0x0C /* Buffered mode/master */
#define ICW4_SFNM 0x10		   /* Special fully nested (not) */

static inline void io_wait() {}

void x86_pic_remap(uint32_t offset1, uint32_t offset2)
{
  uint8_t mask1 = inb(PIC1_DATA);
  uint8_t mask2 = inb(PIC2_DATA);

  outb(PIC1_CMD, ICW1_INIT | ICW1_ICW4);
  io_wait();
  outb(PIC2_CMD, ICW1_INIT | ICW1_ICW4);
  io_wait();
  outb(PIC1_DATA, offset1);
  io_wait();
  outb(PIC2_DATA, offset2);
  io_wait();
  outb(PIC1_DATA, 4);
  io_wait();
  outb(PIC2_DATA, 2);
  io_wait();

  outb(PIC1_DATA, ICW4_8086);
  io_wait();
  outb(PIC2_DATA, ICW4_8086);
  io_wait();

  outb(PIC1_DATA, mask1);
  outb(PIC2_DATA, mask2);
}
