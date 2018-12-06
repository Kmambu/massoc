#include <stdio.h>
#include <system.h>

#include <soclib/icu.h>
#include <soclib/aes.h>

#include "../segmentation.h"

// encrypted message
unsigned char encrypted_msg[] = {
  0x4b, 0xb6, 0xaf, 0x42, 0x44, 0xb, 0x96, 0x96, 0x81, 0x72, 0x7e, 0x30,
  0xba, 0x2e, 0xf2, 0xf8, 0x92, 0x7f, 0x9a, 0x29, 0x1, 0x3f, 0x53, 0x2d,
  0x60, 0x49, 0xe3, 0x4c, 0x3, 0xa3, 0x28, 0xdd
};

// make room for clear message
unsigned char clear_msg[256];

// software flag for synchronization between the irq handler and the main
// function
volatile int aes_done = 0;

void irq_handler(int irq)
{
  // TME: reset the IRQ of the AES
  // ...

  // AES is done, warn the main
  aes_done = 1;
}

int main(void)
{
  printf("CPU0 says hello !\n");

  // setup interrupt handler.
  set_irq_handler(irq_handler);

  // setup the ICU and enable AES interrupt.
  soclib_io_set(ICU_BASE, ICU_MASK_SET, 1);

  // enable interrupts of the MIPS (irq of the ICU, and irqs in general)
  enable_hw_irq(0);
  irq_enable();

  // setup data for AES
  int i;
  volatile unsigned char *aes_data = AES_DATA_BASE;
  for (i = 0; i < sizeof(encrypted_msg); i++)
    aes_data[i] = encrypted_msg[i];

  // TME: configure AES with the size of data
  // ...

  // TME: run AES accelerator
  // ...

  // TME: wait until the AES is done decrypting (by checking the flag)
  // ...

  // get the decrypted message into clear_msg
  // (don't forget to set the last byte of clear_msg to 0)
  for (i = 0; i < sizeof(encrypted_msg); i++)
    clear_msg[i] = aes_data[i];
  clear_msg[i] = 0;

  // print clear_msg
  printf("%s\n", clear_msg);

  while (1) /* wait forever doing nothing */ ;
  return 0;
}
