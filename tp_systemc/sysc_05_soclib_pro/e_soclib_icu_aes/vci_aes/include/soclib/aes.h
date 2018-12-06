#ifndef AES_REGS_H
#define AES_REGS_H

enum SoclibAesRegisters {
  SOCLIB_AES_MODE     = 0,
  SOCLIB_AES_SIZE     = 1,
  SOCLIB_AES_RESETIRQ = 2,
};

enum SoclibAesMode {
  SOCLIB_AES_RUNNING = 1,
};

#endif
