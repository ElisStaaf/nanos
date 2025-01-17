#include "nanos.h"

void setup_intrps() {
  extern void x86_setup_idt();
  x86_setup_idt();

  puts(DL0, "testing interrupts... ");
  extern int x86_interrupt_test();
  int res = x86_interrupt_test();
  if(!res) panic("interrupt test failed!");
  else putsn(DL0, "done");

}
