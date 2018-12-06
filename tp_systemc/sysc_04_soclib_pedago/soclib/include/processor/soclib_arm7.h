// -*- c++ -*-
//////////////////////////////////////////////////////////////////////////////////
// File: soclib_arm7.h
// Authors: François Pêcheux 
// Date : 23/02/2004
// This program is released under the GNU Public License.
// Copyright : UPMC-LIP6
// 
// This file is based on the source code found in 
// VisualBoyAdvance-1.7.1-SDL-linux-glibc22.tar.gz
// VisualBoyAdvance-src-1.7.1.tar.gz
// It just has been "Soclibified", i.e. transformed into
// 3 SC_METHODs :transition, genMoore and genMealy
//////////////////////////////////////////////////////////////////////////////////

#ifndef SOCLIB_ARM7_H
#define SOCLIB_ARM7_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <systemc.h>
#include "shared/soclib_mapping_table.h"
#include "cache/soclib_xcache_interface.h"

///////////////////////////////////////////////////////
//      Utility types
///////////////////////////////////////////////////////

typedef union
{
  struct
  {
#ifdef WORDS_BIGENDIAN
    unsigned char B3;
    unsigned char B2;
    unsigned char B1;
    unsigned char B0;
#else
    unsigned char B0;
    unsigned char B1;
    unsigned char B2;
    unsigned char B3;
#endif
  } B;

  struct
  {
#ifdef WORDS_BIGENDIAN
    unsigned short W1;
    unsigned short W0;
#else
    unsigned short W0;
    unsigned short W1;
#endif
  } W;

  unsigned int I;
} reg_pair;

enum {
	COMPUTEADDRESS=0,
	REGISTER=1,
	END=2
     };

///////////////////////////////////////////////////////
//      Macros that speed up things a little bit        
///////////////////////////////////////////////////////

// define alias for the different modes
#define USER_MODE 0x10
#define SYSTEM_MODE 0x1F
#define SUPERVISOR_MODE 0x13
#define ABORT_MODE 0x17
#define UNDEFINED_MODE 0x1B
#define IRQ_MODE 0x12
#define FIQ_MODE 0x11

#define ARM_RESET_VECTOR          0x00000000
#define ARM_UNDEF_INSTR_VECTOR    0X00000004
#define ARM_SWI_VECTOR            0x00000008
#define ARM_PREFETCH_ABORT_VECTOR 0x0000000c
#define ARM_DATA_ABORT_VECTOR     0x00000010
#define ARM_IRQ_VECTOR            0x00000018
#define ARM_FIQ_VECTOR            0x0000001c


#define ARM_NOP_INST 0xE1A00000
#define ARM_THUMB_NOP_INST 0x1C001C00

#define R13_IRQ  18
#define R14_IRQ  19
#define SPSR_IRQ 20
#define OFFSET_IRQ (R13_IRQ - 13)

#define R13_USR  26
#define R14_USR  27
#define OFFSET_USR (R13_USR - 13)

#define R13_SVC  28
#define R14_SVC  29
#define SPSR_SVC 30
#define OFFSET_SVC (R13_SVC - 13)

#define R13_ABT  31
#define R14_ABT  32
#define SPSR_ABT 33
#define OFFSET_ABT (R13_ABT - 13)

#define R13_UND  34
#define R14_UND  35
#define SPSR_UND 36
#define OFFSET_UND (R13_UND - 13)

#define R8_FIQ   37
#define R9_FIQ   38
#define R10_FIQ  39
#define R11_FIQ  40
#define R12_FIQ  41
#define R13_FIQ  42
#define R14_FIQ  43
#define SPSR_FIQ 44
#define OFFSET_FIQ (R8_FIQ - 8)

#define RESET_INT 0
#define DATA_ABORT_INT 1
#define FIQ_INT 2
#define IRQ_INT 3
#define PREFETCH_ABORT_INT 4
#define UNDEF_INSTR_INT 5
#define SWI_INT 6

#define ARM_OP_AND \
      reg[dest].I = reg[(opcode>>16) & 15].I & value;

#define ARM_OP_ANDS \
      reg[dest].I = reg[(opcode>>16) & 15].I & value;\
      \
      N_FLAG = (reg[dest].I & 0x80000000) ? true : false;\
      Z_FLAG = (reg[dest].I) ? false : true;\
      C_FLAG = C_OUT;\
      CPUUpdateCPSR();

#define ARM_OP_EOR \
      reg[dest].I = reg[(opcode>>16)&15].I ^ value;

#define ARM_OP_EORS \
      reg[dest].I = reg[(opcode>>16)&15].I ^ value;\
      \
      N_FLAG = (reg[dest].I & 0x80000000) ? true : false;\
      Z_FLAG = (reg[dest].I) ? false : true;\
      C_FLAG = C_OUT;\
      CPUUpdateCPSR();

#define NEG(i) ((i) >> 31)
#define POS(i) ((~(i)) >> 31)
#define ADDCARRY(a, b, c) \
  C_FLAG = ((NEG(a) & NEG(b)) |\
            (NEG(a) & POS(c)) |\
            (NEG(b) & POS(c))) ? true : false;
#define ADDOVERFLOW(a, b, c) \
  V_FLAG = ((NEG(a) & NEG(b) & POS(c)) |\
            (POS(a) & POS(b) & NEG(c))) ? true : false;
#define SUBCARRY(a, b, c) \
  C_FLAG = ((NEG(a) & POS(b)) |\
            (NEG(a) & POS(c)) |\
            (POS(b) & POS(c))) ? true : false;
#define SUBOVERFLOW(a, b, c)\
  V_FLAG = ((NEG(a) & POS(b) & POS(c)) |\
            (POS(a) & NEG(b) & NEG(c))) ? true : false;
#define ARM_OP_SUB \
    {\
      int base = (opcode >> 16) & 0x0F;\
      reg[dest].I = reg[base].I - value;\
    }
#define ARM_OP_SUBS \
   {\
     int base = (opcode >> 16) & 0x0F;\
     unsigned int lhs = reg[base].I;\
     unsigned int rhs = value;\
     unsigned int res = lhs - rhs;\
     reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(lhs, rhs, res);\
     SUBOVERFLOW(lhs, rhs, res);\
     CPUUpdateCPSR();\
   }
#define ARM_OP_RSB \
    {\
      int base = (opcode >> 16) & 0x0F;\
      reg[dest].I = value - reg[base].I;\
    }
#define ARM_OP_RSBS \
   {\
     int base = (opcode >> 16) & 0x0F;\
     unsigned int lhs = reg[base].I;\
     unsigned int rhs = value;\
     unsigned int res = rhs - lhs;\
     reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(rhs, lhs, res);\
     SUBOVERFLOW(rhs, lhs, res);\
     CPUUpdateCPSR();\
   }
#define ARM_OP_ADD \
    {\
      int base = (opcode >> 16) & 0x0F;\
      reg[dest].I = reg[base].I + value;\
    }
#define ARM_OP_ADDS \
   {\
      int base = (opcode >> 16) & 0x0F;\
     unsigned int lhs = reg[base].I;\
     unsigned int rhs = value;\
     unsigned int res = lhs + rhs;\
     reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     ADDCARRY(lhs, rhs, res);\
     ADDOVERFLOW(lhs, rhs, res);\
     CPUUpdateCPSR();\
   }
#define ARM_OP_ADC \
    {\
      int base = (opcode >> 16) & 0x0F;\
      reg[dest].I = reg[base].I + value + (unsigned int)C_FLAG;\
    }
#define ARM_OP_ADCS \
   {\
      int base = (opcode >> 16) & 0x0F;\
     unsigned int lhs = reg[base].I;\
     unsigned int rhs = value;\
     unsigned int res = lhs + rhs + (unsigned int)C_FLAG;\
     reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     ADDCARRY(lhs, rhs, res);\
     ADDOVERFLOW(lhs, rhs, res);\
     CPUUpdateCPSR();\
   }
#define ARM_OP_SBC \
    {\
      int base = (opcode >> 16) & 0x0F;\
      reg[dest].I = reg[base].I - value - !((unsigned int)C_FLAG);\
    }
#define ARM_OP_SBCS \
   {\
      int base = (opcode >> 16) & 0x0F;\
     unsigned int lhs = reg[base].I;\
     unsigned int rhs = value;\
     unsigned int res = lhs - rhs - !((unsigned int)C_FLAG);\
     reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(lhs, rhs, res);\
     SUBOVERFLOW(lhs, rhs, res);\
     CPUUpdateCPSR();\
   }
#define ARM_OP_RSC \
    {\
      int base = (opcode >> 16) & 0x0F;\
      reg[dest].I = value - reg[base].I - !((unsigned int)C_FLAG);\
    }
#define ARM_OP_RSCS \
   {\
      int base = (opcode >> 16) & 0x0F;\
     unsigned int lhs = reg[base].I;\
     unsigned int rhs = value;\
     unsigned int res = rhs - lhs - !((unsigned int)C_FLAG);\
     reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(rhs, lhs, res);\
     SUBOVERFLOW(rhs, lhs, res);\
     CPUUpdateCPSR();\
   }
#define ARM_OP_CMP \
   {\
      int base = (opcode >> 16) & 0x0F;\
     unsigned int lhs = reg[base].I;\
     unsigned int rhs = value;\
     unsigned int res = lhs - rhs;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(lhs, rhs, res);\
     SUBOVERFLOW(lhs, rhs, res);\
     CPUUpdateCPSR();\
   }
#define ARM_OP_CMN \
   {\
      int base = (opcode >> 16) & 0x0F;\
     unsigned int lhs = reg[base].I;\
     unsigned int rhs = value;\
     unsigned int res = lhs + rhs;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     ADDCARRY(lhs, rhs, res);\
     ADDOVERFLOW(lhs, rhs, res);\
     CPUUpdateCPSR();\
   }

#define LOGICAL_LSL_REG \
   {\
     unsigned int v = reg[opcode & 0x0f].I;\
     C_OUT = (v >> (32 - shift)) & 1 ? true : false;\
     value = v << shift;\
   }
#define LOGICAL_LSR_REG \
   {\
     unsigned int v = reg[opcode & 0x0f].I;\
     C_OUT = (v >> (shift - 1)) & 1 ? true : false;\
     value = v >> shift;\
   }
#define LOGICAL_ASR_REG \
   {\
     unsigned int v = reg[opcode & 0x0f].I;\
     C_OUT = ((signed int)v >> (int)(shift - 1)) & 1 ? true : false;\
     value = (signed int)v >> (int)shift;\
   }
#define LOGICAL_ROR_REG \
   {\
     unsigned int v = reg[opcode & 0x0f].I;\
     C_OUT = (v >> (shift - 1)) & 1 ? true : false;\
     value = ((v << (32 - shift)) |\
              (v >> shift));\
   }
#define LOGICAL_RRX_REG \
   {\
     unsigned int v = reg[opcode & 0x0f].I;\
     shift = (int)C_FLAG;\
     C_OUT = (v  & 1) ? true : false;\
     value = ((v >> 1) |\
              (shift << 31));\
   }
#define LOGICAL_ROR_IMM \
   {\
     unsigned int v = opcode & 0xff;\
     C_OUT = (v >> (shift - 1)) & 1 ? true : false;\
     value = ((v << (32 - shift)) |\
              (v >> shift));\
   }
#define ARITHMETIC_LSL_REG \
   {\
     unsigned int v = reg[opcode & 0x0f].I;\
     value = v << shift;\
   }
#define ARITHMETIC_LSR_REG \
   {\
     unsigned int v = reg[opcode & 0x0f].I;\
     value = v >> shift;\
   }
#define ARITHMETIC_ASR_REG \
   {\
     unsigned int v = reg[opcode & 0x0f].I;\
     value = (signed int)v >> (int)shift;\
   }
#define ARITHMETIC_ROR_REG \
   {\
     unsigned int v = reg[opcode & 0x0f].I;\
     value = ((v << (32 - shift)) |\
              (v >> shift));\
   }
#define ARITHMETIC_RRX_REG \
   {\
     unsigned int v = reg[opcode & 0x0f].I;\
     shift = (int)C_FLAG;\
     value = ((v >> 1) |\
              (shift << 31));\
   }
#define ARITHMETIC_ROR_IMM \
   {\
     unsigned int v = opcode & 0xff;\
     value = ((v << (32 - shift)) |\
              (v >> shift));\
   }
#define ROR_IMM_MSR \
   {\
     unsigned int v = opcode & 0xff;\
     value = ((v << (32 - shift)) |\
              (v >> shift));\
   }
#define ROR_VALUE \
   {\
     value = ((value << (32 - shift)) |\
              (value >> shift));\
   }
#define RCR_VALUE \
   {\
     shift = (int)C_FLAG;\
     value = ((value >> 1) |\
              (shift << 31));\
   }

#define ARM_OP_TST \
      int base = (opcode >> 16) & 0x0F;\
      unsigned int res = reg[base].I & value;\
      N_FLAG = (res & 0x80000000) ? true : false;\
      Z_FLAG = (res) ? false : true;\
      C_FLAG = C_OUT;\
      CPUUpdateCPSR();

#define ARM_OP_TEQ \
      int base = (opcode >> 16) & 0x0F;\
      unsigned int res = reg[base].I ^ value;\
      N_FLAG = (res & 0x80000000) ? true : false;\
      Z_FLAG = (res) ? false : true;\
      C_FLAG = C_OUT;\
      CPUUpdateCPSR();

#define ARM_OP_ORR \
      int base = (opcode >> 16) & 0x0F;\
    reg[dest].I = reg[base].I | value;

#define ARM_OP_ORRS \
      int base = (opcode >> 16) & 0x0F;\
    reg[dest].I = reg[base].I | value;\
    N_FLAG = (reg[dest].I & 0x80000000) ? true : false;\
    Z_FLAG = (reg[dest].I) ? false : true;\
    C_FLAG = C_OUT;\
    CPUUpdateCPSR();

#define ARM_OP_MOV \
    reg[dest].I = value;\
    if (dest == 15)\
      modified_pc = true;  

#define ARM_OP_MOVS \
    reg[dest].I = value;\
    N_FLAG = (reg[dest].I & 0x80000000) ? true : false;\
    Z_FLAG = (reg[dest].I) ? false : true;\
    C_FLAG = C_OUT;\
    CPUUpdateCPSR();\
    if (dest == 15)\
     modified_pc = true;

#define ARM_OP_BIC \
      int base = (opcode >> 16) & 0x0F;\
    reg[dest].I = reg[base].I & (~value);

#define ARM_OP_BICS \
      int base = (opcode >> 16) & 0x0F;\
    reg[dest].I = reg[base].I & (~value);\
    N_FLAG = (reg[dest].I & 0x80000000) ? true : false;\
    Z_FLAG = (reg[dest].I) ? false : true;\
    C_FLAG = C_OUT;\
    CPUUpdateCPSR();

#define ARM_OP_MVN \
    reg[dest].I = ~value;

#define ARM_OP_MVNS \
    reg[dest].I = ~value; \
    N_FLAG = (reg[dest].I & 0x80000000) ? true : false;\
    Z_FLAG = (reg[dest].I) ? false : true;\
    C_FLAG = C_OUT;\
    CPUUpdateCPSR();

#define CASE_16(BASE) \
  case BASE:\
  case BASE+1:\
  case BASE+2:\
  case BASE+3:\
  case BASE+4:\
  case BASE+5:\
  case BASE+6:\
  case BASE+7:\
  case BASE+8:\
  case BASE+9:\
  case BASE+10:\
  case BASE+11:\
  case BASE+12:\
  case BASE+13:\
  case BASE+14:\
  case BASE+15:

#define CASE_256(BASE) \
  CASE_16(BASE)\
  CASE_16(BASE+0x10)\
  CASE_16(BASE+0x20)\
  CASE_16(BASE+0x30)\
  CASE_16(BASE+0x40)\
  CASE_16(BASE+0x50)\
  CASE_16(BASE+0x60)\
  CASE_16(BASE+0x70)\
  CASE_16(BASE+0x80)\
  CASE_16(BASE+0x90)\
  CASE_16(BASE+0xa0)\
  CASE_16(BASE+0xb0)\
  CASE_16(BASE+0xc0)\
  CASE_16(BASE+0xd0)\
  CASE_16(BASE+0xe0)\
  CASE_16(BASE+0xf0)

#define LOGICAL_DATA_OPCODE(OPCODE, OPCODE2, BASE) \
  case BASE: \
  case BASE+8:\
    {\
      /* OP Rd,Rb,Rm LSL # */ \
      int shift = (opcode >> 7) & 0x1F;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      unsigned int value;\
      \
      if(shift) {\
        LOGICAL_LSL_REG\
      } else {\
        value = reg[opcode & 0x0F].I;\
      }\
      if(dest == 15) {\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
        }\
	branch_taken=true;\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+2:\
  case BASE+10:\
    {\
       /* OP Rd,Rb,Rm LSR # */ \
      int shift = (opcode >> 7) & 0x1F;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      unsigned int value;\
      if(shift) {\
        LOGICAL_LSR_REG\
      } else {\
        value = 0;\
        C_OUT = (reg[opcode & 0x0F].I & 0x80000000) ? true : false;\
      }\
      \
      if(dest == 15) {\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
        }\
	branch_taken=true;\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+4:\
  case BASE+12:\
    {\
       /* OP Rd,Rb,Rm ASR # */\
      int shift = (opcode >> 7) & 0x1F;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      unsigned int value;\
      if(shift) {\
        LOGICAL_ASR_REG\
      } else {\
        if(reg[opcode & 0x0F].I & 0x80000000){\
          value = 0xFFFFFFFF;\
          C_OUT = true;\
        } else {\
          value = 0;\
          C_OUT = false;\
        }                   \
      }\
      \
      if(dest == 15) {\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
        }\
	branch_taken=true;\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+6:\
  case BASE+14:\
    {\
       /* OP Rd,Rb,Rm ROR # */\
      int shift = (opcode >> 7) & 0x1F;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      unsigned int value;\
      if(shift) {\
        LOGICAL_ROR_REG\
      } else {\
        LOGICAL_RRX_REG\
      }\
      if(dest == 15) {\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
        }\
	branch_taken=true;\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+1:\
    {\
       /* OP Rd,Rb,Rm LSL Rs */\
      int shift = reg[(opcode >> 8)&15].B.B0;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      unsigned int value;\
      if(shift) {\
        if(shift == 32) {\
          value = 0;\
          C_OUT = (reg[opcode & 0x0F].I & 1 ? true : false);\
        } else if(shift < 32) {\
           LOGICAL_LSL_REG\
        } else {\
          value = 0;\
          C_OUT = false;\
        }\
      } else {\
        value = reg[opcode & 0x0F].I;\
      }\
      if(dest == 15) {\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
        }\
	branch_taken=true;\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+3:\
    {\
       /* OP Rd,Rb,Rm LSR Rs */ \
      int shift = reg[(opcode >> 8)&15].B.B0;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      unsigned int value;\
      if(shift) {\
        if(shift == 32) {\
          value = 0;\
          C_OUT = (reg[opcode & 0x0F].I & 0x80000000 ? true : false);\
        } else if(shift < 32) {\
            LOGICAL_LSR_REG\
        } else {\
          value = 0;\
          C_OUT = false;\
        }\
      } else {\
        value = reg[opcode & 0x0F].I;\
      }\
      if(dest == 15) {\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
        }\
	branch_taken=true;\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+5:\
    {\
       /* OP Rd,Rb,Rm ASR Rs */ \
      int shift = reg[(opcode >> 8)&15].B.B0;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      unsigned int value;\
      if(shift < 32) {\
        if(shift) {\
          LOGICAL_ASR_REG\
        } else {\
          value = reg[opcode & 0x0F].I;\
        }\
      } else {\
        if(reg[opcode & 0x0F].I & 0x80000000){\
          value = 0xFFFFFFFF;\
          C_OUT = true;\
        } else {\
          value = 0;\
          C_OUT = false;\
        }\
      }\
      if(dest == 15) {\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
        }\
	branch_taken=true;\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+7:\
    {\
       /* OP Rd,Rb,Rm ROR Rs */\
      int shift = reg[(opcode >> 8)&15].B.B0;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      unsigned int value;\
      if(shift) {\
        shift &= 0x1f;\
        if(shift) {\
          LOGICAL_ROR_REG\
        } else {\
          value = reg[opcode & 0x0F].I;\
          C_OUT = (value & 0x80000000 ? true : false);\
        }\
      } else {\
        value = reg[opcode & 0x0F].I;\
        C_OUT = (value & 0x80000000 ? true : false);\
      }\
      if(dest == 15) {\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
        }\
	branch_taken=true;\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+0x200:\
  case BASE+0x201:\
  case BASE+0x202:\
  case BASE+0x203:\
  case BASE+0x204:\
  case BASE+0x205:\
  case BASE+0x206:\
  case BASE+0x207:\
  case BASE+0x208:\
  case BASE+0x209:\
  case BASE+0x20a:\
  case BASE+0x20b:\
  case BASE+0x20c:\
  case BASE+0x20d:\
  case BASE+0x20e:\
  case BASE+0x20f:\
    {\
      int shift = (opcode & 0xF00) >> 7;\
      int dest = (opcode >> 12) & 0x0F;\
      bool C_OUT = C_FLAG;\
      unsigned int value;\
      if(shift) {\
        LOGICAL_ROR_IMM\
      } else {\
        value = opcode & 0xff;\
      }\
      if(dest == 15) {\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
        }\
	branch_taken=true;\
      } else {\
        OPCODE \
      }\
    }\
    break;

#define ARITHMETIC_DATA_OPCODE(OPCODE, OPCODE2, BASE) \
  case BASE:\
  case BASE+8:\
    {\
      /* OP Rd,Rb,Rm LSL # */\
      int shift = (opcode >> 7) & 0x1F;\
      int dest = (opcode>>12) & 15;\
      unsigned int value;\
      if(shift) {\
        ARITHMETIC_LSL_REG\
      } else {\
        value = reg[opcode & 0x0F].I;\
      }\
      if(dest == 15) {\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
        }\
	branch_taken=true;\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+2:\
  case BASE+10:\
    {\
      /* OP Rd,Rb,Rm LSR # */\
      int shift = (opcode >> 7) & 0x1F;\
      int dest = (opcode>>12) & 15;\
      unsigned int value;\
      if(shift) {\
        ARITHMETIC_LSR_REG\
      } else {\
        value = 0;\
      }\
      if(dest == 15) {\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
        }\
	branch_taken=true;\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+4:\
  case BASE+12:\
    {\
      /* OP Rd,Rb,Rm ASR # */\
      int shift = (opcode >> 7) & 0x1F;\
      int dest = (opcode>>12) & 15;\
      unsigned int value;\
      if(shift) {\
        ARITHMETIC_ASR_REG\
      } else {\
        if(reg[opcode & 0x0F].I & 0x80000000){\
          value = 0xFFFFFFFF;\
        } else value = 0;\
      }\
      if(dest == 15) {\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
        }\
	branch_taken=true;\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+6:\
  case BASE+14:\
    {\
      /* OP Rd,Rb,Rm ROR # */\
      int shift = (opcode >> 7) & 0x1F;\
      int dest = (opcode>>12) & 15;\
      unsigned int value;\
      if(shift) {\
         ARITHMETIC_ROR_REG\
      } else {\
         ARITHMETIC_RRX_REG\
      }\
      if(dest == 15) {\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
        }\
	branch_taken=true;\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+1:\
    {\
      /* OP Rd,Rb,Rm LSL Rs */\
      int shift = reg[(opcode >> 8)&15].B.B0;\
      int dest = (opcode>>12) & 15;\
      unsigned int value;\
      if(shift) {\
        if(shift == 32) {\
          value = 0;\
        } else if(shift < 32) {\
           ARITHMETIC_LSL_REG\
        } else value = 0;\
      } else {\
        value = reg[opcode & 0x0F].I;\
      }\
      if(dest == 15) {\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
        }\
	branch_taken=true;\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+3:\
    {\
      /* OP Rd,Rb,Rm LSR Rs */\
      int shift = reg[(opcode >> 8)&15].B.B0;\
      int dest = (opcode>>12) & 15;\
      unsigned int value;\
      if(shift) {\
        if(shift == 32) {\
          value = 0;\
        } else if(shift < 32) {\
           ARITHMETIC_LSR_REG\
        } else value = 0;\
      } else {\
        value = reg[opcode & 0x0F].I;\
      }\
      if(dest == 15) {\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
        }\
	branch_taken=true;\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+5:\
    {\
      /* OP Rd,Rb,Rm ASR Rs */\
      int shift = reg[(opcode >> 8)&15].B.B0;\
      int dest = (opcode>>12) & 15;\
      unsigned int value;\
      if(shift < 32) {\
        if(shift) {\
           ARITHMETIC_ASR_REG\
        } else {\
          value = reg[opcode & 0x0F].I;\
        }\
      } else {\
        if(reg[opcode & 0x0F].I & 0x80000000){\
          value = 0xFFFFFFFF;\
        } else value = 0;\
      }\
      if(dest == 15) {\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
        }\
	branch_taken=true;\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+7:\
    {\
      /* OP Rd,Rb,Rm ROR Rs */\
      int shift = reg[(opcode >> 8)&15].B.B0;\
      int dest = (opcode>>12) & 15;\
      unsigned int value;\
      if(shift) {\
        shift &= 0x1f;\
        if(shift) {\
           ARITHMETIC_ROR_REG\
        } else {\
           value = reg[opcode & 0x0F].I;\
        }\
      } else {\
        value = reg[opcode & 0x0F].I;\
      }\
      if(dest == 15) {\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
        }\
	branch_taken=true;\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+0x200:\
  case BASE+0x201:\
  case BASE+0x202:\
  case BASE+0x203:\
  case BASE+0x204:\
  case BASE+0x205:\
  case BASE+0x206:\
  case BASE+0x207:\
  case BASE+0x208:\
  case BASE+0x209:\
  case BASE+0x20a:\
  case BASE+0x20b:\
  case BASE+0x20c:\
  case BASE+0x20d:\
  case BASE+0x20e:\
  case BASE+0x20f:\
    {\
      int shift = (opcode & 0xF00) >> 7;\
      int dest = (opcode >> 12) & 0x0F;\
      unsigned int value;\
      {\
        ARITHMETIC_ROR_IMM\
      }\
      if(dest == 15) {\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
        }\
	branch_taken=true;\
      } else {\
        OPCODE \
      }\
    }\
    break;

// macros for thumb mode
#define ADD_RD_RS_RN \
   {\
     unsigned int lhs = reg[source].I;\
     unsigned int rhs = value;\
     unsigned int res = lhs + rhs;\
     reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     ADDCARRY(lhs, rhs, res);\
     ADDOVERFLOW(lhs, rhs, res);\
     CPUUpdateCPSR();\
   }
#define ADD_RD_RS_O3 \
   {\
     unsigned int lhs = reg[source].I;\
     unsigned int rhs = value;\
     unsigned int res = lhs + rhs;\
     reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     ADDCARRY(lhs, rhs, res);\
     ADDOVERFLOW(lhs, rhs, res);\
     CPUUpdateCPSR();\
   }
#define ADD_RN_O8(d) \
   {\
     unsigned int lhs = reg[(d)].I;\
     unsigned int rhs = (opcode & 255);\
     unsigned int res = lhs + rhs;\
     reg[(d)].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     ADDCARRY(lhs, rhs, res);\
     ADDOVERFLOW(lhs, rhs, res);\
     CPUUpdateCPSR();\
   }
#define CMN_RD_RS \
   {\
     unsigned int lhs = reg[dest].I;\
     unsigned int rhs = value;\
     unsigned int res = lhs + rhs;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     ADDCARRY(lhs, rhs, res);\
     ADDOVERFLOW(lhs, rhs, res);\
     CPUUpdateCPSR();\
   }
#define ADC_RD_RS \
   {\
     unsigned int lhs = reg[dest].I;\
     unsigned int rhs = value;\
     unsigned int res = lhs + rhs + (unsigned int)C_FLAG;\
     reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     ADDCARRY(lhs, rhs, res);\
     ADDOVERFLOW(lhs, rhs, res);\
     CPUUpdateCPSR();\
   }
#define SUB_RD_RS_RN \
   {\
     unsigned int lhs = reg[source].I;\
     unsigned int rhs = value;\
     unsigned int res = lhs - rhs;\
     reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(lhs, rhs, res);\
     SUBOVERFLOW(lhs, rhs, res);\
     CPUUpdateCPSR();\
   }
#define SUB_RD_RS_O3 \
   {\
     unsigned int lhs = reg[source].I;\
     unsigned int rhs = value;\
     unsigned int res = lhs - rhs;\
     reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(lhs, rhs, res);\
     SUBOVERFLOW(lhs, rhs, res);\
     CPUUpdateCPSR();\
   }
#define SUB_RN_O8(d) \
   {\
     unsigned int lhs = reg[(d)].I;\
     unsigned int rhs = (opcode & 255);\
     unsigned int res = lhs - rhs;\
     reg[(d)].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(lhs, rhs, res);\
     SUBOVERFLOW(lhs, rhs, res);\
     CPUUpdateCPSR();\
   }
#define CMP_RN_O8(d) \
   {\
     unsigned int lhs = reg[(d)].I;\
     unsigned int rhs = (opcode & 255);\
     unsigned int res = lhs - rhs;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(lhs, rhs, res);\
     SUBOVERFLOW(lhs, rhs, res);\
     CPUUpdateCPSR();\
   }
#define SBC_RD_RS \
   {\
     unsigned int lhs = reg[dest].I;\
     unsigned int rhs = value;\
     unsigned int res = lhs - rhs - !((unsigned int)C_FLAG);\
     reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(lhs, rhs, res);\
     SUBOVERFLOW(lhs, rhs, res);\
     CPUUpdateCPSR();\
   }
#define LSL_RD_RM_I5 \
   {\
     C_FLAG = (reg[source].I >> (32 - shift)) & 1 ? true : false;\
     CPUUpdateCPSR();\
     value = reg[source].I << shift;\
   }
#define LSL_RD_RS \
   {\
     C_FLAG = (reg[dest].I >> (32 - value)) & 1 ? true : false;\
     CPUUpdateCPSR();\
     value = reg[dest].I << value;\
   }
#define LSR_RD_RM_I5 \
   {\
     C_FLAG = (reg[source].I >> (shift - 1)) & 1 ? true : false;\
     CPUUpdateCPSR();\
     value = reg[source].I >> shift;\
   }
#define LSR_RD_RS \
   {\
     C_FLAG = (reg[dest].I >> (value - 1)) & 1 ? true : false;\
     CPUUpdateCPSR();\
     value = reg[dest].I >> value;\
   }
#define ASR_RD_RM_I5 \
   {\
     C_FLAG = ((int)reg[source].I >> (int)(shift - 1)) & 1 ? true : false;\
     CPUUpdateCPSR();\
     value = (int)reg[source].I >> (int)shift;\
   }
#define ASR_RD_RS \
   {\
     C_FLAG = ((int)reg[dest].I >> (int)(value - 1)) & 1 ? true : false;\
     CPUUpdateCPSR();\
     value = (int)reg[dest].I >> (int)value;\
   }
#define ROR_RD_RS \
   {\
     C_FLAG = (reg[dest].I >> (value - 1)) & 1 ? true : false;\
     CPUUpdateCPSR();\
     value = ((reg[dest].I << (32 - value)) |\
              (reg[dest].I >> value));\
   }
#define NEG_RD_RS \
   {\
     unsigned int lhs = reg[source].I;\
     unsigned int rhs = 0;\
     unsigned int res = rhs - lhs;\
     reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(rhs, lhs, res);\
     SUBOVERFLOW(rhs, lhs, res);\
     CPUUpdateCPSR();\
   }
#define CMP_RD_RS \
   {\
     unsigned int lhs = reg[dest].I;\
     unsigned int rhs = value;\
     unsigned int res = lhs - rhs;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(lhs, rhs, res);\
     SUBOVERFLOW(lhs, rhs, res);\
     CPUUpdateCPSR();\
   }

///////////////////////////////////////////////////////
//      Processor structure definition
///////////////////////////////////////////////////////

struct SOCLIB_ARM7:sc_module
{

  //      I/O PORTS  

  sc_in < bool > CLK;
  sc_in < bool > RESETN;
  ICACHE_PROCESSOR_PORTS ICACHE; // instruction cache
  DCACHE_PROCESSOR_PORTS DCACHE; // data cache
  sc_in < bool > IRQ; // IRQ
  sc_in < bool > FIQ; //FIQ

  //      STRUCTURAL PARAMETERS

  const char *NAME;		// Name of the current instance 
  bool *UNCACHED_TABLE;		// pointer on the uncached table
  int MSB_NUMBER;		// number of address MSB bits
  int MSB_MASK;			// mask for the uncached table index
  int IDENT;			// Processor Identification 

  bool N_FLAG;
  bool Z_FLAG;
  bool C_FLAG;
  bool V_FLAG;
  bool armIrqEnable;
  bool armFiqEnable;
  bool armState;
  int armMode;

  int armNextPC;
  unsigned char cpuBitsSet[256];
  unsigned char cpuLowestBitSet[256];

  unsigned char int_vector; // char to store the 7 interrupts
  // we store them in the following manner:
  // 0: Reset
  // 1: Data Abort
  // 2: FIQ
  // 3: IRQ
  // 4: Prefetch Abort
  // 5: Undefined Instruction
  // 6: SWI
  //
  // The highest priority is stored in the LSB

  //      REGISTERS

  reg_pair reg[45];		// General Purpose Registers, used for accessability 
  sc_signal < int >GPR[45];	// General Purpose Registers 
  //sc_signal < int >PC;	// Program Counter 
  sc_signal < int >IR;	// Instruction register 
  sc_signal < int >IRD;	// Instruction register in Decode stage 

  sc_signal <int > COPRO0[1];

  sc_signal<int> swpFsmState;
  sc_signal<unsigned int> tempSwpRead;
  sc_signal<int> mFsmState;
  int num_register; // store the last register seen for stmX & ldmX
  sc_signal<unsigned int> dm_address;
  char reg_remaining; // nb of register we still have to save/load fot srm & ldm
  bool msb;

#ifdef DEBUG
  char logfilename[10];
  FILE * logfile;
#endif

  unsigned int sv;
  int bubble_cycle; // remaining cycles to stall
  bool modified_pc;
  ///////////////////////////////////////////////////////////
  //      constructor
  ///////////////////////////////////////////////////////////

  SC_HAS_PROCESS (SOCLIB_ARM7);

  SOCLIB_ARM7 (sc_module_name insname, int ident)
  {
    SC_METHOD (transition);
    dont_initialize ();
    sensitive_pos << CLK;
    dont_initialize();

    SC_METHOD (genMoore);
    dont_initialize ();
    sensitive_neg << CLK;
    dont_initialize();
    
    NAME = (const char *) insname;
    IDENT = ident;

    // Uncached Table allocation & initialization

//    MSB_NUMBER = segtable.getAddrMSBSize()+segtable.getAddrLSBSize();
//    MSB_MASK = 0x7FFFFFFF >> (31 - MSB_NUMBER);

//    UNCACHED_TABLE = new bool[1 << MSB_NUMBER];
//    segtable.initUncachedTable (UNCACHED_TABLE);

    for (int i = 0; i < 256; i++)
      {
	int count = 0;
	int j;
	for (j = 0; j < 8; j++)
	  if (i & (1 << j))
	    count++;
	cpuBitsSet[i] = count;

	for (j = 0; j < 8; j++)
	  if (i & (1 << j))
	    break;
	cpuLowestBitSet[i] = j;
      }

    swpFsmState=0;
    mFsmState=COMPUTEADDRESS;
    num_register=-1;
    bubble_cycle=0;

    msb = true;
    modified_pc = false;

#ifdef DEBUG
    sprintf(logfilename, "out_p%d",ident);
    logfile = fopen(logfilename,"w");
#endif

    COPRO0[0] = ident;
    //int_vector |= (1<<2);;
    int_vector = 0;
    armMode=0x13;
    printf ("Successful Instanciation of SOCLIB_ARM7 : %s\n", NAME);
  };				// end constructor

  ///////////////////////////////////////////////////////
  //      destructor
  ///////////////////////////////////////////////////////
  
  //SC_HAS_PROCESS(~SOCLIB_ARM7);
  
  ~SOCLIB_ARM7() {
#ifdef DEBUG
    fclose(logfile);
#endif
  } // end destructor

  ///////////////////////////////////////////////////////
  //      Transition()
  ///////////////////////////////////////////////////////

  void SOCLIB_ARM7::transition ()
  {

    for (int i = 0; i < 45; i++)
      reg[i].I = GPR[i];
    
    armMode = reg[16].I & 0x1F;
    
    switch(armMode) {
    case USER_MODE:
    case SYSTEM_MODE:
      break;

    case SUPERVISOR_MODE:
      reg[13].I = reg[R13_SVC].I;
      reg[14].I = reg[R14_SVC].I;
      reg[17].I = reg[SPSR_SVC].I;
      break;

    case ABORT_MODE:
      reg[13].I = reg[R13_ABT].I;
      reg[14].I = reg[R14_ABT].I;
      reg[17].I = reg[SPSR_ABT].I;
      break;

    case UNDEFINED_MODE:
      reg[13].I = reg[R13_UND].I;
      reg[14].I = reg[R14_UND].I;
      reg[17].I = reg[SPSR_UND].I;
      break;

    case IRQ_MODE:
      reg[13].I = reg[R13_IRQ].I;
      reg[14].I = reg[R14_IRQ].I;
      reg[17].I = reg[SPSR_IRQ].I;
      break;

    case FIQ_MODE:
      reg[8].I = reg[R8_FIQ].I;
      reg[9].I = reg[R9_FIQ].I;
      reg[10].I = reg[R10_FIQ].I;
      reg[11].I = reg[R11_FIQ].I;
      reg[12].I = reg[R12_FIQ].I;
      reg[13].I = reg[R13_FIQ].I;
      reg[14].I = reg[R14_FIQ].I;
      reg[17].I = reg[SPSR_FIQ].I;
      break;

    default:
#ifdef DEBUG
      fprintf(logfile,"Error in the CPSR: %x\n",armMode);
#endif
      break;
    }

    if (!(reg[16].I & 0x80) && (IRQ == true)) {
      int_vector |= (1<<IRQ_INT);
    }

    /////////////////////////////
    //      RESET 
    /////////////////////////////

    if (RESETN == false) {
      //       for (int i = 0; i < 45; i++)
      // 	reg[i].I = GPR[i];

#ifdef DEBUG
      fprintf(logfile,"ResetN asserted\n");
      fprintf(logfile,"mode: 0x%x\n",armMode);
      fprintf(logfile,"CPSR: 0x%x\n",reg[16].I);
#endif

      //       GPR[15]=ARM_RESET_VECTOR;
      //       IRD=ARM_NOP_INST;
      //       IR=ARM_NOP_INST;

      swpFsmState=0;
      mFsmState=COMPUTEADDRESS;
      num_register=-1;

      int_vector |= (1<<RESET_INT); 
    }			// end RESET

      
    
    //////////////////////////////////////////////
    //      NO REGISTER WRITE IN CASE OF FRZ
    //////////////////////////////////////////////

    if ((ICACHE.FRZ == true))
      {
// #ifdef DEBUG
// 	fprintf(logfile,"Instruction Cache miss...\n");
// #endif
	return;
      }

    if ((DCACHE.FRZ == true)) {
// #ifdef DEBUG
//       fprintf(logfile,"Data Cache miss...\n");
// #endif
      return;
    }

#ifdef DEBUG
    fprintf(logfile,"\n");
    fprintf(logfile,"--------------------------------------\n");
    fprintf(logfile,"processeur %d\n",COPRO0[0].read()); 
#endif

    //////////////////////////////////////
    //      INSTRUCTION DECODE
    //////////////////////////////////////
    unsigned int opcode = (unsigned int)IRD.read();
    bool branch_taken=false;
    bool need_to_stall=false;
    
    // First read all the registers into structured variables
    // reg[]. GPR[] will be assigned with reg[] values
    // at the end of function transition

#ifdef DEBUG
    fprintf(logfile,"\n");
    fprintf(logfile,"previous code: %x\n", sv);
    fprintf(logfile,"bubble: %d\n", bubble_cycle);
    fprintf(logfile,"mode: 0x%x\n",armMode);
    fprintf(logfile,"CPSR: 0x%x\n",reg[16].I);
    fprintf(logfile,"r[15]: 0x%x\n",reg[15].I);
    fprintf(logfile,"IRD: 0x%x\n", IRD.read());
    fprintf(logfile,"IR: 0x%x\n", IR.read());
    
      for (int i=0;i<4;i++)
	fprintf(logfile," r[%2.2d]=%8.8x",i,reg[i].I);
      fprintf(logfile,"\n");
      for (int i=4;i<8;i++)
	fprintf(logfile," r[%2.2d]=%8.8x",i,reg[i].I);
      fprintf(logfile,"\n");
      for (int i=8;i<12;i++)
	fprintf(logfile," r[%2.2d]=%8.8x",i,reg[i].I);
      fprintf(logfile,"\n");
      for (int i=12;i<16;i++)
	fprintf(logfile," r[%2.2d]=%8.8x",i,reg[i].I);
      fprintf(logfile,"\n");
#endif

    armNextPC = reg[15].I;
    // reg[15]=PC is always incremented.

    if (!armState) {
      if ((reg[15].I % 4) == 0)
	msb = false;
      else
	msb = true;

#ifdef DEBUG
      fprintf(logfile,"Thumb mode...\n");
      fprintf(logfile,"msb: %d\n",(msb)?1:0);
#endif
      // decode thumb instruction to arm instruction
      if (msb)
	opcode = (opcode & 0xFFFF0000) >> 16;
      else
	opcode = (opcode & 0x0000FFFF);
#ifdef DEBUG
      fprintf(logfile,"\n opcode=%x\n\n\n",opcode);
#endif
      switch(opcode >> 8) {
      case 0x00:
      case 0x01:
      case 0x02:
      case 0x03:
      case 0x04:
      case 0x05:
      case 0x06:
      case 0x07:
	{
	  // LSL Rd, Rm, #Imm 5
	  int dest = opcode & 0x07;
	  int source = (opcode >> 3) & 0x07;
	  int shift = (opcode >> 6) & 0x1f;
	  unsigned int value;
     
	  if(shift) {
	    LSL_RD_RM_I5;
	  } else {
	    value = reg[source].I;
	  }
	  reg[dest].I = value;
	  // C_FLAG set above
	  N_FLAG = (value & 0x80000000 ? true : false);
	  Z_FLAG = (value ? false : true);
	}
	break;
      case 0x08:
      case 0x09:
      case 0x0a:
      case 0x0b:
      case 0x0c:
      case 0x0d:
      case 0x0e:
      case 0x0f:
	{
	  // LSR Rd, Rm, #Imm 5
	  int dest = opcode & 0x07;
	  int source = (opcode >> 3) & 0x07;
	  int shift = (opcode >> 6) & 0x1f;
	  unsigned int value;
     
	  if(shift) {
	    LSR_RD_RM_I5;
	  } else {
	    C_FLAG = reg[source].I & 0x80000000 ? true : false;
	    value = 0;
	  }
	  reg[dest].I = value;
	  // C_FLAG set above
	  N_FLAG = (value & 0x80000000 ? true : false);
	  Z_FLAG = (value ? false : true);
	}
	break;
      case 0x10:
      case 0x11:
      case 0x12:
      case 0x13:
      case 0x14:
      case 0x15:
      case 0x16:
      case 0x17:
	{     
	  // ASR Rd, Rm, #Imm 5
	  int dest = opcode & 0x07;
	  int source = (opcode >> 3) & 0x07;
	  int shift = (opcode >> 6) & 0x1f;
	  unsigned int value;
     
	  if(shift) {
	    ASR_RD_RM_I5;
	  } else {
	    if(reg[source].I & 0x80000000) {
	      value = 0xFFFFFFFF;
	      C_FLAG = true;
	    } else {
	      value = 0;
	      C_FLAG = false;
	    }
	  }
	  reg[dest].I = value;
	  // C_FLAG set above
	  N_FLAG = (value & 0x80000000 ? true : false);
	  Z_FLAG = (value ? false :true);
	}
	break;
      case 0x18:
      case 0x19:
	{
	  // ADD Rd, Rs, Rn
	  int dest = opcode & 0x07;
	  int source = (opcode >> 3) & 0x07;
	  unsigned int value = reg[(opcode>>6)& 0x07].I;
	  ADD_RD_RS_RN;
	}
	break;
      case 0x1a:
      case 0x1b:
	{
	  // SUB Rd, Rs, Rn
	  int dest = opcode & 0x07;
	  int source = (opcode >> 3) & 0x07;
	  unsigned int value = reg[(opcode>>6)& 0x07].I;
	  SUB_RD_RS_RN;
	}
	break;
      case 0x1c:
      case 0x1d:
	{
	  // ADD Rd, Rs, #Offset3
	  int dest = opcode & 0x07;
	  int source = (opcode >> 3) & 0x07;
	  unsigned int value = (opcode >> 6) & 7;
	  ADD_RD_RS_O3;
	}
	break;
      case 0x1e:
      case 0x1f:
	{
	  // SUB Rd, Rs, #Offset3
	  int dest = opcode & 0x07;
	  int source = (opcode >> 3) & 0x07;
	  unsigned int value = (opcode >> 6) & 7;
	  SUB_RD_RS_O3;
	}
	break;
      case 0x20:
	// MOV R0, #Offset8
	reg[0].I = opcode & 255;
	N_FLAG = false;
	Z_FLAG = (reg[0].I ? false : true);
	break;
      case 0x21:
	// MOV R1, #Offset8
	reg[1].I = opcode & 255;
	N_FLAG = false;
	Z_FLAG = (reg[1].I ? false : true);
	break;   
      case 0x22:
	// MOV R2, #Offset8
	reg[2].I = opcode & 255;
	N_FLAG = false;
	Z_FLAG = (reg[2].I ? false : true);
	break;   
      case 0x23:
	// MOV R3, #Offset8
	reg[3].I = opcode & 255;
	N_FLAG = false;
	Z_FLAG = (reg[3].I ? false : true);
	break;   
      case 0x24:
	// MOV R4, #Offset8
	reg[4].I = opcode & 255;
	N_FLAG = false;
	Z_FLAG = (reg[4].I ? false : true);
	break;   
      case 0x25:
	// MOV R5, #Offset8
	reg[5].I = opcode & 255;
	N_FLAG = false;
	Z_FLAG = (reg[5].I ? false : true);
	break;   
      case 0x26:
	// MOV R6, #Offset8
	reg[6].I = opcode & 255;
	N_FLAG = false;
	Z_FLAG = (reg[6].I ? false : true);
	break;   
      case 0x27:
	// MOV R7, #Offset8
	reg[7].I = opcode & 255;
	N_FLAG = false;
	Z_FLAG = (reg[7].I ? false : true);
	break;
      case 0x28:
	// CMP R0, #Offset8
	CMP_RN_O8(0);
	break;
      case 0x29:
	// CMP R1, #Offset8
	CMP_RN_O8(1);
	break;
      case 0x2a:
	// CMP R2, #Offset8
	CMP_RN_O8(2);
	break;
      case 0x2b:
	// CMP R3, #Offset8
	CMP_RN_O8(3);
	break;
      case 0x2c:
	// CMP R4, #Offset8
	CMP_RN_O8(4);
	break;
      case 0x2d:
	// CMP R5, #Offset8
	CMP_RN_O8(5);
	break;
      case 0x2e:
	// CMP R6, #Offset8
	CMP_RN_O8(6);
	break;
      case 0x2f:
	// CMP R7, #Offset8
	CMP_RN_O8(7);
	break;
      case 0x30:
	// ADD R0,#Offset8
	ADD_RN_O8(0);
	break;   
      case 0x31:
	// ADD R1,#Offset8
	ADD_RN_O8(1);
	break;   
      case 0x32:
	// ADD R2,#Offset8
	ADD_RN_O8(2);
	break;   
      case 0x33:
	// ADD R3,#Offset8
	ADD_RN_O8(3);
	break;   
      case 0x34:
	// ADD R4,#Offset8
	ADD_RN_O8(4);
	break;   
      case 0x35:
	// ADD R5,#Offset8
	ADD_RN_O8(5);
	break;   
      case 0x36:
	// ADD R6,#Offset8
	ADD_RN_O8(6);
	break;   
      case 0x37:
	// ADD R7,#Offset8
	ADD_RN_O8(7);
	break;
      case 0x38:
	// SUB R0,#Offset8
	SUB_RN_O8(0);
	break;
      case 0x39:
	// SUB R1,#Offset8
	SUB_RN_O8(1);
	break;
      case 0x3a:
	// SUB R2,#Offset8
	SUB_RN_O8(2);
	break;
      case 0x3b:
	// SUB R3,#Offset8
	SUB_RN_O8(3);
	break;
      case 0x3c:
	// SUB R4,#Offset8
	SUB_RN_O8(4);
	break;
      case 0x3d:
	// SUB R5,#Offset8
	SUB_RN_O8(5);
	break;
      case 0x3e:
	// SUB R6,#Offset8
	SUB_RN_O8(6);
	break;
      case 0x3f:
	// SUB R7,#Offset8
	SUB_RN_O8(7);
	break;
      case 0x40:
	switch((opcode >> 6) & 3) {
	case 0x00:
	  {
	    // AND Rd, Rs
	    int dest = opcode & 7;
	    reg[dest].I &= reg[(opcode >> 3)&7].I;
	    N_FLAG = reg[dest].I & 0x80000000 ? true : false;
	    Z_FLAG = reg[dest].I ? false : true;
	  }
	  break;
	case 0x01:
	  // EOR Rd, Rs
	  {
	    int dest = opcode & 7;
	    reg[dest].I ^= reg[(opcode >> 3)&7].I;
	    N_FLAG = reg[dest].I & 0x80000000 ? true : false;
	    Z_FLAG = reg[dest].I ? false : true;
	  }
	  break;
	case 0x02:
	  // LSL Rd, Rs
	  {
	    int dest = opcode & 7;
	    unsigned int value = reg[(opcode >> 3)&7].B.B0;
	    if(value) {
	      if(value == 32) {
		value = 0;
		C_FLAG = (reg[dest].I & 1 ? true : false);
	      } else if(value < 32) {
		LSL_RD_RS;
	      } else {
		value = 0;
		C_FLAG = false;
	      }
	      reg[dest].I = value;        
	    }
	    N_FLAG = reg[dest].I & 0x80000000 ? true : false;
	    Z_FLAG = reg[dest].I ? false : true;
	  }
	  break;
	case 0x03:
	  {
	    // LSR Rd, Rs
	    int dest = opcode & 7;
	    unsigned int value = reg[(opcode >> 3)&7].B.B0;
	    if(value) {
	      if(value == 32) {
		value = 0;
		C_FLAG = (reg[dest].I & 0x80000000 ? true : false);
	      } else if(value < 32) {
		LSR_RD_RS;
	      } else {
		value = 0;
		C_FLAG = false;
	      }
	      reg[dest].I = value;        
	    }
	    N_FLAG = reg[dest].I & 0x80000000 ? true : false;
	    Z_FLAG = reg[dest].I ? false : true;
	  }
	  break;
	}
	break;
      case 0x41:
	switch((opcode >> 6) & 3) {
	case 0x00:
	  {
	    // ASR Rd, Rs
	    int dest = opcode & 7;
	    unsigned int value = reg[(opcode >> 3)&7].B.B0;
	    // ASR
	    if(value) {
	      if(value < 32) {
		ASR_RD_RS;
		reg[dest].I = value;        
	      } else {
		if(reg[dest].I & 0x80000000){
		  reg[dest].I = 0xFFFFFFFF;
		  C_FLAG = true;
		} else {
		  reg[dest].I = 0x00000000;
		  C_FLAG = false;
		}
	      }
	    }
	    N_FLAG = reg[dest].I & 0x80000000 ? true : false;
	    Z_FLAG = reg[dest].I ? false : true;
	  }
	  break;
	case 0x01:
	  {
	    // ADC Rd, Rs
	    int dest = opcode & 0x07;
	    unsigned int value = reg[(opcode >> 3)&7].I;
	    // ADC
	    ADC_RD_RS;
	  }
	  break;
	case 0x02:
	  {
	    // SBC Rd, Rs
	    int dest = opcode & 0x07;
	    unsigned int value = reg[(opcode >> 3)&7].I;
       
	    // SBC
	    SBC_RD_RS;
	  }
	  break;
	case 0x03:
	  // ROR Rd, Rs
	  {
	    int dest = opcode & 7;
	    unsigned int value = reg[(opcode >> 3)&7].B.B0;
       
	    if(value) {
	      value = value & 0x1f;
	      if(value == 0) {
		C_FLAG = (reg[dest].I & 0x80000000 ? true : false);
	      } else {
		ROR_RD_RS;
		reg[dest].I = value;
	      }
	    }
	    N_FLAG = reg[dest].I & 0x80000000 ? true : false;
	    Z_FLAG = reg[dest].I ? false : true;
	  }
	  break;
	}
	break;
      case 0x42:
	switch((opcode >> 6) & 3) {
	case 0x00:
	  {
	    // TST Rd, Rs
	    unsigned int value = reg[opcode & 7].I & reg[(opcode >> 3) & 7].I;
	    N_FLAG = value & 0x80000000 ? true : false;
	    Z_FLAG = value ? false : true;
	  }
	  break;
	case 0x01:
	  {
	    // NEG Rd, Rs
	    int dest = opcode & 7;
	    int source = (opcode >> 3) & 7;
	    NEG_RD_RS;
	  }
	  break;
	case 0x02:
	  {
	    // CMP Rd, Rs
	    int dest = opcode & 7;
	    unsigned int value = reg[(opcode >> 3)&7].I;
	    CMP_RD_RS;
	  }
	  break;
	case 0x03:
	  {
	    // CMN Rd, Rs
	    int dest = opcode & 7;
	    unsigned int value = reg[(opcode >> 3)&7].I;
	    // CMN
	    CMN_RD_RS;
	  }
	  break;
	}
	break;
      case 0x43:
	switch((opcode >> 6) & 3) {
	case 0x00:
	  {
	    // ORR Rd, Rs       
	    int dest = opcode & 7;
	    reg[dest].I |= reg[(opcode >> 3) & 7].I;
	    Z_FLAG = reg[dest].I ? false : true;
	    N_FLAG = reg[dest].I & 0x80000000 ? true : false;
	  }
	  break;
	case 0x01:
	  {
	    // MUL Rd, Rs
	    int dest = opcode & 7;
	    unsigned int rm = reg[(opcode >> 3) & 7].I;
	    reg[dest].I = reg[dest].I * rm;
	    if (((int)rm) < 0)
	      rm = ~rm;
	    Z_FLAG = reg[dest].I ? false : true;
	    N_FLAG = reg[dest].I & 0x80000000 ? true : false;
	  }
	  break;
	case 0x02:
	  {
	    // BIC Rd, Rs
	    int dest = opcode & 7;
	    reg[dest].I &= (~reg[(opcode >> 3) & 7].I);
	    Z_FLAG = reg[dest].I ? false : true;
	    N_FLAG = reg[dest].I & 0x80000000 ? true : false;
	  }
	  break;
	case 0x03:
	  {
	    // MVN Rd, Rs
	    int dest = opcode & 7;
	    reg[dest].I = ~reg[(opcode >> 3) & 7].I;
	    Z_FLAG = reg[dest].I ? false : true;
	    N_FLAG = reg[dest].I & 0x80000000 ? true : false;
	  }
	  break;
	}
	break;
      case 0x44:
	{
	  int dest = opcode & 7;
	  int base = (opcode >> 3) & 7;
	  switch((opcode >> 6)& 3) {
	  default:
	    goto unknown_thumb;
	  case 1:
	    // ADD Rd, Hs
	    reg[dest].I += reg[base+8].I;
	    break;
	  case 2:
	    // ADD Hd, Rs
	    reg[dest+8].I += reg[base].I;
	    if(dest == 7) {
	      reg[15].I &= 0xFFFFFFFE;
	      armNextPC = reg[15].I;
	      reg[15].I += 2;
	    }       
	    break;
	  case 3:
	    // ADD Hd, Hs
	    reg[dest+8].I += reg[base+8].I;
	    if(dest == 7) {
	      reg[15].I &= 0xFFFFFFFE;
	      armNextPC = reg[15].I;
	      reg[15].I += 2;
	    }
	    break;
	  }
	}
	break;
      case 0x45:
	{
	  int dest = opcode & 7;
	  int base = (opcode >> 3) & 7;
	  unsigned int value;
	  switch((opcode >> 6) & 3) {
	  case 0:
	    // CMP Rd, Hs
	    value = reg[base].I;
	    CMP_RD_RS;
	    break;
	  case 1:
	    // CMP Rd, Hs
	    value = reg[base+8].I;
	    CMP_RD_RS;
	    break;
	  case 2:
	    // CMP Hd, Rs
	    value = reg[base].I;
	    dest += 8;
	    CMP_RD_RS;
	    break;
	  case 3:
	    // CMP Hd, Hs
	    value = reg[base+8].I;
	    dest += 8;
	    CMP_RD_RS;
	    break;
	  }
	}
	break;
      case 0x46:
	{
	  int dest = opcode & 7;
	  int base = (opcode >> 3) & 7;
	  switch((opcode >> 6) & 3) {
	  case 0:
	    // this form should not be used...
	    // MOV Rd, Rs
	    reg[dest].I = reg[base].I;
	    break;
	  case 1:
	    // MOV Rd, Hs
	    reg[dest].I = reg[base+8].I;
	    break;
	  case 2:
	    // MOV Hd, Rs
	    reg[dest+8].I = reg[base].I;
	    if(dest == 7) {
	      reg[15].I &= 0xFFFFFFFE;
	      armNextPC = reg[15].I;
	      //reg[15].I += 2;
	    }
	    break;
	  case 3:
	    // MOV Hd, Hs
	    reg[dest+8].I = reg[base+8].I;
	    if(dest == 7) {
	      reg[15].I &= 0xFFFFFFFE;
	      armNextPC = reg[15].I;
	      //reg[15].I += 2;
	    }   
	    break;
	  }
	}
	break;
      case 0x47:
	{
	  int base = (opcode >> 3) & 7;
	  switch((opcode >>6) & 3) {
	  case 0:
	    // BX Rs
	    reg[15].I = (reg[base].I) & 0xFFFFFFFE;
	    if(reg[base].I & 1) {
	      armState = false;
	      armNextPC = reg[15].I;
	      reg[15].I -= 2;
	    } else {
	      armState = true;
	      reg[15].I &= 0xFFFFFFFC;
	      armNextPC = reg[15].I;
	      reg[15].I -= 4;
	    }
	    break;
	  case 1:
	    // BX Hs
	    reg[15].I = (reg[8+base].I) & 0xFFFFFFFE;
	    if(reg[8+base].I & 1) {
	      armState = false;
	      armNextPC = reg[15].I;
	      reg[15].I -= 2;
	    } else {
	      armState = true;
	      reg[15].I &= 0xFFFFFFFC;       
	      armNextPC = reg[15].I;
	      reg[15].I -= 4;
	    }
	    break;
	  default:
	    goto unknown_thumb;
	  }
	}
	break;
      case 0x48:
	// LDR R0,[PC, #Imm]
	{
	  unsigned int address = (reg[15].I & 0xFFFFFFFC) + ((opcode & 0xFF) << 2);
	  reg[0].I = CPUReadMemoryTransition(address);
	}
	break;
      case 0x49:
	// LDR R1,[PC, #Imm]
	{
	  unsigned int address = (reg[15].I & 0xFFFFFFFC) + ((opcode & 0xFF) << 2);   
	  reg[1].I = CPUReadMemoryTransition(address);
	}
	break;
      case 0x4a:
	// LDR R2,[PC, #Imm]
	{
	  unsigned int address = (reg[15].I & 0xFFFFFFFC) + ((opcode & 0xFF) << 2);      
	  reg[2].I = CPUReadMemoryTransition(address);
	}
	break;
      case 0x4b:
	// LDR R3,[PC, #Imm]
	{
	  unsigned int address = (reg[15].I & 0xFFFFFFFC) + ((opcode & 0xFF) << 2);      
	  reg[3].I = CPUReadMemoryTransition(address);
	}
	break;
      case 0x4c:
	// LDR R4,[PC, #Imm]
	{
	  unsigned int address = (reg[15].I & 0xFFFFFFFC) + ((opcode & 0xFF) << 2);      
	  reg[4].I = CPUReadMemoryTransition(address);
	}
	break;
      case 0x4d:
	// LDR R5,[PC, #Imm]
	{
	  unsigned int address = (reg[15].I & 0xFFFFFFFC) + ((opcode & 0xFF) << 2);      
	  reg[5].I = CPUReadMemoryTransition(address);
	}
	break;
      case 0x4e:
	// LDR R6,[PC, #Imm]
	{
	  unsigned int address = (reg[15].I & 0xFFFFFFFC) + ((opcode & 0xFF) << 2);      
	  reg[6].I = CPUReadMemoryTransition(address);
	}
	break;
      case 0x4f:
	// LDR R7,[PC, #Imm]
	{
	  unsigned int address = (reg[15].I & 0xFFFFFFFC) + ((opcode & 0xFF) << 2);      
	  reg[7].I = CPUReadMemoryTransition(address);
	}
	break;
      case 0x50:
      case 0x51:
	// STR Rd, [Rs, Rn]
	{
	}
	break;
      case 0x52:
      case 0x53:
	// STRH Rd, [Rs, Rn]
	{
	}
	break;
      case 0x54:
      case 0x55:
	// STRB Rd, [Rs, Rn]
	{
	}
	break;
      case 0x56:
      case 0x57:
	// LDSB Rd, [Rs, Rn]
	{
	  unsigned int address = reg[(opcode>>3)&7].I + reg[(opcode>>6)&7].I;
	  reg[opcode&7].I = (char)CPUReadByteTransition(address);
	}
	break;
      case 0x58:
      case 0x59:
	// LDR Rd, [Rs, Rn]
	{
	  unsigned int address = reg[(opcode>>3)&7].I + reg[(opcode>>6)&7].I;
	  reg[opcode&7].I = CPUReadMemoryTransition(address);
	}
	break;
      case 0x5a:
      case 0x5b:
	// LDRH Rd, [Rs, Rn]
	{
	  unsigned int address = reg[(opcode>>3)&7].I + reg[(opcode>>6)&7].I;
	  reg[opcode&7].I = CPUReadHalfWordTransition(address);
	}
	break;
      case 0x5c:
      case 0x5d:
	// LDRB Rd, [Rs, Rn]
	{
	  unsigned int address = reg[(opcode>>3)&7].I + reg[(opcode>>6)&7].I;
	  reg[opcode&7].I = CPUReadByteTransition(address);
	}
	break;
      case 0x5e:
      case 0x5f:
	// LDSH Rd, [Rs, Rn]
	{
	  unsigned int address = reg[(opcode>>3)&7].I + reg[(opcode>>6)&7].I;
	  reg[opcode&7].I = (short)CPUReadHalfWordSignedTransition(address);
	}
	break;
      case 0x60:
      case 0x61:
      case 0x62:
      case 0x63:
      case 0x64:
      case 0x65:
      case 0x66:
      case 0x67:
	// STR Rd, [Rs, #Imm]
	{
	}
	break;
      case 0x68:
      case 0x69:
      case 0x6a:
      case 0x6b:
      case 0x6c:
      case 0x6d:
      case 0x6e:
      case 0x6f:
	// LDR Rd, [Rs, #Imm]
	{
	  unsigned int address = reg[(opcode>>3)&7].I + (((opcode>>6)&31)<<2);
	  reg[opcode&7].I = CPUReadMemoryTransition(address);
	}
	break;
      case 0x70:
      case 0x71:
      case 0x72:
      case 0x73:
      case 0x74:
      case 0x75:
      case 0x76:
      case 0x77:
	// STRB Rd, [Rs, #Imm]
	{
	}
	break;
      case 0x78:
      case 0x79:
      case 0x7a:
      case 0x7b:
      case 0x7c:
      case 0x7d:
      case 0x7e:
      case 0x7f:
	// LDRB Rd, [Rs, #Imm]
	{
	  unsigned int address = reg[(opcode>>3)&7].I + (((opcode>>6)&31));
	  reg[opcode&7].I = CPUReadByteTransition(address);
	}
	break;
      case 0x80:
      case 0x81:
      case 0x82:
      case 0x83:
      case 0x84:
      case 0x85:
      case 0x86:
      case 0x87:
	// STRH Rd, [Rs, #Imm]
	{
	}
	break;   
      case 0x88:
      case 0x89:
      case 0x8a:
      case 0x8b:
      case 0x8c:
      case 0x8d:
      case 0x8e:
      case 0x8f:
	// LDRH Rd, [Rs, #Imm]
	{
	  unsigned int address = reg[(opcode>>3)&7].I + (((opcode>>6)&31)<<1);
	  reg[opcode&7].I = CPUReadHalfWordTransition(address);
	}
	break;
      case 0x90: // STR R0, [SP, #Imm]
      case 0x91: // STR R1, [SP, #Imm]
      case 0x92: // STR R2, [SP, #Imm]
      case 0x93: // STR R3, [SP, #Imm]
      case 0x94: // STR R4, [SP, #Imm]
      case 0x95: // STR R5, [SP, #Imm]
      case 0x96: // STR R6, [SP, #Imm]
      case 0x97: // STR R7, [SP, #Imm]
	break;

      case 0x98:
	// LDR R0, [SP, #Imm]
	{
	  unsigned int address = reg[13].I + ((opcode&255)<<2);   
	  reg[0].I = CPUReadMemoryTransition(address);
	}
	break;
      case 0x99:
	// LDR R1, [SP, #Imm]
	{
	  unsigned int address = reg[13].I + ((opcode&255)<<2);   
	  reg[1].I = CPUReadMemoryTransition(address);
	}
	break;
      case 0x9a:
	// LDR R2, [SP, #Imm]
	{
	  unsigned int address = reg[13].I + ((opcode&255)<<2);   
	  reg[2].I = CPUReadMemoryTransition(address);
	}
	break;
      case 0x9b:
	// LDR R3, [SP, #Imm]
	{
	  unsigned int address = reg[13].I + ((opcode&255)<<2);   
	  reg[3].I = CPUReadMemoryTransition(address);
	}
	break;
      case 0x9c:
	// LDR R4, [SP, #Imm]
	{
	  unsigned int address = reg[13].I + ((opcode&255)<<2);   
	  reg[4].I = CPUReadMemoryTransition(address);
	}
	break;
      case 0x9d:
	// LDR R5, [SP, #Imm]
	{
	  unsigned int address = reg[13].I + ((opcode&255)<<2);   
	  reg[5].I = CPUReadMemoryTransition(address);
	}
	break;
      case 0x9e:
	// LDR R6, [SP, #Imm]
	{
	  unsigned int address = reg[13].I + ((opcode&255)<<2);   
	  reg[6].I = CPUReadMemoryTransition(address);
	}
	break;
      case 0x9f:
	// LDR R7, [SP, #Imm]
	{
	  unsigned int address = reg[13].I + ((opcode&255)<<2);   
	  reg[7].I = CPUReadMemoryTransition(address);
	}
	break;
      case 0xa0:
	// ADD R0, PC, Imm
	reg[0].I = (reg[15].I & 0xFFFFFFFC) + ((opcode&255)<<2);
	break;   
      case 0xa1:
	// ADD R1, PC, Imm
	reg[1].I = (reg[15].I & 0xFFFFFFFC) + ((opcode&255)<<2);
	break;   
      case 0xa2:
	// ADD R2, PC, Imm
	reg[2].I = (reg[15].I & 0xFFFFFFFC) + ((opcode&255)<<2);
	break;   
      case 0xa3:
	// ADD R3, PC, Imm
	reg[3].I = (reg[15].I & 0xFFFFFFFC) + ((opcode&255)<<2);
	break;   
      case 0xa4:
	// ADD R4, PC, Imm
	reg[4].I = (reg[15].I & 0xFFFFFFFC) + ((opcode&255)<<2);
	break;   
      case 0xa5:
	// ADD R5, PC, Imm
	reg[5].I = (reg[15].I & 0xFFFFFFFC) + ((opcode&255)<<2);
	break;   
      case 0xa6:
	// ADD R6, PC, Imm
	reg[6].I = (reg[15].I & 0xFFFFFFFC) + ((opcode&255)<<2);
	break;   
      case 0xa7:
	// ADD R7, PC, Imm
	reg[7].I = (reg[15].I & 0xFFFFFFFC) + ((opcode&255)<<2);
	break;   
      case 0xa8:
	// ADD R0, SP, Imm
	reg[0].I = reg[13].I + ((opcode&255)<<2);
	break;   
      case 0xa9:
	// ADD R1, SP, Imm
	reg[1].I = reg[13].I + ((opcode&255)<<2);
	break;   
      case 0xaa:
	// ADD R2, SP, Imm
	reg[2].I = reg[13].I + ((opcode&255)<<2);
	break;   
      case 0xab:
	// ADD R3, SP, Imm
	reg[3].I = reg[13].I + ((opcode&255)<<2);
	break;   
      case 0xac:
	// ADD R4, SP, Imm
	reg[4].I = reg[13].I + ((opcode&255)<<2);
	break;   
      case 0xad:
	// ADD R5, SP, Imm
	reg[5].I = reg[13].I + ((opcode&255)<<2);
	break;   
      case 0xae:
	// ADD R6, SP, Imm
	reg[6].I = reg[13].I + ((opcode&255)<<2);
	break;   
      case 0xaf:
	// ADD R7, SP, Imm
	reg[7].I = reg[13].I + ((opcode&255)<<2);
	break;   
      case 0xb0:
	{
	  // ADD SP, Imm
	  int offset = (opcode & 127) << 2;
	  if(opcode & 0x80)
	    offset = -offset;
	  reg[13].I += offset;
	}
	break;
      case 0xb4:
	// PUSH {Rlist}
	{
	  unsigned int temp = reg[13].I - 4 * cpuBitsSet[opcode & 0xff];
	  unsigned int address = temp & 0xFFFFFFFC;
	  switch(mFsmState) {
	  case COMPUTEADDRESS:
#ifdef DEBUG
	    fprintf(logfile,"Compute Address: %x\n",address);
#endif
	    num_register=-1;
	    need_to_stall=true;
	    reg[15].I -= 2;
	    mFsmState=REGISTER;
	    dm_address = address;
	    break;

	  case REGISTER: {
	    need_to_stall = true;
	    reg[15].I -= 2;
	    int r = findNextRegThumb(opcode);
	    if (r >=0) {
#ifdef DEBUG
	      fprintf(logfile,"Store register %d at @ %x\n",num_register,dm_address.read());
#endif
	    } else
	      mFsmState = END;		  
	    dm_address = dm_address + 4;
	  }
	    break;

	  case END:
#ifdef DEBUG
	    fprintf(logfile,"Push End\n");
#endif
	    need_to_stall=false;
	    reg[13].I = temp;
	    num_register=-1;
	    mFsmState = COMPUTEADDRESS;
	    break;
	  }
	}
	break;
      case 0xb5:
	// PUSH {Rlist, LR}
	{
	  unsigned int temp = reg[13].I - 4 - 4 * cpuBitsSet[opcode & 0xff];
	  unsigned int address = temp & 0xFFFFFFFC;
	  switch(mFsmState) {
	  case COMPUTEADDRESS:
#ifdef DEBUG
	    fprintf(logfile,"Compute Address: %x\n",address);
#endif
	    num_register=-1;
	    need_to_stall=true;
	    reg[15].I -= 2;
	    mFsmState=REGISTER;
	    dm_address = address;
	    break;

	  case REGISTER: {
	    need_to_stall = true;
	    reg[15].I -= 2;
	    int r = findNextRegThumb(opcode);
	    if (r >=0) {
#ifdef DEBUG
	      fprintf(logfile,"Store register %d at @ %x\n",num_register,dm_address.read());
#endif
	    } else
	      mFsmState = END;		  
	    dm_address = dm_address + 4;
	  }
	    break;

	  case END:
#ifdef DEBUG
	    fprintf(logfile,"Push End\n");
#endif
	    need_to_stall=false;
	    reg[13].I = temp;
	    num_register=-1;
	    mFsmState = COMPUTEADDRESS;
	    break;
	  }
	}
	break;

      case 0xbc:
	// POP {Rlist}
	{
	  unsigned int address = reg[13].I & 0xFFFFFFFC;
	  unsigned int temp = reg[13].I + 4*cpuBitsSet[opcode & 0xFF];
	  switch(mFsmState) {
	  case COMPUTEADDRESS:
#ifdef DEBUG
	    fprintf(logfile,"Compute Address: %x\n", address);
#endif
	    num_register=-1;
	    need_to_stall=true;
	    reg[15].I -= 2;
	    mFsmState = REGISTER;
	    dm_address = address;
	    break;

	  case REGISTER:{
	    need_to_stall=true;
	    reg[15].I -= 2;
	    int r = findNextRegThumb(opcode);
	    if (r >= 0) {
#ifdef DEBUG
	      fprintf(logfile,"Load from @ %x in register %d\n",dm_address.read(),num_register);
#endif
	      reg[r].I = CPUReadMemoryTransition(dm_address);
	    } else {
	      mFsmState = END;
	    }
	    dm_address = dm_address + 4;
	  }
	    break;

	  case END:
#ifdef DEBUG
	    fprintf(logfile,"Pop end\n");
#endif
	    need_to_stall = false;
	    num_register = -1;
	    reg[13].I = temp;
	    mFsmState = COMPUTEADDRESS;
	    break;
	  }
	}
	break;   
      case 0xbd:
	// POP {Rlist, PC}
	{
	  unsigned int address = reg[13].I & 0xFFFFFFFC;
	  unsigned int temp = reg[13].I + 4 + 4*cpuBitsSet[opcode & 0xFF];
	  switch(mFsmState) {
	  case COMPUTEADDRESS:
#ifdef DEBUG
	    fprintf(logfile,"Compute Address: %x\n", address);
#endif
	    num_register=-1;
	    need_to_stall=true;
	    reg[15].I -= 2;
	    mFsmState = REGISTER;
	    dm_address = address;
	    break;

	  case REGISTER:{
	    need_to_stall=true;
	    reg[15].I -= 2;
	    int r = findNextRegThumb(opcode);
	    if (r >= 0) {
#ifdef DEBUG
	      fprintf(logfile,"Load from @ %x in register %d\n",dm_address.read(),num_register);
#endif
	      reg[r].I = CPUReadMemoryTransition(dm_address);
	      if (r == 15) {
		reg[15].I = reg[15].I & 0xFFFFFFFE;
		armNextPC = reg[15].I;
		branch_taken=true;
		num_register = -1;
		reg[13].I = temp;
		mFsmState = COMPUTEADDRESS;
	      }
	    } else {
	      mFsmState = END;
	    }
	    dm_address = dm_address + 4;
	  }
	    break;
	    
	  case END:
#ifdef DEBUG
	    fprintf(logfile,"Pop end\n");
#endif
	    need_to_stall = false;
	    num_register = -1;
	    reg[13].I = temp;
	    mFsmState = COMPUTEADDRESS;
	    break;
	  }
	}
	break;

      case 0xc0:
	{
	  // STM R0!, {Rlist}
	  unsigned int address = reg[0].I & 0xFFFFFFFC;
	  unsigned int temp = reg[0].I + 4*cpuBitsSet[opcode & 0xff];
	  switch(mFsmState) {
	  case COMPUTEADDRESS:
#ifdef DEBUG
	    fprintf(logfile,"Compute Address: %x\n",address);
#endif
	    num_register=-1;
	    need_to_stall=true;
	    reg[15].I -= 2;
	    mFsmState=REGISTER;
	    dm_address = address;
	    break;

	  case REGISTER: {
	    need_to_stall = true;
	    reg[15].I -= 2;
	    int r = findNextRegThumb(opcode);
	    if (r >=0) {
#ifdef DEBUG
	      fprintf(logfile,"Store register %d at @ %x\n",num_register,dm_address.read());
#endif
	    } else
	      mFsmState = END;		  
	    dm_address = dm_address + 4;
	  }
	    break;

	  case END:
#ifdef DEBUG
	    fprintf(logfile,"Push End\n");
#endif
	    need_to_stall=false;
	    reg[0].I = temp;
	    num_register=-1;
	    mFsmState = COMPUTEADDRESS;
	    break;
	  }
	}
	break;   
      case 0xc1:
	{
	  // STM R1!, {Rlist}
	  unsigned int address = reg[1].I & 0xFFFFFFFC;
	  unsigned int temp = reg[1].I + 4*cpuBitsSet[opcode & 0xff];
	  switch(mFsmState) {
	  case COMPUTEADDRESS:
#ifdef DEBUG
	    fprintf(logfile,"Compute Address: %x\n",address);
#endif
	    num_register=-1;
	    need_to_stall=true;
	    reg[15].I -= 2;
	    mFsmState=REGISTER;
	    dm_address = address;
	    break;

	  case REGISTER: {
	    need_to_stall = true;
	    reg[15].I -= 2;
	    int r = findNextRegThumb(opcode);
	    if (r >=0) {
#ifdef DEBUG
	      fprintf(logfile,"Store register %d at @ %x\n",num_register,dm_address.read());
#endif
	    } else
	      mFsmState = END;		  
	    dm_address = dm_address + 4;
	  }
	    break;

	  case END:
#ifdef DEBUG
	    fprintf(logfile,"Push End\n");
#endif
	    need_to_stall=false;
	    reg[1].I = temp;
	    num_register=-1;
	    mFsmState = COMPUTEADDRESS;
	    break;
	  }
	}
	break;      
      case 0xc2:
	{
	  // STM R2!, {Rlist}
	  unsigned int address = reg[2].I & 0xFFFFFFFC;
	  unsigned int temp = reg[2].I + 4*cpuBitsSet[opcode & 0xff];
	  switch(mFsmState) {
	  case COMPUTEADDRESS:
#ifdef DEBUG
	    fprintf(logfile,"Compute Address: %x\n",address);
#endif
	    num_register=-1;
	    need_to_stall=true;
	    reg[15].I -= 2;
	    mFsmState=REGISTER;
	    dm_address = address;
	    break;

	  case REGISTER: {
	    need_to_stall = true;
	    reg[15].I -= 2;
	    int r = findNextRegThumb(opcode);
	    if (r >=0) {
#ifdef DEBUG
	      fprintf(logfile,"Store register %d at @ %x\n",num_register,dm_address.read());
#endif
	    } else
	      mFsmState = END;		  
	    dm_address = dm_address + 4;
	  }
	    break;

	  case END:
#ifdef DEBUG
	    fprintf(logfile,"Push End\n");
#endif
	    need_to_stall=false;
	    reg[2].I = temp;
	    num_register=-1;
	    mFsmState = COMPUTEADDRESS;
	    break;
	  }
	}
	break;      
      case 0xc3:
	{
	  // STM R3!, {Rlist}
	  unsigned int address = reg[3].I & 0xFFFFFFFC;
	  unsigned int temp = reg[3].I + 4*cpuBitsSet[opcode & 0xff];
	  switch(mFsmState) {
	  case COMPUTEADDRESS:
#ifdef DEBUG
	    fprintf(logfile,"Compute Address: %x\n",address);
#endif
	    num_register=-1;
	    need_to_stall=true;
	    reg[15].I -= 2;
	    mFsmState=REGISTER;
	    dm_address = address;
	    break;

	  case REGISTER: {
	    need_to_stall = true;
	    reg[15].I -= 2;
	    int r = findNextRegThumb(opcode);
	    if (r >=0) {
#ifdef DEBUG
	      fprintf(logfile,"Store register %d at @ %x\n",num_register,dm_address.read());
#endif
	    } else
	      mFsmState = END;		  
	    dm_address = dm_address + 4;
	  }
	    break;

	  case END:
#ifdef DEBUG
	    fprintf(logfile,"Push End\n");
#endif
	    need_to_stall=false;
	    reg[3].I = temp;
	    num_register=-1;
	    mFsmState = COMPUTEADDRESS;
	    break;
	  }
	}
	break;   
      case 0xc4:
	{
	  // STM R4!, {Rlist}
	  unsigned int address = reg[4].I & 0xFFFFFFFC;
	  unsigned int temp = reg[4].I + 4*cpuBitsSet[opcode & 0xff];
	  switch(mFsmState) {
	  case COMPUTEADDRESS:
#ifdef DEBUG
	    fprintf(logfile,"Compute Address: %x\n",address);
#endif
	    num_register=-1;
	    need_to_stall=true;
	    reg[15].I -= 2;
	    mFsmState=REGISTER;
	    dm_address = address;
	    break;

	  case REGISTER: {
	    need_to_stall = true;
	    reg[15].I -= 2;
	    int r = findNextRegThumb(opcode);
	    if (r >=0) {
#ifdef DEBUG
	      fprintf(logfile,"Store register %d at @ %x\n",num_register,dm_address.read());
#endif
	    } else
	      mFsmState = END;		  
	    dm_address = dm_address + 4;
	  }
	    break;

	  case END:
#ifdef DEBUG
	    fprintf(logfile,"Push End\n");
#endif
	    need_to_stall=false;
	    reg[4].I = temp;
	    num_register=-1;
	    mFsmState = COMPUTEADDRESS;
	    break;
	  }
	}
	break;   
      case 0xc5:
	{
	  // STM R5!, {Rlist}
	  unsigned int address = reg[5].I & 0xFFFFFFFC;
	  unsigned int temp = reg[5].I + 4*cpuBitsSet[opcode & 0xff];
	  switch(mFsmState) {
	  case COMPUTEADDRESS:
#ifdef DEBUG
	    fprintf(logfile,"Compute Address: %x\n",address);
#endif
	    num_register=-1;
	    need_to_stall=true;
	    reg[15].I -= 2;
	    mFsmState=REGISTER;
	    dm_address = address;
	    break;

	  case REGISTER: {
	    need_to_stall = true;
	    reg[15].I -= 2;
	    int r = findNextRegThumb(opcode);
	    if (r >=0) {
#ifdef DEBUG
	      fprintf(logfile,"Store register %d at @ %x\n",num_register,dm_address.read());
#endif
	    } else
	      mFsmState = END;		  
	    dm_address = dm_address + 4;
	  }
	    break;

	  case END:
#ifdef DEBUG
	    fprintf(logfile,"Push End\n");
#endif
	    need_to_stall=false;
	    reg[5].I = temp;
	    num_register=-1;
	    mFsmState = COMPUTEADDRESS;
	    break;
	  }
	}
	break;   
      case 0xc6:
	{
	  // STM R6!, {Rlist}
	  unsigned int address = reg[6].I & 0xFFFFFFFC;
	  unsigned int temp = reg[6].I + 4*cpuBitsSet[opcode & 0xff];
	  switch(mFsmState) {
	  case COMPUTEADDRESS:
#ifdef DEBUG
	    fprintf(logfile,"Compute Address: %x\n",address);
#endif
	    num_register=-1;
	    need_to_stall=true;
	    reg[15].I -= 2;
	    mFsmState=REGISTER;
	    dm_address = address;
	    break;

	  case REGISTER: {
	    need_to_stall = true;
	    reg[15].I -= 2;
	    int r = findNextRegThumb(opcode);
	    if (r >=0) {
#ifdef DEBUG
	      fprintf(logfile,"Store register %d at @ %x\n",num_register,dm_address.read());
#endif
	    } else
	      mFsmState = END;		  
	    dm_address = dm_address + 4;
	  }
	    break;

	  case END:
#ifdef DEBUG
	    fprintf(logfile,"Push End\n");
#endif
	    need_to_stall=false;
	    reg[6].I = temp;
	    num_register=-1;
	    mFsmState = COMPUTEADDRESS;
	    break;
	  }
	}
	break;   
      case 0xc7:
	{
	  // STM R7!, {Rlist}
	  unsigned int address = reg[7].I & 0xFFFFFFFC;
	  unsigned int temp = reg[7].I + 4*cpuBitsSet[opcode & 0xff];
	  switch(mFsmState) {
	  case COMPUTEADDRESS:
#ifdef DEBUG
	    fprintf(logfile,"Compute Address: %x\n",address);
#endif
	    num_register=-1;
	    need_to_stall=true;
	    reg[15].I -= 2;
	    mFsmState=REGISTER;
	    dm_address = address;
	    break;

	  case REGISTER: {
	    need_to_stall = true;
	    reg[15].I -= 2;
	    int r = findNextRegThumb(opcode);
	    if (r >=0) {
#ifdef DEBUG
	      fprintf(logfile,"Store register %d at @ %x\n",num_register,dm_address.read());
#endif
	    } else
	      mFsmState = END;		  
	    dm_address = dm_address + 4;
	  }
	    break;

	  case END:
#ifdef DEBUG
	    fprintf(logfile,"Push End\n");
#endif
	    need_to_stall=false;
	    reg[7].I = temp;
	    num_register=-1;
	    mFsmState = COMPUTEADDRESS;
	    break;
	  }
	}
	break;

      case 0xc8:
	{
	  // LDM R0!, {Rlist}
	  unsigned int address = reg[0].I & 0xFFFFFFFC;
 	  unsigned int temp = reg[0].I + 4*cpuBitsSet[opcode & 0xFF];
	  switch(mFsmState) {
	  case COMPUTEADDRESS:
#ifdef DEBUG
	    fprintf(logfile,"Compute Address: %x\n", address);
#endif
	    num_register=-1;
	    need_to_stall=true;
	    reg[15].I -= 2;
	    mFsmState = REGISTER;
	    dm_address = address;
	    break;
	    
	  case REGISTER:{
	    need_to_stall=true;
	    reg[15].I -= 2;
	    int r = findNextRegThumb(opcode);
	    if (r >= 0) {
#ifdef DEBUG
	      fprintf(logfile,"Load from @ %x in register %d\n",dm_address.read(),num_register);
#endif
	      reg[r].I = CPUReadMemoryTransition(dm_address);
	    } else {
	      mFsmState = END;
	    }
	    dm_address = dm_address + 4;
	  }
	    break;
	    
	  case END:
#ifdef DEBUG
	    fprintf(logfile,"Ldm end\n");
#endif
	    need_to_stall = false;
	    num_register = -1;
	    if (!(opcode & 1))
	      reg[0].I = temp;
	    mFsmState = COMPUTEADDRESS;
	    break;
	  }
	}
	break;
      case 0xc9:
	{
	  // LDM R1!, {Rlist}
	  unsigned int address = reg[1].I & 0xFFFFFFFC;
	  unsigned int temp = reg[1].I + 4*cpuBitsSet[opcode & 0xFF];
	  switch(mFsmState) {
	  case COMPUTEADDRESS:
#ifdef DEBUG
	    fprintf(logfile,"Compute Address: %x\n", address);
#endif
	    num_register=-1;
	    need_to_stall=true;
	    reg[15].I -= 2;
	    mFsmState = REGISTER;
	    dm_address = address;
	    break;
	    
	  case REGISTER:{
	    need_to_stall=true;
	    reg[15].I -= 2;
	    int r = findNextRegThumb(opcode);
	    if (r >= 0) {
#ifdef DEBUG
	      fprintf(logfile,"Load from @ %x in register %d\n",dm_address.read(),num_register);
#endif
	      reg[r].I = CPUReadMemoryTransition(dm_address);
	    } else {
	      mFsmState = END;
	    }
	    dm_address = dm_address + 4;
	  }
	    break;
	    
	  case END:
#ifdef DEBUG
	    fprintf(logfile,"Ldm end\n");
#endif
	    need_to_stall = false;
	    num_register = -1;
	    if (!(opcode & 2))
	      reg[1].I = temp;
	    mFsmState = COMPUTEADDRESS;
	    break;
	  }
	}
	break;
      case 0xca:
	{
	  // LDM R2!, {Rlist}
	  unsigned int address = reg[2].I & 0xFFFFFFFC;
	  unsigned int temp = reg[2].I + 4*cpuBitsSet[opcode & 0xFF];
	  switch(mFsmState) {
	  case COMPUTEADDRESS:
#ifdef DEBUG
	    fprintf(logfile,"Compute Address: %x\n", address);
#endif
	    num_register=-1;
	    need_to_stall=true;
	    reg[15].I -= 2;
	    mFsmState = REGISTER;
	    dm_address = address;
	    break;
	    
	  case REGISTER:{
	    need_to_stall=true;
	    reg[15].I -= 2;
	    int r = findNextRegThumb(opcode);
	    if (r >= 0) {
#ifdef DEBUG
	      fprintf(logfile,"Load from @ %x in register %d\n",dm_address.read(),num_register);
#endif
	      reg[r].I = CPUReadMemoryTransition(dm_address);
	    } else {
	      mFsmState = END;
	    }
	    dm_address = dm_address + 4;
	  }
	    break;
	    
	  case END:
#ifdef DEBUG
	    fprintf(logfile,"Ldm end\n");
#endif
	    need_to_stall = false;
	    num_register = -1;
	    if (!(opcode & 4))
	      reg[2].I = temp;
	    mFsmState = COMPUTEADDRESS;
	    break;
	  }
	}
	break;
      case 0xcb:
	{
	  // LDM R3!, {Rlist}
	  unsigned int address = reg[3].I & 0xFFFFFFFC;
	  unsigned int temp = reg[3].I + 4*cpuBitsSet[opcode & 0xFF];
	  switch(mFsmState) {
	  case COMPUTEADDRESS:
#ifdef DEBUG
	    fprintf(logfile,"Compute Address: %x\n", address);
#endif
	    num_register=-1;
	    need_to_stall=true;
	    reg[15].I -= 2;
	    mFsmState = REGISTER;
	    dm_address = address;
	    break;
	    
	  case REGISTER:{
	    need_to_stall=true;
	    reg[15].I -= 2;
	    int r = findNextRegThumb(opcode);
	    if (r >= 0) {
#ifdef DEBUG
	      fprintf(logfile,"Load from @ %x in register %d\n",dm_address.read(),num_register);
#endif
	      reg[r].I = CPUReadMemoryTransition(dm_address);
	    } else {
	      mFsmState = END;
	    }
	    dm_address = dm_address + 4;
	  }
	    break;
	    
	  case END:
#ifdef DEBUG
	    fprintf(logfile,"Ldm end\n");
#endif
	    need_to_stall = false;
	    num_register = -1;
	    if (!(opcode & 8))
	      reg[3].I = temp;
	    mFsmState = COMPUTEADDRESS;
	    break;
	  }
	}
	break;
      case 0xcc:
	{
	  // LDM R4!, {Rlist}
	  unsigned int address = reg[4].I & 0xFFFFFFFC;
	  unsigned int temp = reg[4].I + 4*cpuBitsSet[opcode & 0xFF];
	  switch(mFsmState) {
	  case COMPUTEADDRESS:
#ifdef DEBUG
	    fprintf(logfile,"Compute Address: %x\n", address);
#endif
	    num_register=-1;
	    need_to_stall=true;
	    reg[15].I -= 2;
	    mFsmState = REGISTER;
	    dm_address = address;
	    break;
	    
	  case REGISTER:{
	    need_to_stall=true;
	    reg[15].I -= 2;
	    int r = findNextRegThumb(opcode);
	    if (r >= 0) {
#ifdef DEBUG
	      fprintf(logfile,"Load from @ %x in register %d\n",dm_address.read(),num_register);
#endif
	      reg[r].I = CPUReadMemoryTransition(dm_address);
	    } else {
	      mFsmState = END;
	    }
	    dm_address = dm_address + 4;
	  }
	    break;
	    
	  case END:
#ifdef DEBUG
	    fprintf(logfile,"Ldm end\n");
#endif
	    need_to_stall = false;
	    num_register = -1;
	    if (!(opcode & 0x10))
	      reg[4].I = temp;
	    mFsmState = COMPUTEADDRESS;
	    break;
	  }
	}
	break;
      case 0xcd:
	{
	  // LDM R5!, {Rlist}
	  unsigned int address = reg[5].I & 0xFFFFFFFC;
	  unsigned int temp = reg[5].I + 4*cpuBitsSet[opcode & 0xFF];
	  switch(mFsmState) {
	  case COMPUTEADDRESS:
#ifdef DEBUG
	    fprintf(logfile,"Compute Address: %x\n", address);
#endif
	    num_register=-1;
	    need_to_stall=true;
	    reg[15].I -= 2;
	    mFsmState = REGISTER;
	    dm_address = address;
	    break;
	    
	  case REGISTER:{
	    need_to_stall=true;
	    reg[15].I -= 2;
	    int r = findNextRegThumb(opcode);
	    if (r >= 0) {
#ifdef DEBUG
	      fprintf(logfile,"Load from @ %x in register %d\n",dm_address.read(),num_register);
#endif
	      reg[r].I = CPUReadMemoryTransition(dm_address);
	    } else {
	      mFsmState = END;
	    }
	    dm_address = dm_address + 4;
	  }
	    break;
	    
	  case END:
#ifdef DEBUG
	    fprintf(logfile,"Ldm end\n");
#endif
	    need_to_stall = false;
	    num_register = -1;
	    if (!(opcode & 0x20))
	      reg[5].I = temp;
	    mFsmState = COMPUTEADDRESS;
	    break;
	  }
 	}
	break;
      case 0xce:
	{
	  // LDM R6!, {Rlist}
	  unsigned int address = reg[6].I & 0xFFFFFFFC;
	  unsigned int temp = reg[6].I + 4*cpuBitsSet[opcode & 0xFF];
	  switch(mFsmState) {
	  case COMPUTEADDRESS:
#ifdef DEBUG
	    fprintf(logfile,"Compute Address: %x\n", address);
#endif
	    num_register=-1;
	    need_to_stall=true;
	    reg[15].I -= 2;
	    mFsmState = REGISTER;
	    dm_address = address;
	    break;
	    
	  case REGISTER:{
	    need_to_stall=true;
	    reg[15].I -= 2;
	    int r = findNextRegThumb(opcode);
	    if (r >= 0) {
#ifdef DEBUG
	      fprintf(logfile,"Load from @ %x in register %d\n",dm_address.read(),num_register);
#endif
	      reg[r].I = CPUReadMemoryTransition(dm_address);
	    } else {
	      mFsmState = END;
	    }
	    dm_address = dm_address + 4;
	  }
	    break;
	    
	  case END:
#ifdef DEBUG
	    fprintf(logfile,"Ldm end\n");
#endif
	    need_to_stall = false;
	    num_register = -1;
	    if (!(opcode & 0x40))
	      reg[6].I = temp;
	    mFsmState = COMPUTEADDRESS;
	    break;
	  }
	}
	break;
      case 0xcf:
	{
	  // LDM R7!, {Rlist}
	  unsigned int address = reg[7].I & 0xFFFFFFFC;
	  unsigned int temp = reg[7].I + 4*cpuBitsSet[opcode & 0xFF];
	  switch(mFsmState) {
	  case COMPUTEADDRESS:
#ifdef DEBUG
	    fprintf(logfile,"Compute Address: %x\n", address);
#endif
	    num_register=-1;
	    need_to_stall=true;
	    reg[15].I -= 2;
	    mFsmState = REGISTER;
	    dm_address = address;
	    break;
	    
	  case REGISTER:{
	    need_to_stall=true;
	    reg[15].I -= 2;
	    int r = findNextRegThumb(opcode);
	    if (r >= 0) {
#ifdef DEBUG
	      fprintf(logfile,"Load from @ %x in register %d\n",dm_address.read(),num_register);
#endif
	      reg[r].I = CPUReadMemoryTransition(dm_address);
	    } else {
	      mFsmState = END;
	    }
	    dm_address = dm_address + 4;
	  }
	    break;
	    
	  case END:
#ifdef DEBUG
	    fprintf(logfile,"Ldm end\n");
#endif
	    need_to_stall = false;
	    num_register = -1;
	    if (!(opcode & 0x80))
	      reg[7].I = temp;
	    mFsmState = COMPUTEADDRESS;
	    break;
	  }
	}
	break;
      case 0xd0:
	// BEQ offset
	if(Z_FLAG) {
	  reg[15].I += ((char)(opcode & 0xFF)) << 1;
	  armNextPC = reg[15].I;
	  branch_taken = true;
	}
	break;      
      case 0xd1:
	// BNE offset
	if(!Z_FLAG) {
	  reg[15].I += ((char)(opcode & 0xFF)) << 1;       
	  armNextPC = reg[15].I;
	  branch_taken = true;
	}
	break;   
      case 0xd2:
	// BCS offset
	if(C_FLAG) {
	  reg[15].I += ((char)(opcode & 0xFF)) << 1;       
	  armNextPC = reg[15].I;
	  branch_taken = true;
	}
	break;   
      case 0xd3:
	// BCC offset
	if(!C_FLAG) {
	  reg[15].I += ((char)(opcode & 0xFF)) << 1;       
	  armNextPC = reg[15].I;
	  branch_taken = true;
	}
	break;   
      case 0xd4:
	// BMI offset
	if(N_FLAG) {
	  reg[15].I += ((char)(opcode & 0xFF)) << 1;       
	  armNextPC = reg[15].I;
	  branch_taken = true;
	}
	break;   
      case 0xd5:
	// BPL offset
	if(!N_FLAG) {
	  reg[15].I += ((char)(opcode & 0xFF)) << 1;       
	  armNextPC = reg[15].I;
	  branch_taken = true;
	}
	break;   
      case 0xd6:
	// BVS offset
	if(V_FLAG) {
	  reg[15].I += ((char)(opcode & 0xFF)) << 1;       
	  armNextPC = reg[15].I;
	  branch_taken = true;
	}
	break;   
      case 0xd7:
	// BVC offset
	if(!V_FLAG) {
	  reg[15].I += ((char)(opcode & 0xFF)) << 1;       
	  armNextPC = reg[15].I;
	  branch_taken = true;
	}
	break;   
      case 0xd8:
	// BHI offset
	if(C_FLAG && !Z_FLAG) {
	  reg[15].I += ((char)(opcode & 0xFF)) << 1;       
	  armNextPC = reg[15].I;
	  branch_taken = true;
	}
	break;   
      case 0xd9:
	// BLS offset
	if(!C_FLAG || Z_FLAG) {
	  reg[15].I += ((char)(opcode & 0xFF)) << 1;       
	  armNextPC = reg[15].I;
	  branch_taken = true;
	}
	break;   
      case 0xda:
	// BGE offset
	if(N_FLAG == V_FLAG) {
	  reg[15].I += ((char)(opcode & 0xFF)) << 1;       
	  armNextPC = reg[15].I;
	  branch_taken = true;
	}
	break;   
      case 0xdb:
	// BLT offset
	if(N_FLAG != V_FLAG) {
	  reg[15].I += ((char)(opcode & 0xFF)) << 1;       
	  armNextPC = reg[15].I;
	  branch_taken = true;
	}
	break;   
      case 0xdc:
	// BGT offset
	if(!Z_FLAG && (N_FLAG == V_FLAG)) {
	  reg[15].I += ((char)(opcode & 0xFF)) << 1;       
	  armNextPC = reg[15].I;
	  branch_taken = true;
	}
	break;   
      case 0xdd:
	// BLE offset
	if(Z_FLAG || (N_FLAG != V_FLAG)) {
	  reg[15].I += ((char)(opcode & 0xFF)) << 1;       
	  armNextPC = reg[15].I;
	  branch_taken = true;
	}
	break;   
      case 0xdf:
	// SWI #comment
	CPUSoftwareInterrupt(opcode & 0xFF);
	break;
      case 0xe0:
      case 0xe1:
      case 0xe2:
      case 0xe3:
      case 0xe4:
      case 0xe5:
      case 0xe6:
      case 0xe7:
	{
	  // B offset
	  int offset = (opcode & 0x3FF) << 1;
	  if(opcode & 0x0400)
	    offset |= 0xFFFFF800;
	  reg[15].I += offset;
	  armNextPC = reg[15].I;
	  branch_taken = true;
	}
	break;
      case 0xf0:
      case 0xf1:
      case 0xf2:
      case 0xf3:
	{
	  // BLL #offset
	  int offset = (opcode & 0x7FF);
	  reg[14].I = reg[15].I + (offset << 12);
	}
	break;      
      case 0xf4:
      case 0xf5:
      case 0xf6:
      case 0xf7:
	{
	  // BLL #offset
	  int offset = (opcode & 0x7FF);
	  reg[14].I = reg[15].I + ((offset << 12) | 0xFF800000);
	}
	break;   
      case 0xf8:
      case 0xf9:
      case 0xfa:
      case 0xfb:
      case 0xfc:
      case 0xfd:
      case 0xfe:
      case 0xff:
	{
	  // BLH #offset
	  int offset = (opcode & 0x7FF);
	  unsigned int temp = reg[15].I-2;
	  reg[15].I = (reg[14].I + (offset<<1))&0xFFFFFFFE;
	  armNextPC = reg[15].I;
	  branch_taken = true;
	  reg[14].I = temp|1;
	}
	break;
      case 0xb1:
      case 0xb2:
      case 0xb3:
      case 0xb6:
      case 0xb7:
      case 0xb8:
      case 0xb9:
      case 0xba:
      case 0xbb:
      case 0xbf:
      case 0xde:
      default:
      unknown_thumb:
	CPUUndefinedException();
	break;
      }
    }
    
    else {
#ifdef DEBUG
      fprintf(logfile,"\n opcode=%x\n\n\n",opcode);
#endif
      int cond = opcode >> 28;
      // suggested optimization for frequent cases
      bool cond_res;
      if (cond == 0x0e)
	{
	  cond_res = true;
	}
      else
	{
	  switch (cond)
	    {
	    case 0x00:		// EQ
	      cond_res = Z_FLAG;
	      break;
	    case 0x01:		// NE
	      cond_res = !Z_FLAG;
	      break;
	    case 0x02:		// CS
	      cond_res = C_FLAG;
	      break;
	    case 0x03:		// CC
	      cond_res = !C_FLAG;
	      break;
	    case 0x04:		// MI
	      cond_res = N_FLAG;
	      break;
	    case 0x05:		// PL
	      cond_res = !N_FLAG;
	      break;
	    case 0x06:		// VS
	      cond_res = V_FLAG;
	      break;
	    case 0x07:		// VC
	      cond_res = !V_FLAG;
	      break;
	    case 0x08:		// HI
	      cond_res = C_FLAG && !Z_FLAG;
	      break;
	    case 0x09:		// LS
	      cond_res = !C_FLAG || Z_FLAG;
	      break;
	    case 0x0A:		// GE
	      cond_res = N_FLAG == V_FLAG;
	      break;
	    case 0x0B:		// LT
	      cond_res = N_FLAG != V_FLAG;
	      break;
	    case 0x0C:		// GT
	      cond_res = !Z_FLAG && (N_FLAG == V_FLAG);
	      break;
	    case 0x0D:		// LE
	      cond_res = Z_FLAG || (N_FLAG != V_FLAG);
	      break;
	    case 0x0E:
	      cond_res = true;
	      break;
	    case 0x0F:
	    default:
	      // ???
	      cond_res = false;
	      break;
	    }
	}
      if (cond_res) {
        sv=((opcode >> 16) & 0xFF0) | ((opcode >> 4) & 0x0F);
#ifdef DEBUG
	fprintf(logfile,"svTransition=%x\n",sv);
#endif
	switch(sv)
	  {
	    LOGICAL_DATA_OPCODE (ARM_OP_AND, ARM_OP_AND, 0x000);
	    LOGICAL_DATA_OPCODE (ARM_OP_ANDS, ARM_OP_AND, 0x010);
	  case 0x009:
	    {
	      // MUL Rd, Rm, Rs
	      int dest = (opcode >> 16) & 0x0F;
	      int mult = (opcode & 0x0F);
	      unsigned int rs = reg[(opcode >> 8) & 0x0F].I;
	      reg[dest].I = reg[mult].I * rs;
	    }
	    break;
	  case 0x019:
	    {
	      // MULS Rd, Rm, Rs
	      int dest = (opcode >> 16) & 0x0F;
	      int mult = (opcode & 0x0F);
	      unsigned int rs = reg[(opcode >> 8) & 0x0F].I;
	      reg[dest].I = reg[mult].I * rs;
	      N_FLAG = (reg[dest].I & 0x80000000) ? true : false;
	      Z_FLAG = (reg[dest].I) ? false : true;
	    }
	    break;
	  case 0x00b:
	  case 0x02b:
	    {
	      // STRH Rd, [Rn], -Rm
	      int base = (opcode >> 16) & 0x0F;
	      unsigned int address = reg[base].I;
	      int offset = reg[opcode & 0x0F].I;
	      address -= offset;
	      reg[base].I = address;
	    }
	    break;
	  case 0x04b:
	  case 0x06b:
	    {
	      // STRH Rd, [Rn], #-offset
	      int base = (opcode >> 16) & 0x0F;
	      unsigned int address = reg[base].I;
	      int offset = (opcode & 0x0F) | ((opcode >> 4) & 0xF0);
	      address -= offset;
	      reg[base].I = address;
	    }
	    break;
	  case 0x08b:
	  case 0x0ab:
	    {
	      // STRH Rd, [Rn], Rm
	      int base = (opcode >> 16) & 0x0F;
	      unsigned int address = reg[base].I;
	      int offset = reg[opcode & 0x0F].I;
	      address += offset;
	      reg[base].I = address;
	    }
	    break;
	  case 0x0cb:
	  case 0x0eb:
	    {
	      // STRH Rd, [Rn], #offset
	      int base = (opcode >> 16) & 0x0F;
	      unsigned int address = reg[base].I;
	      int offset = (opcode & 0x0F) | ((opcode >> 4) & 0xF0);
	      address += offset;
	      reg[base].I = address;
	    }
	    break;
	  case 0x10b:
	    {
	      // STRH Rd, [Rn, -Rm]
	    }
	    break;
	  case 0x12b:
	    {
	      // STRH Rd, [Rn, -Rm]!
	      int base = (opcode >> 16) & 0x0F;
	      unsigned int address = reg[base].I - reg[opcode & 0x0F].I;
	      reg[base].I = address;
	    }
	    break;
	  case 0x14b:
	    {
	      // STRH Rd, [Rn, -#offset]
	    }
	    break;
	  case 0x16b:
	    {
	      // STRH Rd, [Rn, -#offset]!
	      int base = (opcode >> 16) & 0x0F;
	      unsigned int address =
		reg[base].I - ((opcode & 0x0F) | ((opcode >> 4) & 0xF0));
	      reg[base].I = address;
	    }
	    break;
	  case 0x18b:
	    {
	      // STRH Rd, [Rn, Rm]
	    }
	    break;
	  case 0x1ab:
	    {
	      // STRH Rd, [Rn, Rm]!
	      int base = (opcode >> 16) & 0x0F;
	      unsigned int address = reg[base].I + reg[opcode & 0x0F].I;
	      reg[base].I = address;
	    }
	    break;
	  case 0x1cb:
	    {
	      // STRH Rd, [Rn, #offset]
	    }
	    break;
	  case 0x1eb:
	    {
	      // STRH Rd, [Rn, #offset]!
	      int base = (opcode >> 16) & 0x0F;
	      unsigned int address =
		reg[base].I + ((opcode & 0x0F) | ((opcode >> 4) & 0xF0));
	      reg[base].I = address;
	    }
	    break;
	  case 0x01b:
	  case 0x03b:
	    {
	      // LDRH Rd, [Rn], -Rm
	      int base = (opcode >> 16) & 0x0F;
	      int dest = (opcode >> 12) & 0x0F;
	      unsigned int address = reg[base].I;
	      int offset = reg[opcode & 0x0F].I;
	      reg[dest].I = CPUReadHalfWordTransition (address);
	      if (dest != base)
		{
		  address -= offset;
		  reg[base].I = address;
		}
	    }
	    break;
	  case 0x05b:
	  case 0x07b:
	    {
	      // LDRH Rd, [Rn], #-offset
	      int base = (opcode >> 16) & 0x0F;
	      int dest = (opcode >> 12) & 0x0F;
	      unsigned int address = reg[base].I;
	      int offset = (opcode & 0x0F) | ((opcode >> 4) & 0xF0);
	      reg[dest].I = CPUReadHalfWordTransition (address);
	      if (dest != base)
		{
		  address -= offset;
		  reg[base].I = address;
		}
	    }
	    break;
	  case 0x09b:
	  case 0x0bb:
	    {
	      // LDRH Rd, [Rn], Rm
	      int base = (opcode >> 16) & 0x0F;
	      int dest = (opcode >> 12) & 0x0F;
	      unsigned int address = reg[base].I;
	      int offset = reg[opcode & 0x0F].I;
	      reg[dest].I = CPUReadHalfWordTransition (address);
	      if (dest != base)
		{
		  address += offset;
		  reg[base].I = address;
		}
	    }
	    break;
	  case 0x0db:
	  case 0x0fb:
	    {
	      // LDRH Rd, [Rn], #offset
	      int base = (opcode >> 16) & 0x0F;
	      int dest = (opcode >> 12) & 0x0F;
	      unsigned int address = reg[base].I;
	      int offset = (opcode & 0x0F) | ((opcode >> 4) & 0xF0);
	      reg[dest].I = CPUReadHalfWordTransition (address);
	      if (dest != base)
		{
		  address += offset;
		  reg[base].I = address;
		}
	    }
	    break;
	  case 0x11b:
	    {
	      // LDRH Rd, [Rn, -Rm]
	      int base = (opcode >> 16) & 0x0F;
	      int dest = (opcode >> 12) & 0x0F;
	      unsigned int address = reg[base].I - reg[opcode & 0x0F].I;
	      reg[dest].I = CPUReadHalfWordTransition (address);
	    }
	    break;
	  case 0x13b:
	    {
	      // LDRH Rd, [Rn, -Rm]!
	      int base = (opcode >> 16) & 0x0F;
	      int dest = (opcode >> 12) & 0x0F;
	      unsigned int address = reg[base].I - reg[opcode & 0x0F].I;
	      reg[dest].I = CPUReadHalfWordTransition (address);
	      if (dest != base)
		reg[base].I = address;
	    }
	    break;
	  case 0x15b:
	    {
	      // LDRH Rd, [Rn, -#offset]
	      int base = (opcode >> 16) & 0x0F;
	      int dest = (opcode >> 12) & 0x0F;
	      unsigned int address =
		reg[base].I - ((opcode & 0x0F) | ((opcode >> 4) & 0xF0));
	      reg[dest].I = CPUReadHalfWordTransition (address);
	    }
	    break;
	  case 0x17b:
	    {
	      // LDRH Rd, [Rn, -#offset]!
	      int base = (opcode >> 16) & 0x0F;
	      int dest = (opcode >> 12) & 0x0F;
	      unsigned int address =
		reg[base].I - ((opcode & 0x0F) | ((opcode >> 4) & 0xF0));
	      reg[dest].I = CPUReadHalfWordTransition (address);
	      if (dest != base)
		reg[base].I = address;
	    }
	    break;
	  case 0x19b:
	    {
	      // LDRH Rd, [Rn, Rm]
	      int base = (opcode >> 16) & 0x0F;
	      int dest = (opcode >> 12) & 0x0F;
	      unsigned int address = reg[base].I + reg[opcode & 0x0F].I;
	      reg[dest].I = CPUReadHalfWordTransition (address);
	    }
	    break;
	  case 0x1bb:
	    {
	      // LDRH Rd, [Rn, Rm]!
	      int base = (opcode >> 16) & 0x0F;
	      int dest = (opcode >> 12) & 0x0F;
	      unsigned int address = reg[base].I + reg[opcode & 0x0F].I;
	      reg[dest].I = CPUReadHalfWordTransition (address);
	      if (dest != base)
		reg[base].I = address;
	    }
	    break;
	  case 0x1db:
	    {
	      // LDRH Rd, [Rn, #offset]
	      int base = (opcode >> 16) & 0x0F;
	      int dest = (opcode >> 12) & 0x0F;
	      unsigned int address =
		reg[base].I + ((opcode & 0x0F) | ((opcode >> 4) & 0xF0));
	      reg[dest].I = CPUReadHalfWordTransition (address);
	    }
	    break;
	  case 0x1fb:
	    {
	      // LDRH Rd, [Rn, #offset]!
	      int base = (opcode >> 16) & 0x0F;
	      int dest = (opcode >> 12) & 0x0F;
	      unsigned int address =
		reg[base].I + ((opcode & 0x0F) | ((opcode >> 4) & 0xF0));
	      reg[dest].I = CPUReadHalfWordTransition (address);
	      if (dest != base)
		reg[base].I = address;
	    }
	    break;
	  case 0x01d:
	  case 0x03d:
	    {
	      // LDRSB Rd, [Rn], -Rm
	      int base = (opcode >> 16) & 0x0F;
	      int dest = (opcode >> 12) & 0x0F;
	      unsigned int address = reg[base].I;
	      int offset = reg[opcode & 0x0F].I;
	      reg[dest].I = (signed char) CPUReadByteTransition (address);
	      if (dest != base)
		{
		  address -= offset;
		  reg[base].I = address;
		}
	    }
	    break;
	  case 0x05d:
	  case 0x07d:
	    {
	      // LDRSB Rd, [Rn], #-offset
	      int base = (opcode >> 16) & 0x0F;
	      int dest = (opcode >> 12) & 0x0F;
	      unsigned int address = reg[base].I;
	      int offset = (opcode & 0x0F) | ((opcode >> 4) & 0xF0);
	      reg[dest].I = (signed char) CPUReadByteTransition (address);
	      if (dest != base)
		{
		  address -= offset;
		  reg[base].I = address;
		}
	    }
	    break;
	  case 0x09d:
	  case 0x0bd:
	    {
	      // LDRSB Rd, [Rn], Rm
	      int base = (opcode >> 16) & 0x0F;
	      int dest = (opcode >> 12) & 0x0F;
	      unsigned int address = reg[base].I;
	      int offset = reg[opcode & 0x0F].I;
	      reg[dest].I = (signed char) CPUReadByteTransition (address);
	      if (dest != base)
		{
		  address += offset;
		  reg[base].I = address;
		}
	    }
	    break;
	  case 0x0dd:
	  case 0x0fd:
	    {
	      // LDRSB Rd, [Rn], #offset
	      int base = (opcode >> 16) & 0x0F;
	      int dest = (opcode >> 12) & 0x0F;
	      unsigned int address = reg[base].I;
	      int offset = (opcode & 0x0F) | ((opcode >> 4) & 0xF0);
	      reg[dest].I = (signed char) CPUReadByteTransition (address);
	      if (dest != base)
		{
		  address += offset;
		  reg[base].I = address;
		}
	    }
	    break;
	  case 0x11d:
	    {
	      // LDRSB Rd, [Rn, -Rm]
	      int base = (opcode >> 16) & 0x0F;
	      int dest = (opcode >> 12) & 0x0F;
	      unsigned int address = reg[base].I - reg[opcode & 0x0F].I;
	      reg[dest].I = (signed char) CPUReadByteTransition (address);
	    }
	    break;
	  case 0x13d:
	    {
	      // LDRSB Rd, [Rn, -Rm]!
	      int base = (opcode >> 16) & 0x0F;
	      int dest = (opcode >> 12) & 0x0F;
	      unsigned int address = reg[base].I - reg[opcode & 0x0F].I;
	      reg[dest].I = (signed char) CPUReadByteTransition (address);
	      if (dest != base)
		reg[base].I = address;
	    }
	    break;
	  case 0x15d:
	    {
	      // LDRSB Rd, [Rn, -#offset]
	      int base = (opcode >> 16) & 0x0F;
	      int dest = (opcode >> 12) & 0x0F;
	      unsigned int address =
		reg[base].I - ((opcode & 0x0F) | ((opcode >> 4) & 0xF0));
	      reg[dest].I = (signed char) CPUReadByteTransition (address);
	    }
	    break;
	  case 0x17d:
	    {
	      // LDRSB Rd, [Rn, -#offset]!
	      int base = (opcode >> 16) & 0x0F;
	      int dest = (opcode >> 12) & 0x0F;
	      unsigned int address =
		reg[base].I - ((opcode & 0x0F) | ((opcode >> 4) & 0xF0));
	      reg[dest].I = (signed char) CPUReadByteTransition (address);
	      if (dest != base)
		reg[base].I = address;
	    }
	    break;
	  case 0x19d:
	    {
	      // LDRSB Rd, [Rn, Rm]
	      int base = (opcode >> 16) & 0x0F;
	      int dest = (opcode >> 12) & 0x0F;
	      unsigned int address = reg[base].I + reg[opcode & 0x0F].I;
	      reg[dest].I = (signed char) CPUReadByteTransition (address);
	    }
	    break;
	  case 0x1bd:
	    {
	      // LDRSB Rd, [Rn, Rm]!
	      int base = (opcode >> 16) & 0x0F;
	      int dest = (opcode >> 12) & 0x0F;
	      unsigned int address = reg[base].I + reg[opcode & 0x0F].I;
	      reg[dest].I = (signed char) CPUReadByteTransition (address);
	      if (dest != base)
		reg[base].I = address;
	    }
	    break;
	  case 0x1dd:
	    {
	      // LDRSB Rd, [Rn, #offset]
	      int base = (opcode >> 16) & 0x0F;
	      int dest = (opcode >> 12) & 0x0F;
	      unsigned int address =
		reg[base].I + ((opcode & 0x0F) | ((opcode >> 4) & 0xF0));
	      reg[dest].I = (signed char) CPUReadByteTransition (address);
	    }
	    break;
	  case 0x1fd:
	    {
	      // LDRSB Rd, [Rn, #offset]!
	      int base = (opcode >> 16) & 0x0F;
	      int dest = (opcode >> 12) & 0x0F;
	      unsigned int address =
		reg[base].I + ((opcode & 0x0F) | ((opcode >> 4) & 0xF0));
	      reg[dest].I = (signed char) CPUReadByteTransition (address);
	      if (dest != base)
		reg[base].I = address;
	    }
	    break;
	  case 0x01f:
	  case 0x03f:
	    {
	      // LDRSH Rd, [Rn], -Rm
	      int base = (opcode >> 16) & 0x0F;
	      int dest = (opcode >> 12) & 0x0F;
	      unsigned int address = reg[base].I;
	      int offset = reg[opcode & 0x0F].I;
	      reg[dest].I = (signed short) CPUReadHalfWordSignedTransition (address);
	      if (dest != base)
		{
		  address -= offset;
		  reg[base].I = address;
		}
	    }
	    break;
	  case 0x05f:
	  case 0x07f:
	    {
	      // LDRSH Rd, [Rn], #-offset
	      int base = (opcode >> 16) & 0x0F;
	      int dest = (opcode >> 12) & 0x0F;
	      unsigned int address = reg[base].I;
	      int offset = (opcode & 0x0F) | ((opcode >> 4) & 0xF0);
	      reg[dest].I = (signed short) CPUReadHalfWordSignedTransition (address);
	      if (dest != base)
		{
		  address -= offset;
		  reg[base].I = address;
		}
	    }
	    break;
	  case 0x09f:
	  case 0x0bf:
	    {
	      // LDRSH Rd, [Rn], Rm
	      int base = (opcode >> 16) & 0x0F;
	      int dest = (opcode >> 12) & 0x0F;
	      unsigned int address = reg[base].I;
	      int offset = reg[opcode & 0x0F].I;
	      reg[dest].I = (signed short) CPUReadHalfWordSignedTransition (address);
	      if (dest != base)
		{
		  address += offset;
		  reg[base].I = address;
		}
	    }
	    break;
	  case 0x0df:
	  case 0x0ff:
	    {
	      // LDRSH Rd, [Rn], #offset
	      int base = (opcode >> 16) & 0x0F;
	      int dest = (opcode >> 12) & 0x0F;
	      unsigned int address = reg[base].I;
	      int offset = (opcode & 0x0F) | ((opcode >> 4) & 0xF0);
	      reg[dest].I = (signed short) CPUReadHalfWordSignedTransition (address);
	      if (dest != base)
		{
		  address += offset;
		  reg[base].I = address;
		}
	    }
	    break;
	  case 0x11f:
	    {
	      // LDRSH Rd, [Rn, -Rm]
	      int base = (opcode >> 16) & 0x0F;
	      int dest = (opcode >> 12) & 0x0F;
	      unsigned int address = reg[base].I - reg[opcode & 0x0F].I;
	      reg[dest].I = (signed short) CPUReadHalfWordSignedTransition (address);
	    }
	    break;
	  case 0x13f:
	    {
	      // LDRSH Rd, [Rn, -Rm]!
	      int base = (opcode >> 16) & 0x0F;
	      int dest = (opcode >> 12) & 0x0F;
	      unsigned int address = reg[base].I - reg[opcode & 0x0F].I;
	      reg[dest].I = (signed short) CPUReadHalfWordSignedTransition (address);
	      if (dest != base)
		reg[base].I = address;
	    }
	    break;
	  case 0x15f:
	    {
	      // LDRSH Rd, [Rn, -#offset]
	      int base = (opcode >> 16) & 0x0F;
	      int dest = (opcode >> 12) & 0x0F;
	      unsigned int address =
		reg[base].I - ((opcode & 0x0F) | ((opcode >> 4) & 0xF0));
	      reg[dest].I = (signed short) CPUReadHalfWordSignedTransition (address);
	    }
	    break;
	  case 0x17f:
	    {
	      // LDRSH Rd, [Rn, -#offset]!
	      int base = (opcode >> 16) & 0x0F;
	      int dest = (opcode >> 12) & 0x0F;
	      unsigned int address =
		reg[base].I - ((opcode & 0x0F) | ((opcode >> 4) & 0xF0));
	      reg[dest].I = (signed short) CPUReadHalfWordSignedTransition (address);
	      if (dest != base)
		reg[base].I = address;
	    }
	    break;
	  case 0x19f:
	    {
	      // LDRSH Rd, [Rn, Rm]
	      int base = (opcode >> 16) & 0x0F;
	      int dest = (opcode >> 12) & 0x0F;
	      unsigned int address = reg[base].I + reg[opcode & 0x0F].I;
	      reg[dest].I = (signed short) CPUReadHalfWordSignedTransition (address);
	    }
	    break;
	  case 0x1bf:
	    {
	      // LDRSH Rd, [Rn, Rm]!
	      int base = (opcode >> 16) & 0x0F;
	      int dest = (opcode >> 12) & 0x0F;
	      unsigned int address = reg[base].I + reg[opcode & 0x0F].I;
	      reg[dest].I = (signed short) CPUReadHalfWordSignedTransition (address);
	      if (dest != base)
		reg[base].I = address;
	    }
	    break;
	  case 0x1df:
	    {
	      // LDRSH Rd, [Rn, #offset]
	      int base = (opcode >> 16) & 0x0F;
	      int dest = (opcode >> 12) & 0x0F;
	      unsigned int address =
		reg[base].I + ((opcode & 0x0F) | ((opcode >> 4) & 0xF0));
	      reg[dest].I = (signed short) CPUReadHalfWordSignedTransition (address);
	    }
	    break;
	  case 0x1ff:
	    {
	      // LDRSH Rd, [Rn, #offset]!
	      int base = (opcode >> 16) & 0x0F;
	      int dest = (opcode >> 12) & 0x0F;
	      unsigned int address =
		reg[base].I + ((opcode & 0x0F) | ((opcode >> 4) & 0xF0));
	      reg[dest].I = (signed short) CPUReadHalfWordSignedTransition (address);
	      if (dest != base)
		reg[base].I = address;
	    }
	    break;
	    LOGICAL_DATA_OPCODE (ARM_OP_EOR, ARM_OP_EOR, 0x020);
	    LOGICAL_DATA_OPCODE (ARM_OP_EORS, ARM_OP_EOR, 0x030);
	  case 0x029:
	    {
	      // MLA Rd, Rm, Rs, Rn
	      int dest = (opcode >> 16) & 0x0F;
	      int mult = (opcode & 0x0F);
	      unsigned int rs = reg[(opcode >> 8) & 0x0F].I;
	      reg[dest].I = reg[mult].I * rs + reg[(opcode >> 12) & 0x0f].I;
	      if (((signed int) rs) < 0)
		rs = ~rs;
	    }
	    break;
	  case 0x039:
	    {
	      // MLAS Rd, Rm, Rs, Rn
	      int dest = (opcode >> 16) & 0x0F;
	      int mult = (opcode & 0x0F);
	      unsigned int rs = reg[(opcode >> 8) & 0x0F].I;
	      reg[dest].I = reg[mult].I * rs + reg[(opcode >> 12) & 0x0f].I;
	      N_FLAG = (reg[dest].I & 0x80000000) ? true : false;
	      Z_FLAG = (reg[dest].I) ? false : true;
	      if (((signed int) rs) < 0)
		rs = ~rs;
	    }
	    break;
	    ARITHMETIC_DATA_OPCODE (ARM_OP_SUB, ARM_OP_SUB, 0x040);
	    ARITHMETIC_DATA_OPCODE (ARM_OP_SUBS, ARM_OP_SUB, 0x050);
	    ARITHMETIC_DATA_OPCODE (ARM_OP_RSB, ARM_OP_RSB, 0x060);
	    ARITHMETIC_DATA_OPCODE (ARM_OP_RSBS, ARM_OP_RSB, 0x070);
	    ARITHMETIC_DATA_OPCODE (ARM_OP_ADD, ARM_OP_ADD, 0x080);
	    ARITHMETIC_DATA_OPCODE (ARM_OP_ADDS, ARM_OP_ADD, 0x090);
	  case 0x089:
	    {
	      // UMULL RdLo, RdHi, Rn, Rs
	      unsigned int umult = reg[(opcode & 0x0F)].I;
	      unsigned int usource = reg[(opcode >> 8) & 0x0F].I;
	      int destLo = (opcode >> 12) & 0x0F;
	      int destHi = (opcode >> 16) & 0x0F;
	      unsigned long long uTemp =
		((unsigned long long) umult) * ((unsigned long long) usource);
	      reg[destLo].I = (unsigned int) uTemp;
	      reg[destHi].I = (unsigned int) (uTemp >> 32);
	    }
	    break;
	  case 0x099:
	    {
	      // UMULLS RdLo, RdHi, Rn, Rs
	      unsigned int umult = reg[(opcode & 0x0F)].I;
	      unsigned int usource = reg[(opcode >> 8) & 0x0F].I;
	      int destLo = (opcode >> 12) & 0x0F;
	      int destHi = (opcode >> 16) & 0x0F;
	      unsigned long long uTemp =
		((unsigned long long) umult) * ((unsigned long long) usource);
	      reg[destLo].I = (unsigned int) uTemp;
	      reg[destHi].I = (unsigned int) (uTemp >> 32);
	      Z_FLAG = (uTemp) ? false : true;
	      N_FLAG = (reg[destHi].I & 0x80000000) ? true : false;
	    }
	    break;
	    ARITHMETIC_DATA_OPCODE (ARM_OP_ADC, ARM_OP_ADC, 0x0a0);
	    ARITHMETIC_DATA_OPCODE (ARM_OP_ADCS, ARM_OP_ADC, 0x0b0);
	  case 0x0a9:
	    {
	      // UMLAL RdLo, RdHi, Rn, Rs
	      unsigned int umult = reg[(opcode & 0x0F)].I;
	      unsigned int usource = reg[(opcode >> 8) & 0x0F].I;
	      int destLo = (opcode >> 12) & 0x0F;
	      int destHi = (opcode >> 16) & 0x0F;
	      unsigned long long uTemp = (unsigned long long) reg[destHi].I;
	      uTemp <<= 32;
	      uTemp |= (unsigned long long) reg[destLo].I;
	      uTemp +=
		((unsigned long long) umult) * ((unsigned long long) usource);
	      reg[destLo].I = (unsigned int) uTemp;
	      reg[destHi].I = (unsigned int) (uTemp >> 32);
	    }
	    break;
	  case 0x0b9:
	    {
	      // UMLALS RdLo, RdHi, Rn, Rs
	      unsigned int umult = reg[(opcode & 0x0F)].I;
	      unsigned int usource = reg[(opcode >> 8) & 0x0F].I;
	      int destLo = (opcode >> 12) & 0x0F;
	      int destHi = (opcode >> 16) & 0x0F;
	      unsigned long long uTemp = (unsigned long long) reg[destHi].I;
	      uTemp <<= 32;
	      uTemp |= (unsigned long long) reg[destLo].I;
	      uTemp +=
		((unsigned long long) umult) * ((unsigned long long) usource);
	      reg[destLo].I = (unsigned int) uTemp;
	      reg[destHi].I = (unsigned int) (uTemp >> 32);
	      Z_FLAG = (uTemp) ? false : true;
	      N_FLAG = (reg[destHi].I & 0x80000000) ? true : false;
	    }
	    break;
	    ARITHMETIC_DATA_OPCODE (ARM_OP_SBC, ARM_OP_SBC, 0x0c0);
	    ARITHMETIC_DATA_OPCODE (ARM_OP_SBCS, ARM_OP_SBC, 0x0d0);
	  case 0x0c9:
	    {
	      // SMULL RdLo, RdHi, Rm, Rs
	      int destLo = (opcode >> 12) & 0x0F;
	      int destHi = (opcode >> 16) & 0x0F;
	      unsigned int rs = reg[(opcode >> 8) & 0x0F].I;
	      signed long long m = (signed int) reg[(opcode & 0x0F)].I;
	      signed long long s = (signed int) rs;
	      signed long long sTemp = m * s;
	      reg[destLo].I = (unsigned int) sTemp;
	      reg[destHi].I = (unsigned int) (sTemp >> 32);
	      if (((signed int) rs) < 0)
		rs = ~rs;
	    }
	    break;
	  case 0x0d9:
	    {
	      // SMULLS RdLo, RdHi, Rm, Rs
	      int destLo = (opcode >> 12) & 0x0F;
	      int destHi = (opcode >> 16) & 0x0F;
	      unsigned int rs = reg[(opcode >> 8) & 0x0F].I;
	      signed long long m = (signed int) reg[(opcode & 0x0F)].I;
	      signed long long s = (signed int) rs;
	      signed long long sTemp = m * s;
	      reg[destLo].I = (unsigned int) sTemp;
	      reg[destHi].I = (unsigned int) (sTemp >> 32);
	      Z_FLAG = (sTemp) ? false : true;
	      N_FLAG = (sTemp < 0) ? true : false;
	      if (((signed int) rs) < 0)
		rs = ~rs;
	    }
	    break;
	    ARITHMETIC_DATA_OPCODE (ARM_OP_RSC, ARM_OP_RSC, 0x0e0);
	    ARITHMETIC_DATA_OPCODE (ARM_OP_RSCS, ARM_OP_RSC, 0x0f0);
	  case 0x0e9:
	    {
	      // SMLAL RdLo, RdHi, Rm, Rs
	      int destLo = (opcode >> 12) & 0x0F;
	      int destHi = (opcode >> 16) & 0x0F;
	      unsigned int rs = reg[(opcode >> 8) & 0x0F].I;
	      signed long long m = (signed int) reg[(opcode & 0x0F)].I;
	      signed long long s = (signed int) rs;
	      signed long long sTemp = (unsigned long long) reg[destHi].I;
	      sTemp <<= 32;
	      sTemp |= (unsigned long long) reg[destLo].I;
	      sTemp += m * s;
	      reg[destLo].I = (unsigned int) sTemp;
	      reg[destHi].I = (unsigned int) (sTemp >> 32);
	      if (((signed int) rs) < 0)
		rs = ~rs;
	    }
	    break;
	  case 0x0f9:
	    {
	      // SMLALS RdLo, RdHi, Rm, Rs
	      int destLo = (opcode >> 12) & 0x0F;
	      int destHi = (opcode >> 16) & 0x0F;
	      unsigned int rs = reg[(opcode >> 8) & 0x0F].I;
	      signed long long m = (signed int) reg[(opcode & 0x0F)].I;
	      signed long long s = (signed int) rs;
	      signed long long sTemp = (unsigned long long) reg[destHi].I;
	      sTemp <<= 32;
	      sTemp |= (unsigned long long) reg[destLo].I;
	      sTemp += m * s;
	      reg[destLo].I = (unsigned int) sTemp;
	      reg[destHi].I = (unsigned int) (sTemp >> 32);
	      Z_FLAG = (sTemp) ? false : true;
	      N_FLAG = (sTemp < 0) ? true : false;
	      if (((signed int) rs) < 0)
		rs = ~rs;
	    }
	    break;
	    LOGICAL_DATA_OPCODE (ARM_OP_TST, ARM_OP_TST, 0x110);
	  case 0x100:
	    // MRS Rd, CPSR
	    // TODO: check if right instruction....
#ifdef DEBUG
	    fprintf(logfile,"CPSR before: %x\n",reg[16].I);
#endif
	    CPUUpdateCPSR ();
#ifdef DEBUG
	    fprintf(logfile,"CPSR after: %x\n",reg[16].I);
#endif
	    reg[(opcode >> 12) & 0x0F].I = reg[16].I;
	    break;
	  case 0x109:
	    {
	      // SWP Rd, Rm, [Rn]
	      unsigned int address = reg[(opcode >> 16) & 0xF].I;
              switch(swpFsmState)
		{
		case 0:
#ifdef DEBUG
		  fprintf(logfile,"SWP 0 read address=%d\n",address);
#endif
		  reg[15].I -=4;
		  need_to_stall=true;
		  tempSwpRead = CPUReadMemoryTransition (address);
		  swpFsmState=1;
		  break;
		case 1:
#ifdef DEBUG
		  fprintf(logfile,"SWP 1 Memory write: %x\n",reg[opcode & 0xF].I);
#endif
		  reg[15].I -= 4;
		  need_to_stall=true;
		  swpFsmState=2;
		  break;
		case 2:
#ifdef DEBUG
		  fprintf(logfile,"SWP 2 tempSwpRead=%x\n",tempSwpRead.read());
#endif
		  reg[(opcode >> 12) & 0xF].I = tempSwpRead;
		  swpFsmState=0;
		  break;
		default:
		  break;
		}
	    }
	    break;
	    LOGICAL_DATA_OPCODE (ARM_OP_TEQ, ARM_OP_TEQ, 0x130);
	  case 0x120:
	    {
	      // MSR CPSR_fields, Rm
	      CPUUpdateCPSR ();
	      unsigned int value = reg[opcode & 0xF].I;
	      unsigned int newValue = reg[16].I;
	      if (armMode > 0x10)
		{
		  if (opcode & 0x00010000)
		    newValue = (newValue & 0xFFFFFF00) | (value & 0x000000FF);
		  if (opcode & 0x00020000)
		    newValue = (newValue & 0xFFFF00FF) | (value & 0x0000FF00);
		  if (opcode & 0x00040000)
		    newValue = (newValue & 0xFF00FFFF) | (value & 0x00FF0000);
		}
	      if (opcode & 0x00080000)
		newValue = (newValue & 0x00FFFFFF) | (value & 0xFF000000);
	      newValue |= 0x10;
	      CPUSwitchMode (newValue & 0x1f, false);
	      reg[16].I = newValue;
	      CPUUpdateFlags ();
	    }
	    break;
	  case 0x121:
	    {
	      // BX Rm
	      // TODO: check if right instruction...
	      int base = opcode & 0x0F;
	      armState = reg[base].I & 1 ? false : true;
	      if (armState)
		{
		  reg[15].I = reg[base].I & 0xFFFFFFFC;
		  reg[15].I -= 4;
		}
	      else
		{
		  reg[15].I = reg[base].I & 0xFFFFFFFE;
		  reg[15].I -= 2;
		}
	      armNextPC = reg[15].I;
	      branch_taken = true;
	    }
	    break;
	    ARITHMETIC_DATA_OPCODE (ARM_OP_CMP, ARM_OP_CMP, 0x150);
	  case 0x140:
	    // MRS Rd, SPSR
	    // TODO: check if right instruction...
	    reg[(opcode >> 12) & 0x0F].I = reg[17].I;
	    break;
	  case 0x149:
	    {
	      // SWPB Rd, Rm, [Rn]
	      unsigned int address = reg[(opcode >> 16) & 15].I;
	      unsigned int temp = CPUReadByteTransition (address);
	      reg[(opcode >> 12) & 15].I = temp;
	    }
	    break;

	    ARITHMETIC_DATA_OPCODE (ARM_OP_CMN, ARM_OP_CMN, 0x170);

	  case 0x160:
	    {
	      // MSR SPSR_fields, Rm
	      unsigned int value = reg[opcode & 15].I;
	      if (armMode > 0x10 && armMode < 0x1f)
		{
		  if (opcode & 0x00010000)
		    reg[17].I =
		      (reg[17].I & 0xFFFFFF00) | (value & 0x000000FF);
		  if (opcode & 0x00020000)
		    reg[17].I =
		      (reg[17].I & 0xFFFF00FF) | (value & 0x0000FF00);
		  if (opcode & 0x00040000)
		    reg[17].I =
		      (reg[17].I & 0xFF00FFFF) | (value & 0x00FF0000);
		  if (opcode & 0x00080000)
		    reg[17].I =
		      (reg[17].I & 0x00FFFFFF) | (value & 0xFF000000);
		}
	    }
	    break;

	    LOGICAL_DATA_OPCODE (ARM_OP_ORR, ARM_OP_ORR, 0x180);
	    LOGICAL_DATA_OPCODE (ARM_OP_ORRS, ARM_OP_ORR, 0x190);
	    LOGICAL_DATA_OPCODE (ARM_OP_MOV, ARM_OP_MOV, 0x1a0);
	    LOGICAL_DATA_OPCODE (ARM_OP_MOVS, ARM_OP_MOV, 0x1b0);
	    LOGICAL_DATA_OPCODE (ARM_OP_BIC, ARM_OP_BIC, 0x1c0);
	    LOGICAL_DATA_OPCODE (ARM_OP_BICS, ARM_OP_BIC, 0x1d0);
	    LOGICAL_DATA_OPCODE (ARM_OP_MVN, ARM_OP_MVN, 0x1e0);
	    LOGICAL_DATA_OPCODE (ARM_OP_MVNS, ARM_OP_MVN, 0x1f0);
	    
	    CASE_16 (0x320) {
	      // MSR CPSR_fields, #
	      CPUUpdateCPSR ();
	      unsigned int value = opcode & 0xFF;
	      int shift = (opcode & 0xF00) >> 7;
	      if (shift)
		{
		  ROR_IMM_MSR;
		}
	      unsigned int newValue = reg[16].I;
	      if (armMode > 0x10)
		{
		  if (opcode & 0x00010000)
		    newValue = (newValue & 0xFFFFFF00) | (value & 0x000000FF);
		  if (opcode & 0x00020000)
		    newValue = (newValue & 0xFFFF00FF) | (value & 0x0000FF00);
		  if (opcode & 0x00040000)
		    newValue = (newValue & 0xFF00FFFF) | (value & 0x00FF0000);
		}
	      if (opcode & 0x00080000)
		newValue = (newValue & 0x00FFFFFF) | (value & 0xFF000000);

	      newValue |= 0x10;

	      CPUSwitchMode (newValue & 0x1f, false);
	      reg[16].I = newValue;
	      CPUUpdateFlags ();
	    }
	    break;

	    CASE_16 (0x360) {
	      // MSR SPSR_fields, #
	      if (armMode > 0x10 && armMode < 0x1f)
		{
		  unsigned int value = opcode & 0xFF;
		  int shift = (opcode & 0xF00) >> 7;
		  if (shift)
		    {
		      ROR_IMM_MSR;
		    }
		  if (opcode & 0x00010000)
		    reg[17].I =
		      (reg[17].I & 0xFFFFFF00) | (value & 0x000000FF);
		  if (opcode & 0x00020000)
		    reg[17].I =
		      (reg[17].I & 0xFFFF00FF) | (value & 0x0000FF00);
		  if (opcode & 0x00040000)
		    reg[17].I =
		      (reg[17].I & 0xFF00FFFF) | (value & 0x00FF0000);
		  if (opcode & 0x00080000)
		    reg[17].I =
		      (reg[17].I & 0x00FFFFFF) | (value & 0xFF000000);
		}
	    }
	    break;
	    CASE_16 (0x400)
	      // T versions shouldn't be different on GBA      
	      CASE_16 (0x420)
	      {
		// STR Rd, [Rn], -#
		int offset = opcode & 0xFFF;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I;
		reg[base].I = address - offset;
	      }
	    break;
	    CASE_16 (0x480)
	      // T versions shouldn't be different on GBA
	      CASE_16 (0x4a0)
	      {
		// STR Rd, [Rn], #
		int offset = opcode & 0xFFF;
		//int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I;
		reg[base].I = address + offset;
	      }
	    break;
	    CASE_16 (0x500)
	      {
		// STR Rd, [Rn, -#]
	      }
	    break;
	    CASE_16 (0x520)
	      {
		// STR Rd, [Rn, -#]!
		int offset = opcode & 0xFFF;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I - offset;
		reg[base].I = address;
	      }
	    break;
	    CASE_16 (0x580)
	      {
		// STR Rd, [Rn, #]
	      }
	    break;
	    CASE_16 (0x5a0)
	      {
		// STR Rd, [Rn, #]!
		int offset = opcode & 0xFFF;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I + offset;
		reg[base].I = address;
	      }
	    break;
	    CASE_16 (0x410)
	      {
		// LDR Rd, [Rn], -#
		int offset = opcode & 0xFFF;
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I;
		reg[dest].I = CPUReadMemoryTransition (address);
		if (dest != base)
		  reg[base].I -= offset;
		if (dest == 15)
		  {
		    modified_pc = true;
		    branch_taken = true;
		    reg[15].I &= 0xFFFFFFFC;
		    armNextPC = reg[15].I;
		  }
	      }
	    break;
	    CASE_16 (0x430)
	      {
		// LDRT Rd, [Rn], -#
		int offset = opcode & 0xFFF;
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I;
		reg[dest].I = CPUReadMemoryTransition (address);
		if (dest != base)
		  reg[base].I -= offset;
	      }
	    break;
	    CASE_16 (0x490)
	      {
		// LDR Rd, [Rn], #
		int offset = opcode & 0xFFF;
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I;
		reg[dest].I = CPUReadMemoryTransition (address);
		if (dest != base)
		  reg[base].I += offset;
		if (dest == 15)
		  {
		    modified_pc = true;
		    branch_taken = true;
		    reg[15].I &= 0xFFFFFFFC;
		    armNextPC = reg[15].I;
		  }
	      }
	    break;
	    CASE_16 (0x4b0)
	      {
		// LDRT Rd, [Rn], #
		int offset = opcode & 0xFFF;
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I;
		reg[dest].I = CPUReadMemoryTransition (address);
		if (dest != base)
		  reg[base].I += offset;
	      }
	    break;
	    CASE_16 (0x510)
	      {
		// LDR Rd, [Rn, -#]
		int offset = opcode & 0xFFF;
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I - offset;
		reg[dest].I = CPUReadMemoryTransition (address);
		if (dest == 15)
		  {
		    modified_pc = true;
		    branch_taken = true;
		    reg[15].I &= 0xFFFFFFFC;
		    armNextPC = reg[15].I;
		  }
	      }
	    break;
	    CASE_16 (0x530)
	      {
		// LDR Rd, [Rn, -#]!
		int offset = opcode & 0xFFF;
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I - offset;
		reg[dest].I = CPUReadMemoryTransition (address);
		if (dest != base)
		  reg[base].I = address;
		if (dest == 15)
		  {
		    modified_pc = true;
		    branch_taken = true;
		    reg[15].I &= 0xFFFFFFFC;
		    armNextPC = reg[15].I;
		  }
	      }
	    break;
	    CASE_16 (0x590)
	      {
		// LDR Rd, [Rn, #]
		int offset = opcode & 0xFFF;
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I + offset;
		reg[dest].I = CPUReadMemoryTransition (address);
		if (dest == 15)
		  {
		    modified_pc = true;
		    branch_taken = true;
		    reg[15].I &= 0xFFFFFFFC;
		    armNextPC = reg[15].I;
		  }
	      }
	    break;
	    CASE_16 (0x5b0)
	      {
		// LDR Rd, [Rn, #]!
		int offset = opcode & 0xFFF;
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I + offset;
		reg[dest].I = CPUReadMemoryTransition (address);
		if (dest != base)
		  reg[base].I = address;
		if (dest == 15)
		  {
		    modified_pc = true;
		    branch_taken = true;
		    reg[15].I &= 0xFFFFFFFC;
		    armNextPC = reg[15].I;
		  }
	      }
	    break;
	    CASE_16 (0x440)
	      // T versions shouldn't be different on GBA      
	      CASE_16 (0x460)
	      {
		// STRB Rd, [Rn], -#
		int offset = opcode & 0xFFF;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I;
		reg[base].I = address - offset;
	      }
	    break;
	    CASE_16 (0x4c0)
	      // T versions shouldn't be different on GBA
	      CASE_16 (0x4e0)
	      // STRB Rd, [Rn], #
	      {
		int offset = opcode & 0xFFF;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I;
		reg[base].I = address + offset;
	      }
	    break;
	    CASE_16 (0x540)
	      {
		// STRB Rd, [Rn, -#]
	      }
	    break;
	    CASE_16 (0x560)
	      {
		// STRB Rd, [Rn, -#]!
		int offset = opcode & 0xFFF;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I - offset;
		reg[base].I = address;
	      }
	    break;
	    CASE_16 (0x5c0)
	      {
		// STRB Rd, [Rn, #]
	      }
	    break;
	    CASE_16 (0x5e0)
	      {
		// STRB Rd, [Rn, #]!
		int offset = opcode & 0xFFF;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I + offset;
		reg[base].I = address;
	      }
	    break;
	    CASE_16 (0x450)
	      // T versions shouldn't be different
	      CASE_16 (0x470)
	      {
		// LDRB Rd, [Rn], -#
		int offset = opcode & 0xFFF;
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I;
		reg[dest].I = CPUReadByteTransition (address);
		if (dest != base)
		  reg[base].I -= offset;
	      }
	    break;
	    CASE_16 (0x4d0) CASE_16 (0x4f0)	// T versions should not be different
	      {
		// LDRB Rd, [Rn], #
		int offset = opcode & 0xFFF;
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I;
		reg[dest].I = CPUReadByteTransition (address);
		if (dest != base)
		  reg[base].I += offset;
	      }
	    break;
	    CASE_16 (0x550)
	      {
		// LDRB Rd, [Rn, -#]
		int offset = opcode & 0xFFF;
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I - offset;
		reg[dest].I = CPUReadByteTransition (address);
	      }
	    break;
	    CASE_16 (0x570)
	      {
		// LDRB Rd, [Rn, -#]!
		int offset = opcode & 0xFFF;
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I - offset;
		reg[dest].I = CPUReadByteTransition (address);
		if (dest != base)
		  reg[base].I = address;
	      }
	    break;
	    CASE_16 (0x5d0)
	      {
		// LDRB Rd, [Rn, #]
		int offset = opcode & 0xFFF;
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I + offset;
		reg[dest].I = CPUReadByteTransition (address);
	      }
	    break;
	    CASE_16 (0x5f0)
	      {
		// LDRB Rd, [Rn, #]!
		int offset = opcode & 0xFFF;
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I + offset;
		reg[dest].I = CPUReadByteTransition (address);
		if (dest != base)
		  reg[base].I = address;
	      }
	    break;
	  case 0x600:
	  case 0x608:
	    // T versions are the same
	  case 0x620:
	  case 0x628:
	    {
	      // STR Rd, [Rn], -Rm, LSL #
	      int offset = reg[opcode & 15].I << ((opcode >> 7) & 31);
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I;
	      reg[base].I = address - offset;
	    }
	    break;
	  case 0x602:
	  case 0x60a:
	    // T versions are the same
	  case 0x622:
	  case 0x62a:
	    {
	      // STR Rd, [Rn], -Rm, LSR #
	      int shift = (opcode >> 7) & 31;
	      int offset = shift ? reg[opcode & 15].I >> shift : 0;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I;
	      reg[base].I = address - offset;
	    }
	    break;
	  case 0x604:
	  case 0x60c:
	    // T versions are the same
	  case 0x624:
	  case 0x62c:
	    {
	      // STR Rd, [Rn], -Rm, ASR #
	      int shift = (opcode >> 7) & 31;
	      int offset;
	      if (shift)
		offset = (int) ((signed int) reg[opcode & 15].I >> shift);
	      else if (reg[opcode & 15].I & 0x80000000)
		offset = 0xFFFFFFFF;
	      else
		offset = 0;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I;
	      reg[base].I = address - offset;
	    }
	    break;
	  case 0x606:
	  case 0x60e:
	    // T versions are the same
	  case 0x626:
	  case 0x62e:
	    {
	      // STR Rd, [Rn], -Rm, ROR #
	      int shift = (opcode >> 7) & 31;
	      unsigned int value = reg[opcode & 15].I;
	      if (shift)
		{
		  ROR_VALUE;
		}
	      else
		{
		  RCR_VALUE;
		}
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I;
	      reg[base].I = address - value;
	    }
	    break;
	  case 0x680:
	  case 0x688:
	    // T versions are the same
	  case 0x6a0:
	  case 0x6a8:
	    {
	      // STR Rd, [Rn], Rm, LSL #
	      int offset = reg[opcode & 15].I << ((opcode >> 7) & 31);
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I;
	      reg[base].I = address + offset;
	    }
	    break;
	  case 0x682:
	  case 0x68a:
	    // T versions are the same
	  case 0x6a2:
	  case 0x6aa:
	    {
	      // STR Rd, [Rn], Rm, LSR #
	      int shift = (opcode >> 7) & 31;
	      int offset = shift ? reg[opcode & 15].I >> shift : 0;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I;
	      reg[base].I = address + offset;
	    }
	    break;
	  case 0x684:
	  case 0x68c:
	    // T versions are the same
	  case 0x6a4:
	  case 0x6ac:
	    {
	      // STR Rd, [Rn], Rm, ASR #
	      int shift = (opcode >> 7) & 31;
	      int offset;
	      if (shift)
		offset = (int) ((signed int) reg[opcode & 15].I >> shift);
	      else if (reg[opcode & 15].I & 0x80000000)
		offset = 0xFFFFFFFF;
	      else
		offset = 0;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I;
	      reg[base].I = address + offset;
	    }
	    break;
	  case 0x686:
	  case 0x68e:
	    // T versions are the same
	  case 0x6a6:
	  case 0x6ae:
	    {
	      // STR Rd, [Rn], Rm, ROR #
	      int shift = (opcode >> 7) & 31;
	      unsigned int value = reg[opcode & 15].I;
	      if (shift)
		{
		  ROR_VALUE;
		}
	      else
		{
		  RCR_VALUE;
		}
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I;
	      reg[base].I = address + value;
	    }
	    break;
	  case 0x700:
	  case 0x708:
	    {
	      // STR Rd, [Rn, -Rm, LSL #]
	    }
	    break;
	  case 0x702:
	  case 0x70a:
	    {
	      // STR Rd, [Rn, -Rm, LSR #]
	    }
	    break;
	  case 0x704:
	  case 0x70c:
	    {
	      // STR Rd, [Rn, -Rm, ASR #]
	    }
	    break;
	  case 0x706:
	  case 0x70e:
	    {
	      // STR Rd, [Rn, -Rm, ROR #]
	    }
	    break;
	  case 0x720:
	  case 0x728:
	    {
	      // STR Rd, [Rn, -Rm, LSL #]!
	      int offset = reg[opcode & 15].I << ((opcode >> 7) & 31);
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I - offset;
	      reg[base].I = address;
	    }
	    break;
	  case 0x722:
	  case 0x72a:
	    {
	      // STR Rd, [Rn, -Rm, LSR #]!
	      int shift = (opcode >> 7) & 31;
	      int offset = shift ? reg[opcode & 15].I >> shift : 0;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I - offset;
	      reg[base].I = address;
	    }
	    break;
	  case 0x724:
	  case 0x72c:
	    {
	      // STR Rd, [Rn, -Rm, ASR #]!
	      int shift = (opcode >> 7) & 31;
	      int offset;
	      if (shift)
		offset = (int) ((signed int) reg[opcode & 15].I >> shift);
	      else if (reg[opcode & 15].I & 0x80000000)
		offset = 0xFFFFFFFF;
	      else
		offset = 0;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I - offset;
	      reg[base].I = address;
	    }
	    break;
	  case 0x726:
	  case 0x72e:
	    {
	      // STR Rd, [Rn, -Rm, ROR #]!
	      int shift = (opcode >> 7) & 31;
	      unsigned int value = reg[opcode & 15].I;
	      if (shift)
		{
		  ROR_VALUE;
		}
	      else
		{
		  RCR_VALUE;
		}
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I - value;
	      reg[base].I = address;
	    }
	    break;
	  case 0x780:
	  case 0x788:
	    {
	      // STR Rd, [Rn, Rm, LSL #]
	    }
	    break;
	  case 0x782:
	  case 0x78a:
	    {
	      // STR Rd, [Rn, Rm, LSR #]
	    }
	    break;
	  case 0x784:
	  case 0x78c:
	    {
	      // STR Rd, [Rn, Rm, ASR #]
	    }
	    break;
	  case 0x786:
	  case 0x78e:
	    {
	      // STR Rd, [Rn, Rm, ROR #]
	    }
	    break;
	  case 0x7a0:
	  case 0x7a8:
	    {
	      // STR Rd, [Rn, Rm, LSL #]!
	      int offset = reg[opcode & 15].I << ((opcode >> 7) & 31);
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I + offset;
	      reg[base].I = address;
	    }
	    break;
	  case 0x7a2:
	  case 0x7aa:
	    {
	      // STR Rd, [Rn, Rm, LSR #]!
	      int shift = (opcode >> 7) & 31;
	      int offset = shift ? reg[opcode & 15].I >> shift : 0;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I + offset;
	      reg[base].I = address;
	    }
	    break;
	  case 0x7a4:
	  case 0x7ac:
	    {
	      // STR Rd, [Rn, Rm, ASR #]!
	      int shift = (opcode >> 7) & 31;
	      int offset;
	      if (shift)
		offset = (int) ((signed int) reg[opcode & 15].I >> shift);
	      else if (reg[opcode & 15].I & 0x80000000)
		offset = 0xFFFFFFFF;
	      else
		offset = 0;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I + offset;
	      reg[base].I = address;
	    }
	    break;
	  case 0x7a6:
	  case 0x7ae:
	    {
	      // STR Rd, [Rn, Rm, ROR #]!
	      int shift = (opcode >> 7) & 31;
	      unsigned int value = reg[opcode & 15].I;
	      if (shift)
		{
		  ROR_VALUE;
		}
	      else
		{
		  RCR_VALUE;
		}
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I + value;
	      reg[base].I = address;
	    }
	    break;
	  case 0x610:
	  case 0x618:
	    // T versions are the same
	  case 0x630:
	  case 0x638:
	    {
	      // LDR Rd, [Rn], -Rm, LSL #
	      int offset = reg[opcode & 15].I << ((opcode >> 7) & 31);
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I;
	      reg[dest].I = CPUReadMemoryTransition (address);
	      if (dest != base)
		reg[base].I = address - offset;
	      if (dest == 15)
		{
		  modified_pc = true;
		  branch_taken = true;
		  reg[15].I &= 0xFFFFFFFC;
		  armNextPC = reg[15].I;
		}
	    }
	    break;
	  case 0x612:
	  case 0x61a:
	    // T versions are the same
	  case 0x632:
	  case 0x63a:
	    {
	      // LDR Rd, [Rn], -Rm, LSR #
	      int shift = (opcode >> 7) & 31;
	      int offset = shift ? reg[opcode & 15].I >> shift : 0;
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I;
	      reg[dest].I = CPUReadMemoryTransition (address);
	      if (dest != base)
		reg[base].I = address - offset;
	      if (dest == 15)
		{
		  modified_pc = true;
		  branch_taken = true;
		  reg[15].I &= 0xFFFFFFFC;
		  armNextPC = reg[15].I;
		}
	    }
	    break;
	  case 0x614:
	  case 0x61c:
	    // T versions are the same
	  case 0x634:
	  case 0x63c:
	    {
	      // LDR Rd, [Rn], -Rm, ASR #
	      int shift = (opcode >> 7) & 31;
	      int offset;
	      if (shift)
		offset = (int) ((signed int) reg[opcode & 15].I >> shift);
	      else if (reg[opcode & 15].I & 0x80000000)
		offset = 0xFFFFFFFF;
	      else
		offset = 0;
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I;
	      reg[dest].I = CPUReadMemoryTransition (address);
	      if (dest != base)
		reg[base].I = address - offset;
	      if (dest == 15)
		{
		  modified_pc = true;
		  branch_taken = true;
		  reg[15].I &= 0xFFFFFFFC;
		  armNextPC = reg[15].I;
		}
	    }
	    break;
	  case 0x616:
	  case 0x61e:
	    // T versions are the same
	  case 0x636:
	  case 0x63e:
	    {
	      // LDR Rd, [Rn], -Rm, ROR #
	      int shift = (opcode >> 7) & 31;
	      unsigned int value = reg[opcode & 15].I;
	      if (shift)
		{
		  ROR_VALUE;
		}
	      else
		{
		  RCR_VALUE;
		}
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I;
	      reg[dest].I = CPUReadMemoryTransition (address);
	      if (dest != base)
		reg[base].I = address - value;
	      if (dest == 15)
		{
		  modified_pc = true;
		  branch_taken = true;
		  reg[15].I &= 0xFFFFFFFC;
		  armNextPC = reg[15].I;
		}
	    }
	    break;
	  case 0x690:
	  case 0x698:
	    // T versions are the same
	  case 0x6b0:
	  case 0x6b8:
	    {
	      // LDR Rd, [Rn], Rm, LSL #
	      int offset = reg[opcode & 15].I << ((opcode >> 7) & 31);
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I;
	      reg[dest].I = CPUReadMemoryTransition (address);
	      if (dest != base)
		reg[base].I = address + offset;
	      if (dest == 15)
		{
		  modified_pc = true;
		  branch_taken = true;
		  reg[15].I &= 0xFFFFFFFC;
		  armNextPC = reg[15].I;
		}
	    }
	    break;
	  case 0x692:
	  case 0x69a:
	    // T versions are the same
	  case 0x6b2:
	  case 0x6ba:
	    {
	      // LDR Rd, [Rn], Rm, LSR #
	      int shift = (opcode >> 7) & 31;
	      int offset = shift ? reg[opcode & 15].I >> shift : 0;
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I;
	      reg[dest].I = CPUReadMemoryTransition (address);
	      if (dest != base)
		reg[base].I = address + offset;
	      if (dest == 15)
		{
		  modified_pc = true;
		  branch_taken = true;
		  reg[15].I &= 0xFFFFFFFC;
		  armNextPC = reg[15].I;
		}
	    }
	    break;
	  case 0x694:
	  case 0x69c:
	    // T versions are the same
	  case 0x6b4:
	  case 0x6bc:
	    {
	      // LDR Rd, [Rn], Rm, ASR #
	      int shift = (opcode >> 7) & 31;
	      int offset;
	      if (shift)
		offset = (int) ((signed int) reg[opcode & 15].I >> shift);
	      else if (reg[opcode & 15].I & 0x80000000)
		offset = 0xFFFFFFFF;
	      else
		offset = 0;
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I;
	      reg[dest].I = CPUReadMemoryTransition (address);
	      if (dest != base)
		reg[base].I = address + offset;
	      if (dest == 15)
		{
		  modified_pc = true;
		  branch_taken = true;
		  reg[15].I &= 0xFFFFFFFC;
		  armNextPC = reg[15].I;
		}
	    }
	    break;
	  case 0x696:
	  case 0x69e:
	    // T versions are the same
	  case 0x6b6:
	  case 0x6be:
	    {
	      // LDR Rd, [Rn], Rm, ROR #
	      int shift = (opcode >> 7) & 31;
	      unsigned int value = reg[opcode & 15].I;
	      if (shift)
		{
		  ROR_VALUE;
		}
	      else
		{
		  RCR_VALUE;
		}
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I;
	      reg[dest].I = CPUReadMemoryTransition (address);
	      if (dest != base)
		reg[base].I = address + value;
	      if (dest == 15)
		{
		  modified_pc = true;
		  branch_taken = true;
		  reg[15].I &= 0xFFFFFFFC;
		  armNextPC = reg[15].I;
		}
	    }
	    break;
	  case 0x710:
	  case 0x718:
	    {
	      // LDR Rd, [Rn, -Rm, LSL #]
	      int offset = reg[opcode & 15].I << ((opcode >> 7) & 31);
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I - offset;
	      reg[dest].I = CPUReadMemoryTransition (address);
	      if (dest == 15)
		{
		  modified_pc = true;
		  branch_taken = true;
		  reg[15].I &= 0xFFFFFFFC;
		  armNextPC = reg[15].I;
		}
	    }
	    break;
	  case 0x712:
	  case 0x71a:
	    {
	      // LDR Rd, [Rn, -Rm, LSR #]
	      int shift = (opcode >> 7) & 31;
	      int offset = shift ? reg[opcode & 15].I >> shift : 0;
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I - offset;
	      reg[dest].I = CPUReadMemoryTransition (address);
	      if (dest == 15)
		{
		  modified_pc = true;
		  branch_taken = true;
		  reg[15].I &= 0xFFFFFFFC;
		  armNextPC = reg[15].I;
		}
	    }
	    break;
	  case 0x714:
	  case 0x71c:
	    {
	      // LDR Rd, [Rn, -Rm, ASR #]
	      int shift = (opcode >> 7) & 31;
	      int offset;
	      if (shift)
		offset = (int) ((signed int) reg[opcode & 15].I >> shift);
	      else if (reg[opcode & 15].I & 0x80000000)
		offset = 0xFFFFFFFF;
	      else
		offset = 0;
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I - offset;
	      reg[dest].I = CPUReadMemoryTransition (address);
	      if (dest == 15)
		{
		  modified_pc = true;
		  branch_taken = true;
		  reg[15].I &= 0xFFFFFFFC;
		  armNextPC = reg[15].I;
		}
	    }
	    break;
	  case 0x716:
	  case 0x71e:
	    {
	      // LDR Rd, [Rn, -Rm, ROR #]
	      int shift = (opcode >> 7) & 31;
	      unsigned int value = reg[opcode & 15].I;
	      if (shift)
		{
		  ROR_VALUE;
		}
	      else
		{
		  RCR_VALUE;
		}
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I - value;
	      reg[dest].I = CPUReadMemoryTransition (address);
	      if (dest == 15)
		{
		  modified_pc = true;
		  branch_taken = true;
		  reg[15].I &= 0xFFFFFFFC;
		  armNextPC = reg[15].I;
		}
	    }
	    break;
	  case 0x730:
	  case 0x738:
	    {
	      // LDR Rd, [Rn, -Rm, LSL #]!
	      int offset = reg[opcode & 15].I << ((opcode >> 7) & 31);
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I - offset;
	      reg[dest].I = CPUReadMemoryTransition (address);
	      if (dest != base)
		reg[base].I = address;
	      if (dest == 15)
		{
		  modified_pc = true;
		  branch_taken = true;
		  reg[15].I &= 0xFFFFFFFC;
		  armNextPC = reg[15].I;
		}
	    }
	    break;
	  case 0x732:
	  case 0x73a:
	    {
	      // LDR Rd, [Rn, -Rm, LSR #]!
	      int shift = (opcode >> 7) & 31;
	      int offset = shift ? reg[opcode & 15].I >> shift : 0;
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I - offset;
	      reg[dest].I = CPUReadMemoryTransition (address);
	      if (dest != base)
		reg[base].I = address;
	      if (dest == 15)
		{
		  modified_pc = true;
		  branch_taken = true;
		  reg[15].I &= 0xFFFFFFFC;
		  armNextPC = reg[15].I;
		}
	    }
	    break;
	  case 0x734:
	  case 0x73c:
	    {
	      // LDR Rd, [Rn, -Rm, ASR #]!
	      int shift = (opcode >> 7) & 31;
	      int offset;
	      if (shift)
		offset = (int) ((signed int) reg[opcode & 15].I >> shift);
	      else if (reg[opcode & 15].I & 0x80000000)
		offset = 0xFFFFFFFF;
	      else
		offset = 0;
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I - offset;
	      reg[dest].I = CPUReadMemoryTransition (address);
	      if (dest != base)
		reg[base].I = address;
	      if (dest == 15)
		{
		  modified_pc = true;
		  branch_taken = true;
		  reg[15].I &= 0xFFFFFFFC;
		  armNextPC = reg[15].I;
		}
	    }
	    break;
	  case 0x736:
	  case 0x73e:
	    {
	      // LDR Rd, [Rn, -Rm, ROR #]!
	      int shift = (opcode >> 7) & 31;
	      unsigned int value = reg[opcode & 15].I;
	      if (shift)
		{
		  ROR_VALUE;
		}
	      else
		{
		  RCR_VALUE;
		}
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I - value;
	      reg[dest].I = CPUReadMemoryTransition (address);
	      if (dest != base)
		reg[base].I = address;
	      if (dest == 15)
		{
		  modified_pc = true;
		  branch_taken = true;
		  reg[15].I &= 0xFFFFFFFC;
		  armNextPC = reg[15].I;
		}
	    }
	    break;
	  case 0x790:
	  case 0x798:
	    {
	      // LDR Rd, [Rn, Rm, LSL #]
	      int offset = reg[opcode & 15].I << ((opcode >> 7) & 31);
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I + offset;
	      reg[dest].I = CPUReadMemoryTransition (address);
	      if (dest == 15)
		{
		  modified_pc = true;
		  branch_taken = true;
		  reg[15].I &= 0xFFFFFFFC;
		  armNextPC = reg[15].I;
		}
	    }
	    break;
	  case 0x792:
	  case 0x79a:
	    {
	      // LDR Rd, [Rn, Rm, LSR #]
	      int shift = (opcode >> 7) & 31;
	      int offset = shift ? reg[opcode & 15].I >> shift : 0;
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I + offset;
	      reg[dest].I = CPUReadMemoryTransition (address);
	      if (dest == 15)
		{
		  modified_pc = true;
		  branch_taken = true;
		  reg[15].I &= 0xFFFFFFFC;
		  armNextPC = reg[15].I;
		}
	    }
	    break;
	  case 0x794:
	  case 0x79c:
	    {
	      // LDR Rd, [Rn, Rm, ASR #]
	      int shift = (opcode >> 7) & 31;
	      int offset;
	      if (shift)
		offset = (int) ((signed int) reg[opcode & 15].I >> shift);
	      else if (reg[opcode & 15].I & 0x80000000)
		offset = 0xFFFFFFFF;
	      else
		offset = 0;
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I + offset;
	      reg[dest].I = CPUReadMemoryTransition (address);
	      if (dest == 15)
		{
		  modified_pc = true;
		  branch_taken = true;
		  reg[15].I &= 0xFFFFFFFC;
		  armNextPC = reg[15].I;
		}
	    }
	    break;
	  case 0x796:
	  case 0x79e:
	    {
	      // LDR Rd, [Rn, Rm, ROR #]
	      int shift = (opcode >> 7) & 31;
	      unsigned int value = reg[opcode & 15].I;
	      if (shift)
		{
		  ROR_VALUE;
		}
	      else
		{
		  RCR_VALUE;
		}
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I + value;
	      reg[dest].I = CPUReadMemoryTransition (address);
	      if (dest == 15)
		{
		  modified_pc = true;
		  branch_taken = true;
		  reg[15].I &= 0xFFFFFFFC;
		  armNextPC = reg[15].I;
		}
	    }
	    break;
	  case 0x7b0:
	  case 0x7b8:
	    {
	      // LDR Rd, [Rn, Rm, LSL #]!
	      int offset = reg[opcode & 15].I << ((opcode >> 7) & 31);
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I + offset;
	      reg[dest].I = CPUReadMemoryTransition (address);
	      if (dest != base)
		reg[base].I = address;
	      if (dest == 15)
		{
		  modified_pc = true;
		  branch_taken = true;
		  reg[15].I &= 0xFFFFFFFC;
		  armNextPC = reg[15].I;
		}
	    }
	    break;
	  case 0x7b2:
	  case 0x7ba:
	    {
	      // LDR Rd, [Rn, Rm, LSR #]!
	      int shift = (opcode >> 7) & 31;
	      int offset = shift ? reg[opcode & 15].I >> shift : 0;
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I + offset;
	      reg[dest].I = CPUReadMemoryTransition (address);
	      if (dest != base)
		reg[base].I = address;
	      if (dest == 15)
		{
		  modified_pc = true;
		  branch_taken = true;
		  reg[15].I &= 0xFFFFFFFC;
		  armNextPC = reg[15].I;
		}
	    }
	    break;
	  case 0x7b4:
	  case 0x7bc:
	    {
	      // LDR Rd, [Rn, Rm, ASR #]!
	      int shift = (opcode >> 7) & 31;
	      int offset;
	      if (shift)
		offset = (int) ((signed int) reg[opcode & 15].I >> shift);
	      else if (reg[opcode & 15].I & 0x80000000)
		offset = 0xFFFFFFFF;
	      else
		offset = 0;
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I + offset;
	      reg[dest].I = CPUReadMemoryTransition (address);
	      if (dest != base)
		reg[base].I = address;
	      if (dest == 15)
		{
		  modified_pc = true;
		  branch_taken = true;
		  reg[15].I &= 0xFFFFFFFC;
		  armNextPC = reg[15].I;
		}
	    }
	    break;
	  case 0x7b6:
	  case 0x7be:
	    {
	      // LDR Rd, [Rn, Rm, ROR #]!
	      int shift = (opcode >> 7) & 31;
	      unsigned int value = reg[opcode & 15].I;
	      if (shift)
		{
		  ROR_VALUE;
		}
	      else
		{
		  RCR_VALUE;
		}
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I + value;
	      reg[dest].I = CPUReadMemoryTransition (address);
	      if (dest != base)
		reg[base].I = address;
	      if (dest == 15)
		{
		  modified_pc = true;
		  branch_taken = true;
		  reg[15].I &= 0xFFFFFFFC;
		  armNextPC = reg[15].I;
		}
	    }
	    break;
	  case 0x640:
	  case 0x648:
	    // T versions are the same
	  case 0x660:
	  case 0x668:
	    {
	      // STRB Rd, [Rn], -Rm, LSL #
	      int offset = reg[opcode & 15].I << ((opcode >> 7) & 31);
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I;
	      reg[base].I = address - offset;
	    }
	    break;
	  case 0x642:
	  case 0x64a:
	    // T versions are the same
	  case 0x662:
	  case 0x66a:
	    {
	      // STRB Rd, [Rn], -Rm, LSR #
	      int shift = (opcode >> 7) & 31;
	      int offset = shift ? reg[opcode & 15].I >> shift : 0;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I;
	      reg[base].I = address - offset;
	    }
	    break;
	  case 0x644:
	  case 0x64c:
	    // T versions are the same
	  case 0x664:
	  case 0x66c:
	    {
	      // STRB Rd, [Rn], -Rm, ASR #
	      int shift = (opcode >> 7) & 31;
	      int offset;
	      if (shift)
		offset = (int) ((signed int) reg[opcode & 15].I >> shift);
	      else if (reg[opcode & 15].I & 0x80000000)
		offset = 0xFFFFFFFF;
	      else
		offset = 0;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I;
	      reg[base].I = address - offset;
	    }
	    break;
	  case 0x646:
	  case 0x64e:
	    // T versions are the same
	  case 0x666:
	  case 0x66e:
	    {
	      // STRB Rd, [Rn], -Rm, ROR #
	      int shift = (opcode >> 7) & 31;
	      unsigned int value = reg[opcode & 15].I;
	      if (shift)
		{
		  ROR_VALUE;
		}
	      else
		{
		  RCR_VALUE;
		}
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I;
	      reg[base].I = address - value;
	    }
	    break;
	  case 0x6c0:
	  case 0x6c8:
	    // T versions are the same
	  case 0x6e0:
	  case 0x6e8:
	    {
	      // STRB Rd, [Rn], Rm, LSL #
	      int offset = reg[opcode & 15].I << ((opcode >> 7) & 31);
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I;
	      reg[base].I = address + offset;
	    }
	    break;
	  case 0x6c2:
	  case 0x6ca:
	    // T versions are the same
	  case 0x6e2:
	  case 0x6ea:
	    {
	      // STRB Rd, [Rn], Rm, LSR #
	      int shift = (opcode >> 7) & 31;
	      int offset = shift ? reg[opcode & 15].I >> shift : 0;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I;
	      reg[base].I = address + offset;
	    }
	    break;
	  case 0x6c4:
	  case 0x6cc:
	    // T versions are the same
	  case 0x6e4:
	  case 0x6ec:
	    {
	      // STR Rd, [Rn], Rm, ASR #
	      int shift = (opcode >> 7) & 31;
	      int offset;
	      if (shift)
		offset = (int) ((signed int) reg[opcode & 15].I >> shift);
	      else if (reg[opcode & 15].I & 0x80000000)
		offset = 0xFFFFFFFF;
	      else
		offset = 0;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I;
	      reg[base].I = address + offset;
	    }
	    break;
	  case 0x6c6:
	  case 0x6ce:
	    // T versions are the same
	  case 0x6e6:
	  case 0x6ee:
	    {
	      // STRB Rd, [Rn], Rm, ROR #
	      int shift = (opcode >> 7) & 31;
	      unsigned int value = reg[opcode & 15].I;
	      if (shift)
		{
		  ROR_VALUE;
		}
	      else
		{
		  RCR_VALUE;
		}
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I;
	      reg[base].I = address + value;
	    }
	    break;
	  case 0x740:
	  case 0x748:
	    {
	      // STRB Rd, [Rn, -Rm, LSL #]
	    }
	    break;
	  case 0x742:
	  case 0x74a:
	    {
	      // STRB Rd, [Rn, -Rm, LSR #]
	    }
	    break;
	  case 0x744:
	  case 0x74c:
	    {
	      // STRB Rd, [Rn, -Rm, ASR #]
	    }
	    break;
	  case 0x746:
	  case 0x74e:
	    {
	      // STRB Rd, [Rn, -Rm, ROR #]
	    }
	    break;
	  case 0x760:
	  case 0x768:
	    {
	      // STRB Rd, [Rn, -Rm, LSL #]!
	      int offset = reg[opcode & 15].I << ((opcode >> 7) & 31);
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I - offset;
	      reg[base].I = address;
	    }
	    break;
	  case 0x762:
	  case 0x76a:
	    {
	      // STRB Rd, [Rn, -Rm, LSR #]!
	      int shift = (opcode >> 7) & 31;
	      int offset = shift ? reg[opcode & 15].I >> shift : 0;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I - offset;
	      reg[base].I = address;
	    }
	    break;
	  case 0x764:
	  case 0x76c:
	    {
	      // STRB Rd, [Rn, -Rm, ASR #]!
	      int shift = (opcode >> 7) & 31;
	      int offset;
	      if (shift)
		offset = (int) ((signed int) reg[opcode & 15].I >> shift);
	      else if (reg[opcode & 15].I & 0x80000000)
		offset = 0xFFFFFFFF;
	      else
		offset = 0;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I - offset;
	      reg[base].I = address;
	    }
	    break;
	  case 0x766:
	  case 0x76e:
	    {
	      // STRB Rd, [Rn, -Rm, ROR #]!
	      int shift = (opcode >> 7) & 31;
	      unsigned int value = reg[opcode & 15].I;
	      if (shift)
		{
		  ROR_VALUE;
		}
	      else
		{
		  RCR_VALUE;
		}
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I - value;
	      reg[base].I = address;
	    }
	    break;
	  case 0x7c0:
	  case 0x7c8:
	    {
	      // STRB Rd, [Rn, Rm, LSL #]
	    }
	    break;
	  case 0x7c2:
	  case 0x7ca:
	    {
	      // STRB Rd, [Rn, Rm, LSR #]
	    }
	    break;
	  case 0x7c4:
	  case 0x7cc:
	    {
	      // STRB Rd, [Rn, Rm, ASR #]
	    }
	    break;
	  case 0x7c6:
	  case 0x7ce:
	    {
	      // STRB Rd, [Rn, Rm, ROR #]
	    }
	    break;
	  case 0x7e0:
	  case 0x7e8:
	    {
	      // STRB Rd, [Rn, Rm, LSL #]!
	      int offset = reg[opcode & 15].I << ((opcode >> 7) & 31);
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I + offset;
	      reg[base].I = address;
	    }
	    break;
	  case 0x7e2:
	  case 0x7ea:
	    {
	      // STRB Rd, [Rn, Rm, LSR #]!
	      int shift = (opcode >> 7) & 31;
	      int offset = shift ? reg[opcode & 15].I >> shift : 0;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I + offset;
	      reg[base].I = address;
	    }
	    break;
	  case 0x7e4:
	  case 0x7ec:
	    {
	      // STRB Rd, [Rn, Rm, ASR #]!
	      int shift = (opcode >> 7) & 31;
	      int offset;
	      if (shift)
		offset = (int) ((signed int) reg[opcode & 15].I >> shift);
	      else if (reg[opcode & 15].I & 0x80000000)
		offset = 0xFFFFFFFF;
	      else
		offset = 0;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I + offset;
	      reg[base].I = address;
	    }
	    break;
	  case 0x7e6:
	  case 0x7ee:
	    {
	      // STRB Rd, [Rn, Rm, ROR #]!
	      int shift = (opcode >> 7) & 31;
	      unsigned int value = reg[opcode & 15].I;
	      if (shift)
		{
		  ROR_VALUE;
		}
	      else
		{
		  RCR_VALUE;
		}
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I + value;
	      reg[base].I = address;
	    }
	    break;
	  case 0x650:
	  case 0x658:
	    // T versions are the same
	  case 0x670:
	  case 0x678:
	    {
	      // LDRB Rd, [Rn], -Rm, LSL #
	      int offset = reg[opcode & 15].I << ((opcode >> 7) & 31);
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I;
	      reg[dest].I = CPUReadByteTransition (address);
	      if (dest != base)
		reg[base].I = address - offset;
	    }
	    break;
	  case 0x652:
	  case 0x65a:
	    // T versions are the same
	  case 0x672:
	  case 0x67a:
	    {
	      // LDRB Rd, [Rn], -Rm, LSR #
	      int shift = (opcode >> 7) & 31;
	      int offset = shift ? reg[opcode & 15].I >> shift : 0;
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I;
	      reg[dest].I = CPUReadByteTransition (address);
	      if (dest != base)
		reg[base].I = address - offset;
	    }
	    break;
	  case 0x654:
	  case 0x65c:
	    // T versions are the same
	  case 0x674:
	  case 0x67c:
	    {
	      // LDRB Rd, [Rn], -Rm, ASR #
	      int shift = (opcode >> 7) & 31;
	      int offset;
	      if (shift)
		offset = (int) ((signed int) reg[opcode & 15].I >> shift);
	      else if (reg[opcode & 15].I & 0x80000000)
		offset = 0xFFFFFFFF;
	      else
		offset = 0;
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I;
	      reg[dest].I = CPUReadByteTransition (address);
	      if (dest != base)
		reg[base].I = address - offset;
	    }
	    break;
	  case 0x656:
	  case 0x65e:
	    // T versions are the same
	  case 0x676:
	  case 0x67e:
	    {
	      // LDRB Rd, [Rn], -Rm, ROR #
	      int shift = (opcode >> 7) & 31;
	      unsigned int value = reg[opcode & 15].I;
	      if (shift)
		{
		  ROR_VALUE;
		}
	      else
		{
		  RCR_VALUE;
		}
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I;
	      reg[dest].I = CPUReadByteTransition (address);
	      if (dest != base)
		reg[base].I = address - value;
	    }
	    break;
	  case 0x6d0:
	  case 0x6d8:
	    // T versions are the same
	  case 0x6f0:
	  case 0x6f8:
	    {
	      // LDRB Rd, [Rn], Rm, LSL #
	      int offset = reg[opcode & 15].I << ((opcode >> 7) & 31);
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I;
	      reg[dest].I = CPUReadByteTransition (address);
	      if (dest != base)
		reg[base].I = address + offset;
	    }
	    break;
	  case 0x6d2:
	  case 0x6da:
	    // T versions are the same
	  case 0x6f2:
	  case 0x6fa:
	    {
	      // LDRB Rd, [Rn], Rm, LSR #
	      int shift = (opcode >> 7) & 31;
	      int offset = shift ? reg[opcode & 15].I >> shift : 0;
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I;
	      reg[dest].I = CPUReadByteTransition (address);
	      if (dest != base)
		reg[base].I = address + offset;
	    }
	    break;
	  case 0x6d4:
	  case 0x6dc:
	    // T versions are the same
	  case 0x6f4:
	  case 0x6fc:
	    {
	      // LDRB Rd, [Rn], Rm, ASR #
	      int shift = (opcode >> 7) & 31;
	      int offset;
	      if (shift)
		offset = (int) ((signed int) reg[opcode & 15].I >> shift);
	      else if (reg[opcode & 15].I & 0x80000000)
		offset = 0xFFFFFFFF;
	      else
		offset = 0;
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I;
	      reg[dest].I = CPUReadByteTransition (address);
	      if (dest != base)
		reg[base].I = address + offset;
	    }
	    break;
	  case 0x6d6:
	  case 0x6de:
	    // T versions are the same
	  case 0x6f6:
	  case 0x6fe:
	    {
	      // LDRB Rd, [Rn], Rm, ROR #
	      int shift = (opcode >> 7) & 31;
	      unsigned int value = reg[opcode & 15].I;
	      if (shift)
		{
		  ROR_VALUE;
		}
	      else
		{
		  RCR_VALUE;
		}
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I;
	      reg[dest].I = CPUReadByteTransition (address);
	      if (dest != base)
		reg[base].I = address + value;
	    }
	    break;
	  case 0x750:
	  case 0x758:
	    {
	      // LDRB Rd, [Rn, -Rm, LSL #]
	      int offset = reg[opcode & 15].I << ((opcode >> 7) & 31);
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I - offset;
	      reg[dest].I = CPUReadByteTransition (address);
	    }
	    break;
	  case 0x752:
	  case 0x75a:
	    {
	      // LDRB Rd, [Rn, -Rm, LSR #]
	      int shift = (opcode >> 7) & 31;
	      int offset = shift ? reg[opcode & 15].I >> shift : 0;
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I - offset;
	      reg[dest].I = CPUReadByteTransition (address);
	    }
	    break;
	  case 0x754:
	  case 0x75c:
	    {
	      // LDRB Rd, [Rn, -Rm, ASR #]
	      int shift = (opcode >> 7) & 31;
	      int offset;
	      if (shift)
		offset = (int) ((signed int) reg[opcode & 15].I >> shift);
	      else if (reg[opcode & 15].I & 0x80000000)
		offset = 0xFFFFFFFF;
	      else
		offset = 0;
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I - offset;
	      reg[dest].I = CPUReadByteTransition (address);
	    }
	    break;
	  case 0x756:
	  case 0x75e:
	    {
	      // LDRB Rd, [Rn, -Rm, ROR #]
	      int shift = (opcode >> 7) & 31;
	      unsigned int value = reg[opcode & 15].I;
	      if (shift)
		{
		  ROR_VALUE;
		}
	      else
		{
		  RCR_VALUE;
		}
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I - value;
	      reg[dest].I = CPUReadByteTransition (address);
	    }
	    break;
	  case 0x770:
	  case 0x778:
	    {
	      // LDRB Rd, [Rn, -Rm, LSL #]!
	      int offset = reg[opcode & 15].I << ((opcode >> 7) & 31);
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I - offset;
	      reg[dest].I = CPUReadByteTransition (address);
	      if (dest != base)
		reg[base].I = address;
	    }
	    break;
	  case 0x772:
	  case 0x77a:
	    {
	      // LDRB Rd, [Rn, -Rm, LSR #]!
	      int shift = (opcode >> 7) & 31;
	      int offset = shift ? reg[opcode & 15].I >> shift : 0;
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I - offset;
	      reg[dest].I = CPUReadByteTransition (address);
	      if (dest != base)
		reg[base].I = address;
	    }
	    break;
	  case 0x774:
	  case 0x77c:
	    {
	      // LDRB Rd, [Rn, -Rm, ASR #]!
	      int shift = (opcode >> 7) & 31;
	      int offset;
	      if (shift)
		offset = (int) ((signed int) reg[opcode & 15].I >> shift);
	      else if (reg[opcode & 15].I & 0x80000000)
		offset = 0xFFFFFFFF;
	      else
		offset = 0;
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I - offset;
	      reg[dest].I = CPUReadByteTransition (address);
	      if (dest != base)
		reg[base].I = address;
	    }
	    break;
	  case 0x776:
	  case 0x77e:
	    {
	      // LDRB Rd, [Rn, -Rm, ROR #]!
	      int shift = (opcode >> 7) & 31;
	      unsigned int value = reg[opcode & 15].I;
	      if (shift)
		{
		  ROR_VALUE;
		}
	      else
		{
		  RCR_VALUE;
		}
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I - value;
	      reg[dest].I = CPUReadByteTransition (address);
	      if (dest != base)
		reg[base].I = address;
	    }
	    break;
	  case 0x7d0:
	  case 0x7d8:
	    {
	      // LDRB Rd, [Rn, Rm, LSL #]
	      int offset = reg[opcode & 15].I << ((opcode >> 7) & 31);
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I + offset;
	      reg[dest].I = CPUReadByteTransition (address);
	    }
	    break;
	  case 0x7d2:
	  case 0x7da:
	    {
	      // LDRB Rd, [Rn, Rm, LSR #]
	      int shift = (opcode >> 7) & 31;
	      int offset = shift ? reg[opcode & 15].I >> shift : 0;
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I + offset;
	      reg[dest].I = CPUReadByteTransition (address);
	    }
	    break;
	  case 0x7d4:
	  case 0x7dc:
	    {
	      // LDRB Rd, [Rn, Rm, ASR #]
	      int shift = (opcode >> 7) & 31;
	      int offset;
	      if (shift)
		offset = (int) ((signed int) reg[opcode & 15].I >> shift);
	      else if (reg[opcode & 15].I & 0x80000000)
		offset = 0xFFFFFFFF;
	      else
		offset = 0;
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I + offset;
	      reg[dest].I = CPUReadByteTransition (address);
	    }
	    break;
	  case 0x7d6:
	  case 0x7de:
	    {
	      // LDRB Rd, [Rn, Rm, ROR #]
	      int shift = (opcode >> 7) & 31;
	      unsigned int value = reg[opcode & 15].I;
	      if (shift)
		{
		  ROR_VALUE;
		}
	      else
		{
		  RCR_VALUE;
		}
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I + value;
	      reg[dest].I = CPUReadByteTransition (address);
	    }
	    break;
	  case 0x7f0:
	  case 0x7f8:
	    {
	      // LDRB Rd, [Rn, Rm, LSL #]!
	      int offset = reg[opcode & 15].I << ((opcode >> 7) & 31);
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I + offset;
	      reg[dest].I = CPUReadByteTransition (address);
	      if (dest != base)
		reg[base].I = address;
	    }
	    break;
	  case 0x7f2:
	  case 0x7fa:
	    {
	      // LDRB Rd, [Rn, Rm, LSR #]!
	      int shift = (opcode >> 7) & 31;
	      int offset = shift ? reg[opcode & 15].I >> shift : 0;
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I + offset;
	      reg[dest].I = CPUReadByteTransition (address);
	      if (dest != base)
		reg[base].I = address;
	    }
	    break;
	  case 0x7f4:
	  case 0x7fc:
	    {
	      // LDRB Rd, [Rn, Rm, ASR #]!
	      int shift = (opcode >> 7) & 31;
	      int offset;
	      if (shift)
		offset = (int) ((signed int) reg[opcode & 15].I >> shift);
	      else if (reg[opcode & 15].I & 0x80000000)
		offset = 0xFFFFFFFF;
	      else
		offset = 0;
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I + offset;
	      reg[dest].I = CPUReadByteTransition (address);
	      if (dest != base)
		reg[base].I = address;
	    }
	    break;
	  case 0x7f6:
	  case 0x7fe:
	    {
	      // LDRB Rd, [Rn, Rm, ROR #]!
	      int shift = (opcode >> 7) & 31;
	      unsigned int value = reg[opcode & 15].I;
	      if (shift)
		{
		  ROR_VALUE;
		}
	      else
		{
		  RCR_VALUE;
		}
	      int dest = (opcode >> 12) & 15;
	      int base = (opcode >> 16) & 15;
	      unsigned int address = reg[base].I + value;
	      reg[dest].I = CPUReadByteTransition (address);
	      if (dest != base)
		reg[base].I = address;
	    }
	    break;


	    CASE_16 (0x800)   // STMDA Rn, {Rlist}
	      CASE_16 (0x840) // STMDA Rn, {Rlist}^
	      {
		int base = (opcode & 0x000F0000) >> 16;
		unsigned int temp = reg[base].I -
		  4 * (cpuBitsSet[opcode & 255] +
		       cpuBitsSet[(opcode >> 8) & 255]);
		unsigned int address = (temp + 4) & 0xFFFFFFFC;
		switch(mFsmState) {
		case COMPUTEADDRESS:
#ifdef DEBUG
		  fprintf(logfile,"Compute Address: %x\n",address);
#endif
		  reg_remaining = cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode>>8)&255];
		  num_register=-1;
		  need_to_stall=true;
		  reg[15].I -= 4;
		  mFsmState=REGISTER;
		  dm_address = address;
		  break;
		case REGISTER: {
		  need_to_stall = true;
		  int r = findNextReg(opcode);
		  if (r >=0) {
#ifdef DEBUG
		    fprintf(logfile,"Store register %d at @ %x\n",num_register,dm_address.read());
#endif
		    reg[15].I -= 4;
		    reg_remaining--;
		  } else {
		    mFsmState = END;		  
		    goto STMDA_END;
		  }
		  dm_address = dm_address + 4;
		}
		  break;
		case END:
		STMDA_END:
#ifdef DEBUG
		  fprintf(logfile,"Stm End\n");
#endif
		  //reg[15].I += 4;
		  need_to_stall=false;
		  num_register=-1;
		  mFsmState = COMPUTEADDRESS;
		  break;
		}
	      }
	    break;
	    
	    CASE_16 (0x820)   // STMDA Rn!, {Rlist}
	      CASE_16 (0x860) // STMDA Rn!, {Rlist}^
	      {
		// STMDA Rn!, {Rlist}
		int base = (opcode & 0x000F0000) >> 16;
		unsigned int temp = reg[base].I -
		  4 * (cpuBitsSet[opcode & 255] +
		       cpuBitsSet[(opcode >> 8) & 255]);
		unsigned int address = (temp + 4) & 0xFFFFFFFC;
		switch(mFsmState) {
		case COMPUTEADDRESS:
#ifdef DEBUG
		  fprintf(logfile,"Compute Address: %x\n",address);
#endif
		  reg_remaining = cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode>>8)&255];
		  num_register=-1;
		  need_to_stall=true;
		  reg[15].I -= 4;
		  mFsmState=REGISTER;
		  dm_address = address;
		  break;
		case REGISTER: {
		  need_to_stall = true;

		  int r = findNextReg(opcode);
		  if (r >=0) {
#ifdef DEBUG
		    fprintf(logfile,"Store register %d at @ %x\n",num_register,dm_address.read());
#endif
		  reg[15].I -= 4; 
		  //reg_remaining--;
		  } else {
		    mFsmState = END;		  
		    goto STMDAW_END;
		  }
		  dm_address = dm_address + 4;
		}
		  break;
		case END:
		STMDAW_END:
#ifdef DEBUG
		  fprintf(logfile,"Stm End\n");
#endif
		  //reg[15].I += 4;
		  need_to_stall=false;
		  num_register=-1;
		  reg[base].I = temp;
		  mFsmState = COMPUTEADDRESS;
		  break;
		}
	      }
	    break;

	    CASE_16 (0x880)   // STMIA Rn, {Rlist}
	      CASE_16 (0x8c0) // STMIA Rn, {Rlist}^
	      {
		int base = (opcode & 0x000F0000) >> 16;
		unsigned int address = reg[base].I & 0xFFFFFFFC;
		switch(mFsmState) {
		case COMPUTEADDRESS:
#ifdef DEBUG
		  fprintf(logfile,"Compute Address: %x\n",address);
#endif
		  reg_remaining = cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode>>8)&255];
		  num_register=-1;
		  need_to_stall=true;
		  reg[15].I -= 4;
		  mFsmState=REGISTER;
		  dm_address = address;
		  break;
		case REGISTER: {
		  need_to_stall = true;
		  int r = findNextReg(opcode);
		  if (r >= 0)  {
#ifdef DEBUG
		    fprintf(logfile,"Store register %d at @ %x\n",num_register,dm_address.read());
#endif
		    reg[15].I -= 4; 
		    //reg_remaining--;
		  } else {
		    mFsmState = END;
		    goto STMIA_END;
		  }
		  dm_address = dm_address + 4;
		}
		  break;
		case END:
		STMIA_END:		 
#ifdef DEBUG
		  fprintf(logfile,"Stm End\n");
#endif
		  //reg[15].I += 4;
		  need_to_stall=false;
		  num_register=-1;
		  mFsmState = COMPUTEADDRESS;
		  break;
		}
	      }
	    break;

	    CASE_16 (0x8a0)   // STMIA Rn!, {Rlist}
	      CASE_16 (0x8e0) // STMIA Rn!, {Rlist}^
	      {
		int base = (opcode & 0x000F0000) >> 16;
		unsigned int address = reg[base].I & 0xFFFFFFFC;
		unsigned int temp =
		  reg[base].I + 4 * (cpuBitsSet[opcode & 0xFF] +
				     cpuBitsSet[(opcode >> 8) & 255]);
		switch(mFsmState) {
		case COMPUTEADDRESS:
#ifdef DEBUG
		  fprintf(logfile,"Compute Address: %x\n",address);
#endif
		  //reg_remaining = cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode>>8)&255];
		  num_register=-1;
		  need_to_stall=true;
		  reg[15].I -= 4;
		  mFsmState=REGISTER;
		  dm_address = address;
		  break;
		case REGISTER: {
		  need_to_stall = true;
		  int r = findNextReg(opcode);
		  if (r >= 0) {
#ifdef DEBUG
		    fprintf(logfile,"Store register %d at @ %x\n",num_register,dm_address.read());
#endif
		    reg[15].I -= 4; 
		    //reg_remaining--;
		  } else {
		    mFsmState = END;		  
		    goto STMIAW_END;
		  }
		  dm_address = dm_address + 4;
		}
		  break;
		case END:
		STMIAW_END:
#ifdef DEBUG
		  fprintf(logfile,"Stm End\n");
#endif
		  //reg[15].I += 4;
		  need_to_stall=false;
		  num_register=-1;
		  reg[base].I = temp;
		  mFsmState = COMPUTEADDRESS;
		  break;
		}
	      }
	    break;


	    CASE_16 (0x900) // STMDB Rn, {Rlist}
	      CASE_16 (0x940) // STMDB Rn, {Rlist}^
	      {
		int base = (opcode & 0x000F0000) >> 16;
		unsigned int temp = reg[base].I -
		  4 * (cpuBitsSet[opcode & 255] +
		       cpuBitsSet[(opcode >> 8) & 255]);
		unsigned int address = temp & 0xFFFFFFFC;
		switch(mFsmState) {
		case COMPUTEADDRESS:
#ifdef DEBUG
		  fprintf(logfile,"Compute Address: %x\n",address);
#endif
		  //reg_remaining = cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode>>8)&255];
		  num_register=-1;
		  need_to_stall=true;
		  reg[15].I -= 4;
		  mFsmState=REGISTER;
		  dm_address = address;
		  break;
		case REGISTER: {
		  need_to_stall = true;
		  int r = findNextReg(opcode);
		  if (r >=0) {
#ifdef DEBUG
		    fprintf(logfile,"Store register %d at @ %x\n",num_register,dm_address.read());
#endif
		    reg[15].I -= 4; 
		  } else {
		    mFsmState = END;
		    goto STMDB_END;
		  }
		  dm_address = dm_address + 4;
		}
		  break;
		case END:
		STMDB_END:
#ifdef DEBUG
		  fprintf(logfile,"Stm End\n");
#endif
		  //reg[15].I += 4;
		  need_to_stall=false;
		  num_register=-1;
		  mFsmState = COMPUTEADDRESS;
		  break;
		}
	      }
	    break;

	    CASE_16 (0x920) // STMDB Rn!, {Rlist}
	      CASE_16 (0x960) // STMDB Rn!, {Rlist}^
	      {
		int base = (opcode & 0x000F0000) >> 16;
		unsigned int temp = reg[base].I -
		  4 * (cpuBitsSet[opcode & 255] +
		       cpuBitsSet[(opcode >> 8) & 255]);
		unsigned int address = temp & 0xFFFFFFFC;
		switch(mFsmState) {
		case COMPUTEADDRESS:
#ifdef DEBUG
		  fprintf(logfile,"Compute Address: %x\n",address);
#endif
		  //reg_remaining = cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode>>8)&255];
		  num_register=-1;
		  need_to_stall=true;
		  reg[15].I -= 4;
		  mFsmState=REGISTER;
		  dm_address = address;
		  break;
		  
		case REGISTER: {
		  need_to_stall = true;
		  int r = findNextReg(opcode);
		  if (r >=0) {
#ifdef DEBUG
		    fprintf(logfile,"Store register %d at @ %x\n",num_register,dm_address.read());
#endif
		  reg[15].I -= 4; 
		  } else {
		    mFsmState = END;		  
		    goto STMDBW_END;
		  }
		  dm_address = dm_address + 4;
		}
		  break;
		  
		case END:
		STMDBW_END:
#ifdef DEBUG
		  fprintf(logfile,"Stm End\n");
#endif
		  //reg[15].I += 4;
		  need_to_stall=false;
		  num_register=-1;
		  reg[base].I = address;
		  mFsmState = COMPUTEADDRESS;
		  break;
		}
	      }
	    break;

	    CASE_16 (0x980) // STMIB Rn, {Rlist}
	      CASE_16 (0x9c0) // STMIB Rn, {Rlist}^
	      {
		int base = (opcode & 0x000F0000) >> 16;
		unsigned int address = (reg[base].I + 4) & 0xFFFFFFFC;
		switch(mFsmState) {
		case COMPUTEADDRESS:
#ifdef DEBUG
		  fprintf(logfile,"Compute Address: %x\n",address);
#endif
		  //reg_remaining = cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode>>8)&255];
		  num_register=-1;
		  need_to_stall=true;
		  reg[15].I -= 4;
		  mFsmState=REGISTER;
		  dm_address = address;
		  break;
		case REGISTER: {
		  need_to_stall = true;
		  int r = findNextReg(opcode);
		  if (r >=0) {
#ifdef DEBUG
		    fprintf(logfile,"Store register %d at @ %x\n",num_register,dm_address.read());
#endif
		  reg[15].I -= 4; 
		  } else {
		    mFsmState = END;
		    goto STMIB_END;
		  }
		  dm_address = dm_address + 4;
		}
		  break;
		case END:
		STMIB_END:
#ifdef DEBUG
		  fprintf(logfile,"Stm End\n");
#endif
		  //reg[15].I += 4;
		  need_to_stall=false;
		  num_register=-1;
		  mFsmState = COMPUTEADDRESS;
		  break;
		}
	      }
	    break;

	    CASE_16 (0x9a0) // STMIB Rn!, {Rlist}
	      CASE_16 (0x9e0) // STMIB Rn!, {Rlist}^
	      {
		int base = (opcode & 0x000F0000) >> 16;
		unsigned int address = (reg[base].I + 4) & 0xFFFFFFFC;
		unsigned int temp =
		  reg[base].I + 4 * (cpuBitsSet[opcode & 0xFF] +
				     cpuBitsSet[(opcode >> 8) & 255]);
		switch(mFsmState) {
		case COMPUTEADDRESS:
#ifdef DEBUG
		  fprintf(logfile,"Compute Address: %x\n",address);
#endif
		  //reg_remaining = cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode>>8)&255];
		  num_register=-1;
		  need_to_stall=true;
		  reg[15].I -= 4;
		  mFsmState=REGISTER;
		  dm_address = address;
		  break;
		case REGISTER: {
		  need_to_stall = true;
		  int r = findNextReg(opcode);
		  if (r >=0) {
#ifdef DEBUG
		    fprintf(logfile,"Store register %d at @ %x\n",num_register,dm_address.read());
#endif
		  reg[15].I -= 4; 
		  } else {
		    mFsmState = END;		  
		    goto STMIBW_END;
		  }
		  dm_address = dm_address + 4;
		}
		  break;
		case END:
		STMIBW_END:
#ifdef DEBUG
		  fprintf(logfile,"Stm End\n");
#endif
		  //reg[15].I += 4;
		  need_to_stall=false;
		  num_register=-1;
		  reg[base].I = temp;
		  mFsmState = COMPUTEADDRESS;
		  break;
		}
	      }
	    break;

	    CASE_16 (0x810) {
	      // LDMDA Rn, {Rlist}
	      int base = (opcode & 0x000F0000) >> 16;
	      unsigned int temp = reg[base].I -
		4 * (cpuBitsSet[opcode & 255] +
		     cpuBitsSet[(opcode >> 8) & 255]);
	      unsigned int address = (temp + 4) & 0xFFFFFFFC; 
	      switch(mFsmState) {
	      case COMPUTEADDRESS:
#ifdef DEBUG
		fprintf(logfile,"Compute Address: %x\n", address);
#endif
		//reg_remaining = cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode>>8)&255];
		num_register=-1;
		need_to_stall=true;
		reg[15].I -= 4;
		mFsmState = REGISTER;
		dm_address = address;
		break;

	      case REGISTER:{
		need_to_stall=true;

		int r = findNextReg(opcode);
		if (r >= 0) {
#ifdef DEBUG
		  fprintf(logfile,"Load from @ %x in register %d\n",dm_address.read(),num_register);
#endif
		  reg[r].I = CPUReadMemoryTransition(dm_address);
		  if (r == 15) {
		    armNextPC = reg[15].I;
		    branch_taken=true;
		    num_register = -1;
		    mFsmState = COMPUTEADDRESS;
		    if (armState)
		      reg[15].I -= 4;
		    else
		      reg[15].I -= 2;
		  }
		} else {
		  mFsmState = END;
		  goto LDMDA_END;
		}
		dm_address = dm_address + 4;
	      }
		break;

	      case END:
	      LDMDA_END:
#ifdef DEBUG
		fprintf(logfile,"Ldm end\n");
#endif
		//reg[15].I += 4;
		need_to_stall = false;
		num_register = -1;
		mFsmState = COMPUTEADDRESS;
		break;
	      }
	    }
	    break;

	    CASE_16 (0x830) {
	      // LDMDA Rn!, {Rlist}
	      int base = (opcode & 0x000F0000) >> 16;
	      unsigned int temp = reg[base].I -
		4 * (cpuBitsSet[opcode & 255] +
		     cpuBitsSet[(opcode >> 8) & 255]);
	      unsigned int address = (temp + 4) & 0xFFFFFFFC;
	      switch(mFsmState) {
	      case COMPUTEADDRESS:
#ifdef DEBUG
		fprintf(logfile,"Compute Address: %x\n", address);
#endif
		reg_remaining = cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode>>8)&255];
		num_register=-1;
		need_to_stall=true;
		reg[15].I -= 4;
		mFsmState = REGISTER;
		dm_address = address;
		break;

	      case REGISTER:{
		need_to_stall=true;
		int r = findNextReg(opcode);
		if (r >= 0) {
#ifdef DEBUG
		  fprintf(logfile,"Load from @ %x in register %d\n",dm_address.read(),num_register);
#endif
		  reg[15].I -= 4;
		  reg[r].I = CPUReadMemoryTransition(dm_address);
		  if (r == 15) {
		    armNextPC = reg[15].I;
		    branch_taken=true;
		    num_register = -1;
		    mFsmState = COMPUTEADDRESS;
		    if (!(opcode & 1<<base))
		      reg[base].I = temp;
		    if (armState)
		      reg[15].I -= 4;
		    else
		      reg[15].I -= 2;
		  }
		} else {
		  mFsmState = END;
		  goto LDMDAW_END;
		}
		dm_address = dm_address + 4;
	      }
		break;

	      case END:
	      LDMDAW_END:
#ifdef DEBUG
		fprintf(logfile,"Ldm end\n");
#endif
		//reg[15].I += 4;
		need_to_stall = false;
		num_register = -1;
		// FIXME:
		// As we are currently not cycle accurate, we have
		// to check if we can move the write-back value
		// into the base register.
		if (!(opcode & 1<<base))
		  reg[base].I = temp;
		mFsmState = COMPUTEADDRESS;
		break;
	      }
	    }
	    break;

	    CASE_16 (0x850) {
	      // LDMDA Rn, {Rlist}^
	      int base = (opcode & 0x000F0000) >> 16;
	      unsigned int temp = reg[base].I -
		4 * (cpuBitsSet[opcode & 255] +
		     cpuBitsSet[(opcode >> 8) & 255]);
	      unsigned int address = (temp + 4) & 0xFFFFFFFC;
	      switch(mFsmState) {
	      case COMPUTEADDRESS:
#ifdef DEBUG
		fprintf(logfile,"Compute Address: %x\n", address);
#endif
		reg_remaining = cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode>>8)&255];
		num_register=-1;
		need_to_stall=true;
		reg[15].I -= 4;
		mFsmState = REGISTER;
		dm_address = address;
		break;

	      case REGISTER: {
		need_to_stall=true;
		int r = findNextReg(opcode);
		if (r < 0) {
		  mFsmState = END;
		  goto LDMDAU_END;
		}
		else {
#ifdef DEBUG
		  fprintf(logfile,"Load from @ %x in register %d\n",dm_address.read(),num_register);
#endif
		  reg[15].I -= 4;
		  if (opcode & 0x8000) {
		    reg[num_register].I = CPUReadMemoryTransition(dm_address);
		    // FIXME:
		    // At the moment, we change the processor mode in one
		    // delta cycle so, we are really not cycle accurate
		    // at all.
		    if (r == 15) {
		      branch_taken=true;
		      CPUSwitchMode (reg[17].I & 0x1F, false);
		      num_register = -1;
		      mFsmState = COMPUTEADDRESS;
		      if (armState)
			reg[15].I -= 4;
		      else
			reg[15].I -= 2;
		    }
		  } 
		  else {
		    if ((r >= 8) && (r <= 12)) {
		      if (armMode == 0x11)
			reg[num_register+OFFSET_FIQ].I = CPUReadMemoryTransition(dm_address);
		      else
			reg[num_register].I = CPUReadMemoryTransition(dm_address);
		    } 
		    else if ((r == 13) || (r == 14)) {
		      if ((armMode != 0x10) && (armMode != 0x1F))
			reg[num_register+OFFSET_USR].I = CPUReadMemoryTransition(dm_address);
		      else
			reg[num_register].I = CPUReadMemoryTransition(dm_address);
		    }
		    else
		      reg[num_register].I = CPUReadMemoryTransition(dm_address);
		  }
		}
		dm_address = dm_address + 4;
	      }
		break;

	      case END:
	      LDMDAU_END:
#ifdef DEBUG
		fprintf(logfile,"Ldm end\n");
#endif
		//reg[15].I += 4;
		need_to_stall = false;
		CPUSwitchMode (reg[17].I & 0x1F, false);
		num_register = -1;
		mFsmState = COMPUTEADDRESS;
		break;
	      }
	    }
	    break;

	    CASE_16 (0x870) {
	      // LDMDA Rn!, {Rlist}^
	      int base = (opcode & 0x000F0000) >> 16;
	      unsigned int temp = reg[base].I -
		4 * (cpuBitsSet[opcode & 255] +
		     cpuBitsSet[(opcode >> 8) & 255]);
	      unsigned int address = (temp + 4) & 0xFFFFFFFC;
	      switch(mFsmState) {
	      case COMPUTEADDRESS:
#ifdef DEBUG
		fprintf(logfile,"Compute Address: %x\n", address);
#endif
		reg_remaining = cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode>>8)&255];
		num_register=-1;
		need_to_stall=true;
		reg[15].I -= 4;
		mFsmState = REGISTER;
		dm_address = address;
		break;

	      case REGISTER: {
		need_to_stall=true;
		int r = findNextReg(opcode);
		if (r < 0) {
		  mFsmState = END;
		  goto LDMDAWU_END;
		}
		else {
#ifdef DEBUG
		  fprintf(logfile,"Load from @ %x in register %d\n",dm_address.read(),num_register);
#endif
		  reg[15].I -= 4;
		  if (opcode & 0x8000) {
		    reg[num_register].I = CPUReadMemoryTransition(dm_address);
		    // FIXME:
		    // At the moment, we change the processor mode in one
		    // delta cycle so, we are really not cycle accurate
		    // at all.
		    if (r == 15) {
		      branch_taken=true;
		      num_register = -1;
		      if (!(opcode & 1<<base))
			reg[base].I = temp;
		      CPUSwitchMode (reg[17].I & 0x1F, false);
		      mFsmState = COMPUTEADDRESS;
		      if (armState)
			reg[15].I -= 4;
		      else
			reg[15].I -= 2;
		    }
		  } 
		  else {
		    if ((r >= 8) && (r <= 12)) {
		      if (armMode == 0x11)
			reg[num_register+OFFSET_FIQ].I = CPUReadMemoryTransition(dm_address);
		      else
			reg[num_register].I = CPUReadMemoryTransition(dm_address);
		    } 
		    else if ((r == 13) || (r == 14)) {
		      if ((armMode != 0x10) && (armMode != 0x1F))
			reg[num_register+OFFSET_USR].I = CPUReadMemoryTransition(dm_address);
		      else
			reg[num_register].I = CPUReadMemoryTransition(dm_address);
		    }
		    else
		      reg[num_register].I = CPUReadMemoryTransition(dm_address);
		  }
		}
		dm_address = dm_address + 4;
	      }
		break;

	      case END:
	      LDMDAWU_END:
#ifdef DEBUG
		fprintf(logfile,"Ldm end\n");
#endif
		//reg[15].I += 4;
		need_to_stall = false;
		num_register = -1;
		if (!(opcode & 1<<base))
		  reg[base].I = temp;
		CPUSwitchMode (reg[17].I & 0x1F, false);
		mFsmState = COMPUTEADDRESS;
		break;
	      }
	    }
	    break;

	    CASE_16 (0x890) {
	      // LDMIA Rn, {Rlist}
	      int base = (opcode & 0x000F0000) >> 16;
	      unsigned int address = reg[base].I & 0xFFFFFFFC;
	      switch(mFsmState) {
	      case COMPUTEADDRESS:
#ifdef DEBUG
		fprintf(logfile,"Compute Address: %x\n", address);
#endif
		reg_remaining = cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode>>8)&255];
		num_register=-1;
		need_to_stall=true;
		reg[15].I -= 4;
		mFsmState = REGISTER;
		dm_address = address;
		break;

	      case REGISTER:{
		need_to_stall=true;
		int r = findNextReg(opcode);
		if (r >= 0) {
#ifdef DEBUG
		  fprintf(logfile,"Load from @ %x in register %d\n",dm_address.read(),num_register);
#endif
		  reg[15].I -= 4;
		  reg[r].I = CPUReadMemoryTransition(dm_address);
		  if (r == 15) {
		    branch_taken=true;
		    armNextPC = reg[15].I;
		    num_register = -1;
		    mFsmState = COMPUTEADDRESS;
		    if (armState)
		      reg[15].I -= 4;
		    else
		      reg[15].I -= 2;
		  }
		} else {
		  mFsmState = END;
		  goto LDMIA_END;
		}
		dm_address = dm_address + 4;
	      }
		break;

	      case END:
	      LDMIA_END:
#ifdef DEBUG
		fprintf(logfile,"Ldm end\n");
#endif
		//reg[15].I += 4;
		need_to_stall = false;
		num_register = -1;
		mFsmState = COMPUTEADDRESS;
		break;
	      }
	    }
	    break;

	    CASE_16 (0x8b0) {
	      // LDMIA Rn!, {Rlist}
	      int base = (opcode & 0x000F0000) >> 16;
	      unsigned int temp = reg[base].I +
		4 * (cpuBitsSet[opcode & 255] +
		     cpuBitsSet[(opcode >> 8) & 255]);
	      unsigned int address = reg[base].I & 0xFFFFFFFC;
	      switch(mFsmState) {
	      case COMPUTEADDRESS:
#ifdef DEBUG
		fprintf(logfile,"Compute Address: %x\n", address);
#endif
		reg_remaining = cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode>>8)&255];
		num_register=-1;
		need_to_stall=true;
		reg[15].I -= 4;
		mFsmState = REGISTER;
		dm_address = address;
		break;

	      case REGISTER:{
		need_to_stall=true;
		int r = findNextReg(opcode);
		if (r >= 0) {
#ifdef DEBUG
		  fprintf(logfile,"Load from @ %x in register %d\n",dm_address.read(),num_register);
#endif
		  reg[15].I -= 4;
		  reg[r].I = CPUReadMemoryTransition(dm_address);
		  if (r == 15) {
		    armNextPC = reg[15].I;
		    branch_taken=true;
		    num_register = -1;
		    if (!(opcode & 1<<base))
		      reg[base].I = temp;
		    mFsmState = COMPUTEADDRESS;
		    if (armState)
		      reg[15].I -= 4;
		    else
		      reg[15].I -= 2;
		  }
		} else {
		  mFsmState = END;
		  goto LDMIAW_END;
		}
		dm_address = dm_address + 4;
	      }
		break;

	      case END:
	      LDMIAW_END:
#ifdef DEBUG
		fprintf(logfile,"Ldm end\n");
#endif
		//reg[15].I += 4;
		need_to_stall = false;
		num_register = -1;
		if (!(opcode & 1<<base))
		  reg[base].I = temp;
		mFsmState = COMPUTEADDRESS;
		break;
	      }
	    }
	    break;

	    CASE_16 (0x8d0) {
	      // LDMIA Rn, {Rlist}^
	      int base = (opcode & 0x000F0000) >> 16;
	      unsigned int address = reg[base].I & 0xFFFFFFFC;
	      switch(mFsmState) {
	      case COMPUTEADDRESS:
#ifdef DEBUG
		fprintf(logfile,"Compute Address: %x\n", address);
#endif
		reg_remaining = cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode>>8)&255];
		num_register=-1;
		need_to_stall=true;
		reg[15].I -= 4;
		mFsmState = REGISTER;
		dm_address = address;
		break;

	      case REGISTER: {
		need_to_stall=true;
		int r = findNextReg(opcode);
		if (r < 0) {
		  mFsmState = END;
		  goto LDMIAU_END;
		}
		else {
#ifdef DEBUG
		  fprintf(logfile,"Load from @ %x in register %d\n",dm_address.read(),num_register);
#endif
		  reg[15].I -= 4;
		  if (opcode & 0x8000) {
		    reg[num_register].I = CPUReadMemoryTransition(dm_address);
		    // FIXME:
		    // At the moment, we change the processor mode in one
		    // delta cycle so, we are really not cycle accurate
		    // at all.
		    if (r == 15) {
		      branch_taken=true;
		      CPUSwitchMode (reg[17].I & 0x1F, false);
		      num_register = -1;
		      mFsmState = COMPUTEADDRESS;
		      if (armState)
			reg[15].I -= 4;
		      else
			reg[15].I -= 2;
		    }
		  } 
		  else {
		    if ((r >= 8) && (r <= 12)) {
		      if (armMode == 0x11)
			reg[num_register+OFFSET_FIQ].I = CPUReadMemoryTransition(dm_address);
		      else
			reg[num_register].I = CPUReadMemoryTransition(dm_address);
		    } 
		    else if ((r == 13) || (r == 14)) {
		      if ((armMode != 0x10) && (armMode != 0x1F))
			reg[num_register+OFFSET_USR].I = CPUReadMemoryTransition(dm_address);
		      else
			reg[num_register].I = CPUReadMemoryTransition(dm_address);
		    }
		    else
		      reg[num_register].I = CPUReadMemoryTransition(dm_address);
		  }
		}
		dm_address = dm_address + 4;
	      }
		break;

	      case END:
	      LDMIAU_END:
#ifdef DEBUG
		fprintf(logfile,"Ldm end\n");
#endif
		//reg[15].I += 4;
		need_to_stall = false;
		CPUSwitchMode (reg[17].I & 0x1F, false);
		num_register = -1;
		mFsmState = COMPUTEADDRESS;
		break;
	      }
	    }
	    break;

	    CASE_16 (0x8f0) {
	      // LDMIA Rn!, {Rlist}^
	      int base = (opcode & 0x000F0000) >> 16;
	      unsigned int temp = reg[base].I +
		4 * (cpuBitsSet[opcode & 255] +
		     cpuBitsSet[(opcode >> 8) & 255]);
	      unsigned int address = reg[base].I & 0xFFFFFFFC;
	      switch(mFsmState) {
	      case COMPUTEADDRESS:
#ifdef DEBUG
		fprintf(logfile,"Compute Address: %x\n", address);
#endif
		num_register=-1;
		need_to_stall=true;
		reg[15].I -= 4;
		mFsmState = REGISTER;
		dm_address = address;
		break;

	      case REGISTER: {
		need_to_stall=true;
		int r = findNextReg(opcode);
		if (r < 0) {
		  mFsmState = END;
		  goto LDMIAWU_END;
		}
		else {
#ifdef DEBUG
		  fprintf(logfile,"Load from @ %x in register %d\n",dm_address.read(),num_register);
#endif
		  reg[15].I -= 4;
		  if (opcode & 0x8000) {
		    reg[num_register].I = CPUReadMemoryTransition(dm_address);
		    // FIXME:
		    // At the moment, we change the processor mode in one
		    // delta cycle so, we are really not cycle accurate
		    // at all.
		    if (r == 15) {
		      branch_taken=true;
		      num_register = -1;
		      if (!(opcode & 1<<base))
			reg[base].I = temp;
		      CPUSwitchMode (reg[17].I & 0x1F, false);
		      mFsmState = COMPUTEADDRESS;
		      if (armState)
			reg[15].I -= 4;
		      else
			reg[15].I -= 2;
		    }
		  } 
		  else {
		    if ((r >= 8) && (r <= 12)) {
		      if (armMode == 0x11)
			reg[num_register+OFFSET_FIQ].I = CPUReadMemoryTransition(dm_address);
		      else
			reg[num_register].I = CPUReadMemoryTransition(dm_address);
		    } 
		    else if ((r == 13) || (r == 14)) {
		      if ((armMode != 0x10) && (armMode != 0x1F))
			reg[num_register+OFFSET_USR].I = CPUReadMemoryTransition(dm_address);
		      else
			reg[num_register].I = CPUReadMemoryTransition(dm_address);
		    }
		    else
		      reg[num_register].I = CPUReadMemoryTransition(dm_address);
		  }
		}
		dm_address = dm_address + 4;
	      }
		break;

	      case END:
	      LDMIAWU_END:
#ifdef DEBUG
		fprintf(logfile,"Ldm end\n");
#endif
		//reg[15].I += 4;
		need_to_stall = false;
		num_register = -1;
		if (!(opcode & 1<<base))
		  reg[base].I = temp;
		CPUSwitchMode (reg[17].I & 0x1F, false);
		mFsmState = COMPUTEADDRESS;
		break;
	      }
	    }
	    break;

	    CASE_16 (0x910) {
	      // LDMDB Rn, {Rlist}
	      int base = (opcode & 0x000F0000) >> 16;
	      unsigned int temp = reg[base].I -
		4 * (cpuBitsSet[opcode & 255] +
		     cpuBitsSet[(opcode >> 8) & 255]);
	      unsigned int address = temp & 0xFFFFFFFC;
	      switch(mFsmState) {
	      case COMPUTEADDRESS:
#ifdef DEBUG
		fprintf(logfile,"Compute Address: %x\n", address);
#endif
		num_register=-1;
		need_to_stall=true;
		reg[15].I -= 4;
		mFsmState = REGISTER;
		dm_address = address;
		break;

	      case REGISTER:{
		need_to_stall=true;
		int r = findNextReg(opcode);
		if (r >= 0) {
#ifdef DEBUG
		  fprintf(logfile,"Load from @ %x in register %d\n",dm_address.read(),num_register);
#endif
		  reg[15].I -= 4;
		  reg[r].I = CPUReadMemoryTransition(dm_address);
		  if (r == 15) {
		    armNextPC = reg[15].I;
		    branch_taken=true;
		    num_register = -1;
		    mFsmState = COMPUTEADDRESS;
		    if (armState)
		      reg[15].I -= 4;
		    else
		      reg[15].I -= 2;
		  }
		} else {
		  mFsmState = END;
		  goto LDMDB_END;
		}
		dm_address = dm_address + 4;
	      }
		break;

	      case END:
	      LDMDB_END:
#ifdef DEBUG

		fprintf(logfile,"Ldm end\n");
#endif
		//reg[15].I += 4;
		need_to_stall = false;
		num_register = -1;
		mFsmState = COMPUTEADDRESS;
		break;
	      }
	    }
	    break;

	    CASE_16 (0x930) {
	      // LDMDB Rn!, {Rlist}
	      int base = (opcode & 0x000F0000) >> 16;
	      unsigned int temp = reg[base].I -
		4 * (cpuBitsSet[opcode & 255] +
		     cpuBitsSet[(opcode >> 8) & 255]);
	      unsigned int address = temp & 0xFFFFFFFC;
	      switch(mFsmState) {
	      case COMPUTEADDRESS:
#ifdef DEBUG
		fprintf(logfile,"Compute Address: %x\n", address);
#endif
		num_register=-1;
		need_to_stall=true;
		reg[15].I -= 4;
		mFsmState = REGISTER;
		dm_address = address;
		break;

	      case REGISTER:{
		need_to_stall=true;
		int r = findNextReg(opcode);
		if (r >= 0) {
#ifdef DEBUG
		  fprintf(logfile,"Load from @ %x in register %d\n",dm_address.read(),num_register);
#endif
		  reg[15].I -= 4;
		  reg[r].I = CPUReadMemoryTransition(dm_address);
		  if (r == 15) {
		    armNextPC = reg[15].I;
		    branch_taken=true;
		    if (!(opcode & 1<<base))
		      reg[base].I = temp;
		    num_register = -1;
		    mFsmState = COMPUTEADDRESS;
		    if (armState)
		      reg[15].I -= 4;
		    else
		      reg[15].I -= 2;
		  }
		} else {
		  mFsmState = END;
		  goto LDMDBW_END;
		}
		dm_address = dm_address + 4;}
		break;

	      case END:
	      LDMDBW_END:
#ifdef DEBUG
		fprintf(logfile,"Ldm end\n");
#endif
		//reg[15].I += 4;
		if (!(opcode & 1<<base))
		  reg[base].I = temp;
		need_to_stall = false;
		num_register = -1;
		mFsmState = COMPUTEADDRESS;
		break;
	      }
	    }
	    break;

	    CASE_16 (0x950) {
	      // LDMDB Rn, {Rlist}^
	      int base = (opcode & 0x000F0000) >> 16;
	      unsigned int temp = reg[base].I -
		4 * (cpuBitsSet[opcode & 255] +
		     cpuBitsSet[(opcode >> 8) & 255]);
	      unsigned int address = temp & 0xFFFFFFFC;
	      switch(mFsmState) {
	      case COMPUTEADDRESS:
#ifdef DEBUG
		fprintf(logfile,"Compute Address: %x\n", address);
#endif
		num_register=-1;
		need_to_stall=true;
		reg[15].I -= 4;
		mFsmState = REGISTER;
		dm_address = address;
		break;

	      case REGISTER: {
		need_to_stall=true;
		int r = findNextReg(opcode);
		if (r < 0) {
		  mFsmState = END;
		  goto LDMDBU_END;
		}
		else {
#ifdef DEBUG
		  fprintf(logfile,"Load from @ %x in register %d\n",dm_address.read(),num_register);
#endif
		  reg[15].I -= 4;
		  if (opcode & 0x8000) {
		    reg[num_register].I = CPUReadMemoryTransition(dm_address);
		    // FIXME:
		    // At the moment, we change the processor mode in one
		    // delta cycle so, we are really not cycle accurate
		    // at all.
		    if (r == 15) {
		      CPUSwitchMode (reg[17].I & 0x1F, false);
		      branch_taken=true;
		      num_register = -1;
		      mFsmState = COMPUTEADDRESS;
		      if (armState)
			reg[15].I -= 4;
		      else
			reg[15].I -= 2;
		    }
		  } 
		  else {
		    if ((r >= 8) && (r <= 12)) {
		      if (armMode == 0x11)
			reg[num_register+OFFSET_FIQ].I = CPUReadMemoryTransition(dm_address);
		      else
			reg[num_register].I = CPUReadMemoryTransition(dm_address);
		    } 
		    else if ((r == 13) || (r == 14)) {
		      if ((armMode != 0x10) && (armMode != 0x1F))
			reg[num_register+OFFSET_USR].I = CPUReadMemoryTransition(dm_address);
		      else
			reg[num_register].I = CPUReadMemoryTransition(dm_address);
		    }
		    else
		      reg[num_register].I = CPUReadMemoryTransition(dm_address);
		  }
		}
		dm_address = dm_address + 4;
	      }
		break;

	      case END:
	      LDMDBU_END:
#ifdef DEBUG
		fprintf(logfile,"Ldm end\n");
#endif
		//reg[15].I += 4;
		need_to_stall = false;
		num_register = -1;
		CPUSwitchMode (reg[17].I & 0x1F, false);
		mFsmState = COMPUTEADDRESS;
		break;
	      }
	    }
	    break;

	    CASE_16 (0x970) {
	      // LDMDB Rn!, {Rlist}^
	      int base = (opcode & 0x000F0000) >> 16;
	      unsigned int temp = reg[base].I -
		4 * (cpuBitsSet[opcode & 255] +
		     cpuBitsSet[(opcode >> 8) & 255]);
	      unsigned int address = temp & 0xFFFFFFFC;
	      switch(mFsmState) {
	      case COMPUTEADDRESS:
#ifdef DEBUG
		fprintf(logfile,"Compute Address: %x\n", address);
#endif
		num_register=-1;
		need_to_stall=true;
		reg[15].I -= 4;
		mFsmState = REGISTER;
		dm_address = address;
		break;

	      case REGISTER: {
		need_to_stall=true;
		int r = findNextReg(opcode);
		if (r < 0) {
		  mFsmState = END;
		  goto LDMDBWU_END;
		}
		else {
#ifdef DEBUG
		  fprintf(logfile,"Load from @ %x in register %d\n",dm_address.read(),num_register);
#endif
		  reg[15].I -= 4;
		  if (opcode & 0x8000) {
		    reg[num_register].I = CPUReadMemoryTransition(dm_address);
		    // FIXME:
		    // At the moment, we change the processor mode in one
		    // delta cycle so, we are really not cycle accurate
		    // at all.
		    if (r == 15) {
		      branch_taken=true;
		      num_register = -1;
		      if (!(opcode & 1<<base))
			reg[base].I = temp;
		      CPUSwitchMode (reg[17].I & 0x1F, false);
		      mFsmState = COMPUTEADDRESS;
		      if (armState)
			reg[15].I -= 4;
		      else
			reg[15].I -= 2;
		    }
		  } 
		  else {
		    if ((r >= 8) && (r <= 12)) {
		      if (armMode == 0x11)
			reg[num_register+OFFSET_FIQ].I = CPUReadMemoryTransition(dm_address);
		      else
			reg[num_register].I = CPUReadMemoryTransition(dm_address);
		    } 
		    else if ((r == 13) || (r == 14)) {
		      if ((armMode != 0x10) && (armMode != 0x1F))
			reg[num_register+OFFSET_USR].I = CPUReadMemoryTransition(dm_address);
		      else
			reg[num_register].I = CPUReadMemoryTransition(dm_address);
		    }
		    else
		      reg[num_register].I = CPUReadMemoryTransition(dm_address);
		  }
		}
		dm_address = dm_address + 4;
	      }
		break;

	      case END:
	      LDMDBWU_END:
#ifdef DEBUG
		fprintf(logfile,"Ldm end\n");
#endif
		//reg[15].I += 4;
		need_to_stall = false;
		num_register = -1;
		if (!(opcode & 1<<base))
		  reg[base].I = temp;
		CPUSwitchMode (reg[17].I & 0x1F, false);
		mFsmState = COMPUTEADDRESS;
		break;
	      }
	    }
	    break;

	    CASE_16 (0x990) {
	      // LDMIB Rn, {Rlist}
	      int base = (opcode & 0x000F0000) >> 16;
	      unsigned int address = (reg[base].I + 4) & 0xFFFFFFFC;
	      switch(mFsmState) {
	      case COMPUTEADDRESS:
#ifdef DEBUG
		fprintf(logfile,"Compute Address: %x\n", address);
#endif
		num_register=-1;
		need_to_stall=true;
		reg[15].I -= 4;
		mFsmState = REGISTER;
		dm_address = address;
		break;

	      case REGISTER:{
		need_to_stall=true;
		int r = findNextReg(opcode);
		if (r >= 0) {
#ifdef DEBUG
		  fprintf(logfile,"Load from @ %x in register %d\n",dm_address.read(),num_register);
#endif
		  reg[15].I -= 4;
		  reg[r].I = CPUReadMemoryTransition(dm_address);
		  if (r == 15) {
		    armNextPC = reg[15].I;
		    branch_taken=true;
		    num_register = -1;
		    mFsmState = COMPUTEADDRESS;
		    if (armState)
		      reg[15].I -= 4;
		    else
		      reg[15].I -= 2;
		  }
		} else {
		  mFsmState = END;
		  goto LDMIB_END;
		}
		dm_address = dm_address + 4;
	      }
		break;

	      case END:
	      LDMIB_END:
#ifdef DEBUG
		fprintf(logfile,"Ldm end\n");
#endif
		//reg[15].I += 4;
		need_to_stall = false;
		num_register = -1;
		mFsmState = COMPUTEADDRESS;
		break;
	      }
	    }
	    break;

	    CASE_16 (0x9b0) {
	      // LDMIB Rn!, {Rlist}
	      int base = (opcode & 0x000F0000) >> 16;
	      unsigned int temp = reg[base].I +
		4 * (cpuBitsSet[opcode & 255] +
		     cpuBitsSet[(opcode >> 8) & 255]);
	      unsigned int address = (reg[base].I + 4) & 0xFFFFFFFC;
	      switch(mFsmState) {
	      case COMPUTEADDRESS:
#ifdef DEBUG
		fprintf(logfile,"Compute Address: %x\n", address);
#endif
		num_register=-1;
		need_to_stall=true;
		reg[15].I -= 4;
		mFsmState = REGISTER;
		dm_address = address;
		break;

	      case REGISTER:{
		need_to_stall=true;
		int r = findNextReg(opcode);
		if (r >= 0) {
#ifdef DEBUG
		  fprintf(logfile,"Load from @ %x in register %d\n",dm_address.read(),num_register);
#endif
		  reg[15].I -= 4;
		  reg[r].I = CPUReadMemoryTransition(dm_address);
		  if (r == 15) {
		    armNextPC = reg[15].I;
		    branch_taken=true;
		    num_register = -1;
		    if (!(opcode & 1<<base))
		      reg[base].I = temp;
		    mFsmState = COMPUTEADDRESS;
		    if (armState)
		      reg[15].I -= 4;
		    else
		      reg[15].I -= 2;
		  }
		} else {
		  mFsmState = END;
		  goto LDMIBW_END;
		}
		dm_address = dm_address + 4;
	      }
		break;

	      case END:
	      LDMIBW_END:
#ifdef DEBUG
		fprintf(logfile,"Ldm end\n");
#endif
		//reg[15].I += 4;
		need_to_stall = false;
		num_register = -1;
		if (!(opcode & 1<<base))
		  reg[base].I = temp;
		mFsmState = COMPUTEADDRESS;
		break;
	      }
	    }
	    break;

	    CASE_16 (0x9d0) {
	      // LDMIB Rn, {Rlist}^
	      int base = (opcode & 0x000F0000) >> 16;
	      unsigned int address = (reg[base].I + 4) & 0xFFFFFFFC;
	      switch(mFsmState) {
	      case COMPUTEADDRESS:
#ifdef DEBUG
		fprintf(logfile,"Compute Address: %x\n", address);
#endif
		num_register=-1;
		need_to_stall=true;
		reg[15].I -= 4;
		mFsmState = REGISTER;
		dm_address = address;
		break;

	      case REGISTER: {
		need_to_stall=true;
		int r = findNextReg(opcode);
		if (r < 0) {
		  mFsmState = END;
		  goto LDMIBU_END;
		}
		else {
#ifdef DEBUG
		  fprintf(logfile,"Load from @ %x in register %d\n",dm_address.read(),num_register);
#endif
		  reg[15].I -= 4;
		  if (opcode & 0x8000) {
		    reg[num_register].I = CPUReadMemoryTransition(dm_address);
		    // FIXME:
		    // At the moment, we change the processor mode in one
		    // delta cycle so, we are really not cycle accurate
		    // at all.
		    if (r == 15) {
		      CPUSwitchMode (reg[17].I & 0x1F, false);
		      branch_taken=true;
		      num_register = -1;
		      mFsmState = COMPUTEADDRESS;
		      if (armState)
			reg[15].I -= 4;
		      else
			reg[15].I -= 2;
		    }
		  } 
		  else {
		    if ((r >= 8) && (r <= 12)) {
		      if (armMode == 0x11)
			reg[num_register+OFFSET_FIQ].I = CPUReadMemoryTransition(dm_address);
		      else
			reg[num_register].I = CPUReadMemoryTransition(dm_address);
		    } 
		    else if ((r == 13) || (r == 14)) {
		      if ((armMode != 0x10) && (armMode != 0x1F))
			reg[num_register+OFFSET_USR].I = CPUReadMemoryTransition(dm_address);
		      else
			reg[num_register].I = CPUReadMemoryTransition(dm_address);
		    }
		    else
		      reg[num_register].I = CPUReadMemoryTransition(dm_address);
		  }
		}
		dm_address = dm_address + 4;
	      }
		break;

	      case END:
	      LDMIBU_END:
#ifdef DEBUG
		fprintf(logfile,"Ldm end\n");
#endif
		//reg[15].I += 4;
		need_to_stall = false;
		num_register = -1;
		CPUSwitchMode (reg[17].I & 0x1F, false);
		mFsmState = COMPUTEADDRESS;
		break;
	      }
	    }
	    break;

	    CASE_16 (0x9f0) {
	      // LDMIB Rn!, {Rlist}^
	      int base = (opcode & 0x000F0000) >> 16;
	      unsigned int temp = reg[base].I +
		4 * (cpuBitsSet[opcode & 255] +
		     cpuBitsSet[(opcode >> 8) & 255]);
	      unsigned int address = (reg[base].I + 4) & 0xFFFFFFFC;
	      switch(mFsmState) {
	      case COMPUTEADDRESS:
#ifdef DEBUG
		fprintf(logfile,"Compute Address: %x\n", address);
#endif
		num_register=-1;
		need_to_stall=true;
		reg[15].I -= 4;
		mFsmState = REGISTER;
		dm_address = address;
		break;

	      case REGISTER: {
		need_to_stall=true;
		int r = findNextReg(opcode);
		if (r < 0) {
		  mFsmState = END;
		  goto LDMIBWU_END;
		}
		else {
#ifdef DEBUG
		  fprintf(logfile,"Load from @ %x in register %d\n",dm_address.read(),num_register);
#endif
		  reg[15].I -= 4;
		  if (opcode & 0x8000) {
		    reg[num_register].I = CPUReadMemoryTransition(dm_address);
		    // FIXME:
		    // At the moment, we change the processor mode in one
		    // delta cycle so, we are really not cycle accurate
		    // at all.
		    if (r == 15) {
		      branch_taken=true;
		      num_register = -1;
		      if (!(opcode & 1<<base))
			reg[base].I = temp;
		      CPUSwitchMode (reg[17].I & 0x1F, false);
		      mFsmState = COMPUTEADDRESS;
		      if (armState)
			reg[15].I -= 4;
		      else
			reg[15].I -= 2;
		    }
		  } 
		  else {
		    if ((r >= 8) && (r <= 12)) {
		      if (armMode == 0x11)
			reg[num_register+OFFSET_FIQ].I = CPUReadMemoryTransition(dm_address);
		      else
			reg[num_register].I = CPUReadMemoryTransition(dm_address);
		    } 
		    else if ((r == 13) || (r == 14)) {
		      if ((armMode != 0x10) && (armMode != 0x1F))
			reg[num_register+OFFSET_USR].I = CPUReadMemoryTransition(dm_address);
		      else
			reg[num_register].I = CPUReadMemoryTransition(dm_address);
		    }
		    else
		      reg[num_register].I = CPUReadMemoryTransition(dm_address);
		  }
		}
		dm_address = dm_address + 4;
	      }
		break;

	      case END:
	      LDMIBWU_END:
#ifdef DEBUG
		fprintf(logfile,"Ldm end\n");
#endif
		//reg[15].I += 4;
		need_to_stall = false;
		num_register = -1;
		if (!(opcode & 1<<base))
		  reg[base].I = temp;
		CPUSwitchMode (reg[17].I & 0x1F, false);
		mFsmState = COMPUTEADDRESS;
		break;
	      }
	    }
	    break;

	    CASE_256 (0xa00) {
	      // B <offset>
	      int offset = opcode & 0x00FFFFFF;
	      if (offset & 0x00800000)
		{
		  offset |= 0xFF000000;
		}
	      offset <<= 2;
	      reg[15].I += offset-4;
	      armNextPC = reg[15].I;
	      branch_taken=true;
	    }
	    break;

	    CASE_256 (0xb00) {
	      // BL <offset>
	      int offset = opcode & 0x00FFFFFF;
	      if (offset & 0x00800000)
		{
		  offset |= 0xFF000000;
		}
	      offset <<= 2;
	      reg[14].I = reg[15].I - 4;
	      //reg[14].I = reg[15].I - 8;
	      reg[15].I += offset-4;
	      armNextPC = reg[15].I;
	      branch_taken=true;
	    }
	    break;

	    CASE_256 (0xf00){
	      // SWI <comment>
	      CPUSoftwareInterrupt (opcode & 0x00FFFFFF);
	    }
	    break;
	    
	    CASE_256 (0xe00) {
	      // MCR <coproc>, <opcode1>, <Rd>, <CRn>, <CRm>{, <opcode2>}
	      // MRC <coproc>, <opcode1>, <Rd>, <CRn>, <CRm>{, <opcode2>}
	      if ((sv % 2) == 1) {
		int coproc = (opcode & 0x00000F00) >> 8;
		// 		int opcode1 = (opcode & 0x00F00000) >> 21;
		// 		int opcode2 = (opcode & 0x000000F0) >> 5;
		unsigned int rd = (opcode & 0x0000F000) >> 12;
 		unsigned int crm = (opcode & 0x0000000F);
		// 		unsigned int crn = (opcode & 0x000F0000) >> 16;
		// first, we do the MCR
		if (((sv>>4)%2) == 0) {
		  // In our model, we only implement one coprocessor which will act
		  // as a rather stupid set of registers to store some useful 
		  // values. So, we only test for p0, all the other values vill
		  // go to the dump... We don't even use the opcodes...
		  if (coproc != 0 ) {
		    CPUUndefinedException();
		    break;
		  }
		  if (!crm)
		    COPRO0[crm] = reg[rd].I;
		  else
		    CPUUndefinedException();
		} else { // then the MRC
		  if (coproc != 0 ) {
		    CPUUndefinedException();
		    break;
		  }
		  if (rd != 15) {
		    if (!crm)
		      reg[rd].I = COPRO0[crm].read();
		    else
		      CPUUndefinedException();
		  } else {
		    char val = (COPRO0[0].read() & 0xF0000000) >> 28;
		    N_FLAG = (val & 0x8) >> 3;
		    Z_FLAG = (val & 0x4) >> 2;
		    C_FLAG = (val & 0x2) >> 1;
		    V_FLAG = (val & 0x1);
		  }
		}
	      } else {
		CPUUndefinedException();
	      }
	    }
	    break;
	  default:
	    CPUUndefinedException ();
	    break;
	    // END
	  }
      }
    }

    CPUUpdateCPSR();

    if (branch_taken)
      {
	if (armState) {
	  IRD=ARM_NOP_INST;
	  IR=ARM_NOP_INST;
	} else {
	  IRD=ARM_THUMB_NOP_INST;
	  IR=ARM_THUMB_NOP_INST;
	}
	bubble_cycle = 3;
      }
    else if (need_to_stall)
      {
	// 	if (!armState)
	// 	  msb = !msb;
	// Do not do anything
      }
    else
      {
	IRD=IR;
	IR=(int)ICACHE.INS.read();
      }

    if (bubble_cycle > 0)
      bubble_cycle--;


    
    //////////////////////////////////////////////
    //      INTERRUPTION
    //////////////////////////////////////////////
    
    if (int_vector && (COMPUTEADDRESS == mFsmState)) {

      unsigned char inter = cpuLowestBitSet[int_vector];

      if (inter == FIQ_INT || inter == IRQ_INT || inter == DATA_ABORT_INT )
	goto inc_pc;

#ifdef DEBUG
      fprintf(logfile,"Traitement exception %d\n", inter);
#endif

      bool thumbState = !armState;

      // we have to test if we just had a branch in order to set the PC
      //char msb = (sv>>8)&0xF;
      //fprintf(logfile,"msb: %d\n",msb);

#ifdef DEBUG
      fprintf(logfile,"bubble: %d\n",bubble_cycle);
#endif
      if (bubble_cycle) {
	if (bubble_cycle == 2)
	  reg[15].I += 8;
	if (bubble_cycle == 1)
	  reg[15].I += 4;
      }

      switch(inter) {
      case RESET_INT:
	reg[SPSR_SVC].I = reg[16].I; // copy CPSR to SPSR_mode
	// 	if (!thumbState)
	// 	  reg[R14_SVC].I = reg[15].I;
	// 	else
	// 	  reg[R14_SVC].I = reg[15].I;
	armMode = 0x13;
	CPUSwitchMode(0x13, true); // Go to supervisor mode
	armState = true; // Go to ARM State
	armIrqEnable = false; // IRQ Disable = true
	armFiqEnable = false; // FIQ Disable = truea
	Z_FLAG = false;
	N_FLAG = false;
	C_FLAG = false;
	V_FLAG = false;
	CPUUpdateCPSR();
	reg[15].I = ARM_RESET_VECTOR;
	int_vector &= 0xe; // suppress the interruption
#ifdef DEBUG
	fprintf(logfile,"CPSR: %x\n",reg[16].I);
	fprintf(logfile,"r[15]: %x\n",reg[15].I);
	fprintf(logfile,"r[14]: %x\n",reg[R14_SVC].I);
#endif
	break;
	
      case PREFETCH_ABORT_INT:
	reg[SPSR_ABT].I = reg[16].I;
	if (!thumbState)
	  reg[R14_ABT].I = reg[15].I - 4;
	else
	  reg[R14_ABT].I = reg[15].I - 4;
	CPUSwitchMode(0x17, true);
	armState=true;
	armIrqEnable=false;
	Z_FLAG = false;
	N_FLAG = false;
	C_FLAG = false;
	V_FLAG = false;
	CPUUpdateCPSR();
	reg[15].I = ARM_PREFETCH_ABORT_VECTOR;
	int_vector &= 0xEF;
#ifdef DEBUG
	fprintf(logfile,"CPSR: %x\n",reg[16].I);
	fprintf(logfile,"r[15]: %x\n",reg[15].I);
	fprintf(logfile,"r[14]: %x\n",reg[R14_ABT].I);
#endif
	break;

      case UNDEF_INSTR_INT:
	reg[SPSR_UND].I = reg[16].I;
	if (!thumbState)
	  reg[R14_UND].I = reg[15].I - 8;
	else
	  reg[R14_UND].I = reg[15].I - 4;
	CPUSwitchMode(0x1b, true);
	armState=true;
	armIrqEnable=false;
	Z_FLAG = false;
	N_FLAG = false;
	C_FLAG = false;
	V_FLAG = false;
	CPUUpdateCPSR();
	reg[15].I = ARM_UNDEF_INSTR_VECTOR;
	int_vector &= 0xDF;
#ifdef DEBUG
	fprintf(logfile,"CPSR: %x\n",reg[16].I);
	fprintf(logfile,"r[15]: %x\n",reg[15].I);
	fprintf(logfile,"r[14]: %x\n",reg[R14_UND].I);
#endif
	break;

      case SWI_INT:
	reg[SPSR_SVC].I = reg[16].I;
	if (!thumbState)
	  reg[R14_SVC].I = reg[15].I - 4;
	else
	  reg[R14_SVC].I = reg[15].I - 2;
	CPUSwitchMode(0x13, true);
	armState=true;
	armIrqEnable=false;
	Z_FLAG = false;
	N_FLAG = false;
	C_FLAG = false;
	V_FLAG = false;
	CPUUpdateCPSR();
	reg[15].I = ARM_SWI_VECTOR;
	int_vector &= 0xBF;
#ifdef DEBUG
	fprintf(logfile,"CPSR: %x\n",reg[16].I);
	fprintf(logfile,"r[15]: %x\n",reg[15].I);
	fprintf(logfile,"r[14]: %x\n",reg[R14_SVC].I);
#endif
	break;

      default:
	break;
      }
      
      IRD=ARM_NOP_INST;
      IR=ARM_NOP_INST;
      bubble_cycle = 2;

      for (int i = 0; i < 45; i++)
	GPR[i]=reg[i].I;

      return;
    }    

  inc_pc:
    if (!modified_pc) {
      if (armState)
	reg[15].I += 4; // increment PC
      else
	reg[15].I += 2;
    }
    modified_pc = false;
    // we can now treat the IRQ and FIQ
    if (int_vector && (COMPUTEADDRESS == mFsmState)) {

      unsigned char inter = cpuLowestBitSet[int_vector];
#ifdef DEBUG
      fprintf(logfile,"Traitement exception %d\n", inter);
#endif

      bool thumbState = !armState;
#ifdef DEBUG
      fprintf(logfile,"bubble: %d\n",bubble_cycle);
#endif
      if (bubble_cycle) {
	if (bubble_cycle == 2)
	  reg[15].I += 8;
	if (bubble_cycle == 1)
	  reg[15].I += 4;
      }

      switch(inter) {
      case FIQ_INT:
	reg[SPSR_FIQ].I = reg[16].I;
	if (!thumbState)
	  reg[R14_FIQ].I = reg[15].I - 8; // Next Instruction + 4
	else
	  reg[R14_FIQ].I = reg[15].I - 4; // Next Instruction + 4
	CPUSwitchMode(0x11,true);
	armState=true;
	armIrqEnable=false;
	armFiqEnable=false;
	Z_FLAG = false;
	N_FLAG = false;
	C_FLAG = false;
	V_FLAG = false;
	CPUUpdateCPSR();
	reg[15].I = ARM_FIQ_VECTOR;
	int_vector &= 0xFB;
#ifdef DEBUG
	fprintf(logfile,"CPSR: %x\n",reg[16].I);
	fprintf(logfile,"r[15]: %x\n",reg[15].I);
	fprintf(logfile,"r[14]: %x\n",reg[R14_FIQ].I);
#endif
	break;

      case IRQ_INT:
	reg[SPSR_IRQ].I = reg[16].I;
	if (!thumbState)
	  reg[R14_IRQ].I = reg[15].I - 8;
	else
	  reg[R14_IRQ].I = reg[15].I - 4;
	CPUSwitchMode(0x12,true);
	armState=true;
	armIrqEnable=false;
	Z_FLAG = false;
	N_FLAG = false;
	C_FLAG = false;
	V_FLAG = false;
	CPUUpdateCPSR();
	reg[15].I = ARM_IRQ_VECTOR;
	int_vector &= 0xF7;
#ifdef DEBUG
	fprintf(logfile,"CPSR: %x\n",reg[16].I);
	fprintf(logfile,"r[15]: %x\n",reg[15].I);
	fprintf(logfile,"r[14]: %x\n",reg[R14_IRQ].I);
#endif
	break;
	
      case DATA_ABORT_INT:
	reg[SPSR_ABT].I = reg[16].I;
	reg[R14_ABT].I = reg[15].I;
	CPUSwitchMode(0x17, true);
	armState=true;
	armIrqEnable=false;
	Z_FLAG = false;
	N_FLAG = false;
	C_FLAG = false;
	V_FLAG = false;
	CPUUpdateCPSR();
	reg[15].I = ARM_DATA_ABORT_VECTOR;
	int_vector &= 0xFD;
#ifdef DEBUG
	fprintf(logfile,"CPSR: %x\n",reg[16].I);
	fprintf(logfile,"r[15]: %x\n",reg[15].I);
	fprintf(logfile,"r[14]: %x\n",reg[R14_ABT].I);
#endif
	break;

      default:
	break;
      }
      
      IRD=ARM_NOP_INST;
      IR=ARM_NOP_INST;
      bubble_cycle = 2;
      
      for (int i = 0; i < 45; i++)
	GPR[i]=reg[i].I;
      
      return;
    }

#ifdef DEBUG
      for (int i=0;i<4;i++)
	fprintf(logfile," r[%2.2d]=%8.8x",i,reg[i].I);
      fprintf(logfile,"\n");
      for (int i=4;i<8;i++)
	fprintf(logfile," r[%2.2d]=%8.8x",i,reg[i].I);
      fprintf(logfile,"\n");
      for (int i=8;i<12;i++)
	fprintf(logfile," r[%2.2d]=%8.8x",i,reg[i].I);
      fprintf(logfile,"\n");
      for (int i=12;i<16;i++)
	fprintf(logfile," r[%2.2d]=%8.8x",i,reg[i].I);
      fprintf(logfile,"\n");
      fprintf(logfile,"CPSR: 0x%x\n",reg[16].I);
#endif

    switch(armMode) {
    case USER_MODE:
    case SYSTEM_MODE:
      break;

    case SUPERVISOR_MODE:
      reg[R13_SVC].I = reg[13].I;
      reg[R14_SVC].I = reg[14].I;
      reg[SPSR_SVC].I = reg[17].I;
      break;

    case ABORT_MODE:
      reg[R13_ABT].I = reg[13].I;
      reg[R14_ABT].I = reg[14].I;
      reg[SPSR_ABT].I = reg[17].I;      
      break;

    case UNDEFINED_MODE:
      reg[R13_UND].I = reg[13].I;
      reg[R14_UND].I = reg[14].I;
      reg[SPSR_UND].I = reg[17].I;
      break;

    case IRQ_MODE:
      reg[R13_IRQ].I = reg[13].I;
      reg[R14_IRQ].I = reg[14].I;
      reg[SPSR_IRQ].I = reg[17].I;
      break;

    case FIQ_MODE:
      reg[R8_FIQ].I = reg[8].I ;
      reg[R9_FIQ].I = reg[9].I ;
      reg[R10_FIQ].I = reg[10].I;
      reg[R11_FIQ].I = reg[11].I;
      reg[R12_FIQ].I = reg[12].I;
      reg[R13_FIQ].I = reg[13].I;
      reg[R14_FIQ].I = reg[14].I;
      reg[SPSR_FIQ].I = reg[17].I;
      break;

    default:
#ifdef DEBUG
      fprintf(logfile,"Error in the CPSR: %x\n",armMode);
#endif
      break;
    }

    for (int i = 0; i < 45; i++)
      GPR[i]=reg[i].I;

  };				// end transition()

  ////////////////////////////////////////////////////////////
  //      genMoore()
  ////////////////////////////////////////////////////////////

  void SOCLIB_ARM7::genMoore ()
  {

    unsigned int opcode = (unsigned int)IRD.read();

    for (int i = 0; i < 45; i++)
      reg[i].I = GPR[i];

    if (!armState) {
      if (!msb)
	opcode = (opcode & 0xFFFF0000) >> 16;
      else
	opcode = (opcode & 0x0000FFFF);
#ifdef DEBUG
      fprintf(logfile,"\n opcode=%x\n\n\n",opcode);
#endif
      //       opcode = (opcode & 0x0000FFFF);
      switch(opcode >> 8) {
      case 0x48:
	// LDR R0,[PC, #Imm]
	{
	  unsigned int address = (reg[15].I & 0xFFFFFFFC) + ((opcode & 0xFF) << 2);
	  CPUReadMemoryMoore(address);
	}
	break;
      case 0x49:
	// LDR R1,[PC, #Imm]
	{
	  unsigned int address = (reg[15].I & 0xFFFFFFFC) + ((opcode & 0xFF) << 2);   
	  CPUReadMemoryMoore(address);
	}
	break;
      case 0x4a:
	// LDR R2,[PC, #Imm]
	{
	  unsigned int address = (reg[15].I & 0xFFFFFFFC) + ((opcode & 0xFF) << 2);      
	  CPUReadMemoryMoore(address);
	}
	break;
      case 0x4b:
	// LDR R3,[PC, #Imm]
	{
	  unsigned int address = (reg[15].I & 0xFFFFFFFC) + ((opcode & 0xFF) << 2);      
	  CPUReadMemoryMoore(address);
	}
	break;
      case 0x4c:
	// LDR R4,[PC, #Imm]
	{
	  unsigned int address = (reg[15].I & 0xFFFFFFFC) + ((opcode & 0xFF) << 2);      
	  CPUReadMemoryMoore(address);
	}
	break;
      case 0x4d:
	// LDR R5,[PC, #Imm]
	{
	  unsigned int address = (reg[15].I & 0xFFFFFFFC) + ((opcode & 0xFF) << 2);      
	  CPUReadMemoryMoore(address);
	}
	break;
      case 0x4e:
	// LDR R6,[PC, #Imm]
	{
	  unsigned int address = (reg[15].I & 0xFFFFFFFC) + ((opcode & 0xFF) << 2);      
	  CPUReadMemoryMoore(address);
	}
	break;
      case 0x4f:
	// LDR R7,[PC, #Imm]
	{
	  unsigned int address = (reg[15].I & 0xFFFFFFFC) + ((opcode & 0xFF) << 2);      
	  CPUReadMemoryMoore(address);
	}
	break;
      case 0x50:
      case 0x51:
	// STR Rd, [Rs, Rn]
	{
	  unsigned int address = reg[(opcode>>3)&7].I + reg[(opcode>>6)&7].I;
	  CPUWriteMemoryMoore(address, reg[opcode & 7].I);
	}
	break;
      case 0x52:
      case 0x53:
	// STRH Rd, [Rs, Rn]
	{
	  unsigned int address = reg[(opcode>>3)&7].I + reg[(opcode>>6)&7].I;
	  CPUWriteHalfWordMoore(address,reg[opcode&7].W.W0);
	}
	break;
      case 0x54:
      case 0x55:
	// STRB Rd, [Rs, Rn]
	{
	  unsigned int address = reg[(opcode>>3)&7].I + reg[(opcode >>6)&7].I;
	  CPUWriteByteMoore(address, reg[opcode & 7].B.B0);
	}
	break;
      case 0x56:
      case 0x57:
	// LDSB Rd, [Rs, Rn]
	{
	  unsigned int address = reg[(opcode>>3)&7].I + reg[(opcode>>6)&7].I;
	  CPUReadByteMoore(address);
	}
	break;
      case 0x58:
      case 0x59:
	// LDR Rd, [Rs, Rn]
	{
	  unsigned int address = reg[(opcode>>3)&7].I + reg[(opcode>>6)&7].I;
	  CPUReadMemoryMoore(address);
	}
	break;
      case 0x5a:
      case 0x5b:
	// LDRH Rd, [Rs, Rn]
	{
	  unsigned int address = reg[(opcode>>3)&7].I + reg[(opcode>>6)&7].I;
	  CPUReadHalfWordMoore(address);
	}
	break;
      case 0x5c:
      case 0x5d:
	// LDRB Rd, [Rs, Rn]
	{
	  unsigned int address = reg[(opcode>>3)&7].I + reg[(opcode>>6)&7].I;
	  CPUReadByteMoore(address);
	}
	break;
      case 0x5e:
      case 0x5f:
	// LDSH Rd, [Rs, Rn]
	{
	  unsigned int address = reg[(opcode>>3)&7].I + reg[(opcode>>6)&7].I;
	  CPUReadHalfWordSignedMoore(address);
	}
	break;
      case 0x60:
      case 0x61:
      case 0x62:
      case 0x63:
      case 0x64:
      case 0x65:
      case 0x66:
      case 0x67:
	// STR Rd, [Rs, #Imm]
	{
	  unsigned int address = reg[(opcode>>3)&7].I + (((opcode>>6)&31)<<2);
	  CPUWriteMemoryMoore(address, reg[opcode&7].I);
	}
	break;
      case 0x68:
      case 0x69:
      case 0x6a:
      case 0x6b:
      case 0x6c:
      case 0x6d:
      case 0x6e:
      case 0x6f:
	// LDR Rd, [Rs, #Imm]
	{
	  unsigned int address = reg[(opcode>>3)&7].I + (((opcode>>6)&31)<<2);
	  CPUReadMemoryMoore(address);
	}
	break;
      case 0x70:
      case 0x71:
      case 0x72:
      case 0x73:
      case 0x74:
      case 0x75:
      case 0x76:
      case 0x77:
	// STRB Rd, [Rs, #Imm]
	{
	  unsigned int address = reg[(opcode>>3)&7].I + (((opcode>>6)&31));
	  CPUWriteByteMoore(address, reg[opcode&7].B.B0);
	}
	break;
      case 0x78:
      case 0x79:
      case 0x7a:
      case 0x7b:
      case 0x7c:
      case 0x7d:
      case 0x7e:
      case 0x7f:
	// LDRB Rd, [Rs, #Imm]
	{
	  unsigned int address = reg[(opcode>>3)&7].I + (((opcode>>6)&31));
	  CPUReadByteMoore(address);
	}
	break;
      case 0x80:
      case 0x81:
      case 0x82:
      case 0x83:
      case 0x84:
      case 0x85:
      case 0x86:
      case 0x87:
	// STRH Rd, [Rs, #Imm]
	{
	  unsigned int address = reg[(opcode>>3)&7].I + (((opcode>>6)&31)<<1);
	  CPUWriteHalfWordMoore(address, reg[opcode&7].W.W0);
	}
	break;   
      case 0x88:
      case 0x89:
      case 0x8a:
      case 0x8b:
      case 0x8c:
      case 0x8d:
      case 0x8e:
      case 0x8f:
	// LDRH Rd, [Rs, #Imm]
	{
	  unsigned int address = reg[(opcode>>3)&7].I + (((opcode>>6)&31)<<1);
	  CPUReadHalfWordMoore(address);
	}
	break;
      case 0x90:
	// STR R0, [SP, #Imm]
	{
	  unsigned int address = reg[13].I + ((opcode&255)<<2);
	  CPUWriteMemoryMoore(address, reg[0].I);
	}
	break;      
      case 0x91:
	// STR R1, [SP, #Imm]
	{
	  unsigned int address = reg[13].I + ((opcode&255)<<2);   
	  CPUWriteMemoryMoore(address, reg[1].I);
	}
	break;      
      case 0x92:
	// STR R2, [SP, #Imm]
	{
	  unsigned int address = reg[13].I + ((opcode&255)<<2);   
	  CPUWriteMemoryMoore(address, reg[2].I);
	}
	break;      
      case 0x93:
	// STR R3, [SP, #Imm]
	{
	  unsigned int address = reg[13].I + ((opcode&255)<<2);   
	  CPUWriteMemoryMoore(address, reg[3].I);
	}
	break;      
      case 0x94:
	// STR R4, [SP, #Imm]
	{
	  unsigned int address = reg[13].I + ((opcode&255)<<2);   
	  CPUWriteMemoryMoore(address, reg[4].I);
	}
	break;      
      case 0x95:
	// STR R5, [SP, #Imm]
	{
	  unsigned int address = reg[13].I + ((opcode&255)<<2);   
	  CPUWriteMemoryMoore(address, reg[5].I);
	}
	break;      
      case 0x96:
	// STR R6, [SP, #Imm]
	{
	  unsigned int address = reg[13].I + ((opcode&255)<<2);   
	  CPUWriteMemoryMoore(address, reg[6].I);
	}
	break;      
      case 0x97:
	// STR R7, [SP, #Imm]
	{
	  unsigned int address = reg[13].I + ((opcode&255)<<2);   
	  CPUWriteMemoryMoore(address, reg[7].I);
	}
	break;   
      case 0x98:
	// LDR R0, [SP, #Imm]
	{
	  unsigned int address = reg[13].I + ((opcode&255)<<2);   
	  CPUReadMemoryMoore(address);
	}
	break;
      case 0x99:
	// LDR R1, [SP, #Imm]
	{
	  unsigned int address = reg[13].I + ((opcode&255)<<2);   
	  CPUReadMemoryMoore(address);
	}
	break;
      case 0x9a:
	// LDR R2, [SP, #Imm]
	{
	  unsigned int address = reg[13].I + ((opcode&255)<<2);   
	  CPUReadMemoryMoore(address);
	}
	break;
      case 0x9b:
	// LDR R3, [SP, #Imm]
	{
	  unsigned int address = reg[13].I + ((opcode&255)<<2);   
	  CPUReadMemoryMoore(address);
	}
	break;
      case 0x9c:
	// LDR R4, [SP, #Imm]
	{
	  unsigned int address = reg[13].I + ((opcode&255)<<2);   
	  CPUReadMemoryMoore(address);
	}
	break;
      case 0x9d:
	// LDR R5, [SP, #Imm]
	{
	  unsigned int address = reg[13].I + ((opcode&255)<<2);   
	  CPUReadMemoryMoore(address);
	}
	break;
      case 0x9e:
	// LDR R6, [SP, #Imm]
	{
	  unsigned int address = reg[13].I + ((opcode&255)<<2);   
	  CPUReadMemoryMoore(address);
	}
	break;
      case 0x9f:
	// LDR R7, [SP, #Imm]
	{
	  unsigned int address = reg[13].I + ((opcode&255)<<2);   
	  CPUReadMemoryMoore(address);
	}
	break;
      case 0xb4:
	// PUSH {Rlist}
	{
	  switch(mFsmState) {
	  case COMPUTEADDRESS:
	    break;
	  case REGISTER:
	    if (-1 != num_register) {
#ifdef DEBUG
	      fprintf(logfile,"sending R[%d] = %x to %x\n",num_register, reg[num_register].I, dm_address-4);
#endif
	      CPUWriteMemoryMoore(dm_address-4, reg[num_register].I);
	    }
	    break;
	  case END:
	    break;		
	  }
	}
	break;
      case 0xb5:
	// PUSH {Rlist, LR}
	{
	  switch(mFsmState) {
	  case COMPUTEADDRESS:
	    break;
	  case REGISTER:
	    if (-1 != num_register) {
#ifdef DEBUG
	      fprintf(logfile,"sending R[%d] = %x to %x\n",num_register,reg[num_register].I,dm_address-4);
#endif
	      CPUWriteMemoryMoore(dm_address-4, reg[num_register].I);
	    }
	    break;
	  case END:
	    break;		
	  }
	}
	break;
      case 0xbc:
	// POP {Rlist}
	{
	  switch(mFsmState) {
	  case COMPUTEADDRESS:
#ifdef DEBUG
	    fprintf(logfile,"getting from %x\n",dm_address.read());
#endif
	    CPUReadMemoryMoore(dm_address);
	    break;
	  case REGISTER:
#ifdef DEBUG
	    fprintf(logfile,"getting from %x\n",dm_address.read());
#endif
	    CPUReadMemoryMoore(dm_address);
	    break;
	  case END:
	    break;
	  }
	}
	break;   
      case 0xbd:
	// POP {Rlist, PC}
	{
	  switch(mFsmState) {
	  case COMPUTEADDRESS:
#ifdef DEBUG
	    fprintf(logfile,"getting from %x\n",dm_address.read());
#endif
	    CPUReadMemoryMoore(dm_address);
	    break;
	  case REGISTER:
#ifdef DEBUG
	    fprintf(logfile,"getting from %x\n",dm_address.read());
#endif
	    CPUReadMemoryMoore(dm_address);
	    break;
	  case END:
	    break;
	  }
	}
	break;      

      case 0xc0: // STM R0!, {Rlist}
      case 0xc1: // STM R1!, {Rlist}
      case 0xc2: // STM R2!, {Rlist}
      case 0xc3: // STM R3!, {Rlist}
      case 0xc4: // STM R4!, {Rlist}
      case 0xc5: // STM R5!, {Rlist}
      case 0xc6: // STM R6!, {Rlist}
      case 0xc7: // STM R7!, {Rlist}
	switch(mFsmState) {
	case COMPUTEADDRESS:
	  break;
	case REGISTER:
	  if (-1 != num_register) {
#ifdef DEBUG
	    fprintf(logfile,"sending R[%d] = %x to %x\n",num_register, reg[num_register].I, dm_address-4);
#endif
	    CPUWriteMemoryMoore(dm_address-4, reg[num_register].I);
	  }
	  break;
	case END:
	  break;		
	}
	break;

      case 0xc8: // LDM R0!, {Rlist}
      case 0xc9: // LDM R1!, {Rlist}
      case 0xca: // LDM R2!, {Rlist}
      case 0xcb: // LDM R3!, {Rlist}
      case 0xcc: // LDM R4!, {Rlist}
      case 0xcd: // LDM R5!, {Rlist}
      case 0xce: // LDM R6!, {Rlist}
      case 0xcf: // LDM R7!, {Rlist}
	switch(mFsmState) {
	case COMPUTEADDRESS:
#ifdef DEBUG
	  fprintf(logfile,"getting from %x\n",dm_address.read());
#endif
	  CPUReadMemoryMoore(dm_address);
	  break;
	case REGISTER:
#ifdef DEBUG
	  fprintf(logfile,"getting from %x\n",dm_address.read());
#endif
	  CPUReadMemoryMoore(dm_address);
	  break;
	case END:
	  break;
	}
	break;

      default:
	DCACHE.REQ = false;
	DCACHE.UNC = false;
	DCACHE.ADR   = 0;
	DCACHE.TYPE  = 0;
	DCACHE.WDATA = 0;
	break;
	// END
      }
    } else {
#ifdef DEBUG
      if (ICACHE.FRZ==false && DCACHE.FRZ==false)
	fprintf(logfile,"Moore opcode: 0x%x\n",opcode);
#endif


      int cond = opcode >> 28;
      // suggested optimization for frequent cases
      bool cond_res;
 
      if (cond == 0x0e)
	{
	  cond_res = true;
	}
      else
	{
	  switch (cond)
	    {
	    case 0x00:		// EQ
	      cond_res = Z_FLAG;
	      break;
	    case 0x01:		// NE
	      cond_res = !Z_FLAG;
	      break;
	    case 0x02:		// CS
	      cond_res = C_FLAG;
	      break;
	    case 0x03:		// CC
	      cond_res = !C_FLAG;
	      break;
	    case 0x04:		// MI
	      cond_res = N_FLAG;
	      break;
	    case 0x05:		// PL
	      cond_res = !N_FLAG;
	      break;
	    case 0x06:		// VS
	      cond_res = V_FLAG;
	      break;
	    case 0x07:		// VC
	      cond_res = !V_FLAG;
	      break;
	    case 0x08:		// HI
	      cond_res = C_FLAG && !Z_FLAG;
	      break;
	    case 0x09:		// LS
	      cond_res = !C_FLAG || Z_FLAG;
	      break;
	    case 0x0A:		// GE
	      cond_res = N_FLAG == V_FLAG;
	      break;
	    case 0x0B:		// LT
	      cond_res = N_FLAG != V_FLAG;
	      break;
	    case 0x0C:		// GT
	      cond_res = !Z_FLAG && (N_FLAG == V_FLAG);
	      break;
	    case 0x0D:		// LE
	      cond_res = Z_FLAG || (N_FLAG != V_FLAG);
	      break;
	    case 0x0E:
	      cond_res = true;
	      break;
	    case 0x0F:
	    default:
	      // ???
	      cond_res = false;
	      break;
	    }
	}
      if (cond_res)
	{
	  unsigned int sv=((opcode >> 16) & 0xFF0) | ((opcode >> 4) & 0x0F);

#ifdef DEBUG
	  if (ICACHE.FRZ==false && DCACHE.FRZ==false)
	    fprintf(logfile,"svMoore=%x\n",sv);
#endif

	  //switch (((opcode >> 16) & 0xFF0) | ((opcode >> 4) & 0x0F))
	  switch(sv)
	    {
	    case 0x00b:
	    case 0x02b:
	      {
		// STRH Rd, [Rn], -Rm
		int base = (opcode >> 16) & 0x0F;
		int dest = (opcode >> 12) & 0x0F;
		unsigned int address = reg[base].I;
		CPUWriteHalfWordMoore (address, reg[dest].W.W0);
	      }
	      break;
	    case 0x04b:
	    case 0x06b:
	      {
		// STRH Rd, [Rn], #-offset
		int base = (opcode >> 16) & 0x0F;
		int dest = (opcode >> 12) & 0x0F;
		unsigned int address = reg[base].I;
		CPUWriteHalfWordMoore (address, reg[dest].W.W0);
	      }
	      break;
	    case 0x08b:
	    case 0x0ab:
	      {
		// STRH Rd, [Rn], Rm
		int base = (opcode >> 16) & 0x0F;
		int dest = (opcode >> 12) & 0x0F;
		unsigned int address = reg[base].I;
		CPUWriteHalfWordMoore (address, reg[dest].W.W0);
	      }
	      break;
	    case 0x0cb:
	    case 0x0eb:
	      {
		// STRH Rd, [Rn], #offset
		int base = (opcode >> 16) & 0x0F;
		int dest = (opcode >> 12) & 0x0F;
		unsigned int address = reg[base].I;
		CPUWriteHalfWordMoore (address, reg[dest].W.W0);
	      }
	      break;
	    case 0x10b:
	      {
		// STRH Rd, [Rn, -Rm]
		int base = (opcode >> 16) & 0x0F;
		int dest = (opcode >> 12) & 0x0F;
		unsigned int address = reg[base].I - reg[opcode & 0x0F].I;
		CPUWriteHalfWordMoore (address, reg[dest].W.W0);
	      }
	      break;
	    case 0x12b:
	      {
		// STRH Rd, [Rn, -Rm]!
		int base = (opcode >> 16) & 0x0F;
		int dest = (opcode >> 12) & 0x0F;
		unsigned int address = reg[base].I - reg[opcode & 0x0F].I;
		CPUWriteHalfWordMoore (address, reg[dest].W.W0);
	      }
	      break;
	    case 0x14b:
	      {
		// STRH Rd, [Rn, -#offset]
		int base = (opcode >> 16) & 0x0F;
		int dest = (opcode >> 12) & 0x0F;
		unsigned int address =
		  reg[base].I - ((opcode & 0x0F) | ((opcode >> 4) & 0xF0));
		CPUWriteHalfWordMoore (address, reg[dest].W.W0);
	      }
	      break;
	    case 0x16b:
	      {
		// STRH Rd, [Rn, -#offset]!
		int base = (opcode >> 16) & 0x0F;
		int dest = (opcode >> 12) & 0x0F;
		unsigned int address =
		  reg[base].I - ((opcode & 0x0F) | ((opcode >> 4) & 0xF0));
		CPUWriteHalfWordMoore (address, reg[dest].W.W0);
	      }
	      break;
	    case 0x18b:
	      {
		// STRH Rd, [Rn, Rm]
		int base = (opcode >> 16) & 0x0F;
		int dest = (opcode >> 12) & 0x0F;
		unsigned int address = reg[base].I + reg[opcode & 0x0F].I;
		CPUWriteHalfWordMoore (address, reg[dest].W.W0);
	      }
	      break;
	    case 0x1ab:
	      {
		// STRH Rd, [Rn, Rm]!
		int base = (opcode >> 16) & 0x0F;
		int dest = (opcode >> 12) & 0x0F;
		unsigned int address = reg[base].I + reg[opcode & 0x0F].I;
		CPUWriteHalfWordMoore (address, reg[dest].W.W0);
	      }
	      break;
	    case 0x1cb:
	      {
		// STRH Rd, [Rn, #offset]
		int base = (opcode >> 16) & 0x0F;
		int dest = (opcode >> 12) & 0x0F;
		unsigned int address =
		  reg[base].I + ((opcode & 0x0F) | ((opcode >> 4) & 0xF0));
		CPUWriteHalfWordMoore (address, reg[dest].W.W0);
	      }
	      break;
	    case 0x1eb:
	      {
		// STRH Rd, [Rn, #offset]!
		int base = (opcode >> 16) & 0x0F;
		int dest = (opcode >> 12) & 0x0F;
		unsigned int address =
		  reg[base].I + ((opcode & 0x0F) | ((opcode >> 4) & 0xF0));
		CPUWriteHalfWordMoore (address, reg[dest].W.W0);
	      }
	      break;
	    case 0x01b:
	    case 0x03b:
	      {
		// LDRH Rd, [Rn], -Rm
		int base = (opcode >> 16) & 0x0F;
		unsigned int address = reg[base].I;
		CPUReadHalfWordMoore (address);
	      }
	      break;
	    case 0x05b:
	    case 0x07b:
	      {
		// LDRH Rd, [Rn], #-offset
		int base = (opcode >> 16) & 0x0F;
		unsigned int address = reg[base].I;
		CPUReadHalfWordMoore (address);
	      }
	      break;
	    case 0x09b:
	    case 0x0bb:
	      {
		// LDRH Rd, [Rn], Rm
		int base = (opcode >> 16) & 0x0F;
		unsigned int address = reg[base].I;
		CPUReadHalfWordMoore (address);
	      }
	      break;
	    case 0x0db:
	    case 0x0fb:
	      {
		// LDRH Rd, [Rn], #offset
		int base = (opcode >> 16) & 0x0F;
		unsigned int address = reg[base].I;
		CPUReadHalfWordMoore (address);
	      }
	      break;
	    case 0x11b:
	      {
		// LDRH Rd, [Rn, -Rm]
		int base = (opcode >> 16) & 0x0F;
		unsigned int address = reg[base].I - reg[opcode & 0x0F].I;
		CPUReadHalfWordMoore (address);
	      }
	      break;
	    case 0x13b:
	      {
		// LDRH Rd, [Rn, -Rm]!
		int base = (opcode >> 16) & 0x0F;
		unsigned int address = reg[base].I - reg[opcode & 0x0F].I;
		CPUReadHalfWordMoore (address);
	      }
	      break;
	    case 0x15b:
	      {
		// LDRH Rd, [Rn, -#offset]
		int base = (opcode >> 16) & 0x0F;
		unsigned int address =
		  reg[base].I - ((opcode & 0x0F) | ((opcode >> 4) & 0xF0));
		CPUReadHalfWordMoore (address);
	      }
	      break;
	    case 0x17b:
	      {
		// LDRH Rd, [Rn, -#offset]!
		int base = (opcode >> 16) & 0x0F;
		unsigned int address =
		  reg[base].I - ((opcode & 0x0F) | ((opcode >> 4) & 0xF0));
		CPUReadHalfWordMoore (address);
	      }
	      break;
	    case 0x19b:
	      {
		// LDRH Rd, [Rn, Rm]
		int base = (opcode >> 16) & 0x0F;
		unsigned int address = reg[base].I + reg[opcode & 0x0F].I;
		CPUReadHalfWordMoore (address);
	      }
	      break;
	    case 0x1bb:
	      {
		// LDRH Rd, [Rn, Rm]!
		int base = (opcode >> 16) & 0x0F;
		unsigned int address = reg[base].I + reg[opcode & 0x0F].I;
		CPUReadHalfWordMoore (address);
	      }
	      break;
	    case 0x1db:
	      {
		// LDRH Rd, [Rn, #offset]
		int base = (opcode >> 16) & 0x0F;
		unsigned int address =
		  reg[base].I + ((opcode & 0x0F) | ((opcode >> 4) & 0xF0));
		CPUReadHalfWordMoore (address);
	      }
	      break;
	    case 0x1fb:
	      {
		// LDRH Rd, [Rn, #offset]!
		int base = (opcode >> 16) & 0x0F;
		unsigned int address =
		  reg[base].I + ((opcode & 0x0F) | ((opcode >> 4) & 0xF0));
		CPUReadHalfWordMoore (address);
	      }
	      break;
	    case 0x01d:
	    case 0x03d:
	      {
		// LDRSB Rd, [Rn], -Rm
		int base = (opcode >> 16) & 0x0F;
		unsigned int address = reg[base].I;
		CPUReadByteMoore (address);
	      }
	      break;
	    case 0x05d:
	    case 0x07d:
	      {
		// LDRSB Rd, [Rn], #-offset
		int base = (opcode >> 16) & 0x0F;
		unsigned int address = reg[base].I;
		CPUReadByteMoore (address);
	      }
	      break;
	    case 0x09d:
	    case 0x0bd:
	      {
		// LDRSB Rd, [Rn], Rm
		int base = (opcode >> 16) & 0x0F;
		unsigned int address = reg[base].I;
		CPUReadByteMoore (address);
	      }
	      break;
	    case 0x0dd:
	    case 0x0fd:
	      {
		// LDRSB Rd, [Rn], #offset
		int base = (opcode >> 16) & 0x0F;
		unsigned int address = reg[base].I;
		CPUReadByteMoore (address);
	      }
	      break;
	    case 0x11d:
	      {
		// LDRSB Rd, [Rn, -Rm]
		int base = (opcode >> 16) & 0x0F;
		unsigned int address = reg[base].I - reg[opcode & 0x0F].I;
		CPUReadByteMoore (address);
	      }
	      break;
	    case 0x13d:
	      {
		// LDRSB Rd, [Rn, -Rm]!
		int base = (opcode >> 16) & 0x0F;
		unsigned int address = reg[base].I - reg[opcode & 0x0F].I;
		CPUReadByteMoore (address);
	      }
	      break;
	    case 0x15d:
	      {
		// LDRSB Rd, [Rn, -#offset]
		int base = (opcode >> 16) & 0x0F;
		unsigned int address =
		  reg[base].I - ((opcode & 0x0F) | ((opcode >> 4) & 0xF0));
		CPUReadByteMoore (address);
	      }
	      break;
	    case 0x17d:
	      {
		// LDRSB Rd, [Rn, -#offset]!
		int base = (opcode >> 16) & 0x0F;
		unsigned int address =
		  reg[base].I - ((opcode & 0x0F) | ((opcode >> 4) & 0xF0));
		CPUReadByteMoore (address);
	      }
	      break;
	    case 0x19d:
	      {
		// LDRSB Rd, [Rn, Rm]
		int base = (opcode >> 16) & 0x0F;
		unsigned int address = reg[base].I + reg[opcode & 0x0F].I;
		CPUReadByteMoore (address);
	      }
	      break;
	    case 0x1bd:
	      {
		// LDRSB Rd, [Rn, Rm]!
		int base = (opcode >> 16) & 0x0F;
		unsigned int address = reg[base].I + reg[opcode & 0x0F].I;
		CPUReadByteMoore (address);
	      }
	      break;
	    case 0x1dd:
	      {
		// LDRSB Rd, [Rn, #offset]
		int base = (opcode >> 16) & 0x0F;
		unsigned int address =
		  reg[base].I + ((opcode & 0x0F) | ((opcode >> 4) & 0xF0));
		CPUReadByteMoore (address);
	      }
	      break;
	    case 0x1fd:
	      {
		// LDRSB Rd, [Rn, #offset]!
		int base = (opcode >> 16) & 0x0F;
		unsigned int address =
		  reg[base].I + ((opcode & 0x0F) | ((opcode >> 4) & 0xF0));
		CPUReadByteMoore (address);
	      }
	      break;
	    case 0x01f:
	    case 0x03f:
	      {
		// LDRSH Rd, [Rn], -Rm
		int base = (opcode >> 16) & 0x0F;
		unsigned int address = reg[base].I;
		CPUReadHalfWordSignedMoore (address);
	      }
	      break;
	    case 0x05f:
	    case 0x07f:
	      {
		// LDRSH Rd, [Rn], #-offset
		int base = (opcode >> 16) & 0x0F;
		unsigned int address = reg[base].I;
		CPUReadHalfWordSignedMoore (address);
	      }
	      break;
	    case 0x09f:
	    case 0x0bf:
	      {
		// LDRSH Rd, [Rn], Rm
		int base = (opcode >> 16) & 0x0F;
		unsigned int address = reg[base].I;
		CPUReadHalfWordSignedMoore (address);
	      }
	      break;
	    case 0x0df:
	    case 0x0ff:
	      {
		// LDRSH Rd, [Rn], #offset
		int base = (opcode >> 16) & 0x0F;
		unsigned int address = reg[base].I;
		CPUReadHalfWordSignedMoore (address);
	      }
	      break;
	    case 0x11f:
	      {
		// LDRSH Rd, [Rn, -Rm]
		int base = (opcode >> 16) & 0x0F;
		unsigned int address = reg[base].I - reg[opcode & 0x0F].I;
		CPUReadHalfWordSignedMoore (address);
	      }
	      break;
	    case 0x13f:
	      {
		// LDRSH Rd, [Rn, -Rm]!
		int base = (opcode >> 16) & 0x0F;
		unsigned int address = reg[base].I - reg[opcode & 0x0F].I;
		CPUReadHalfWordSignedMoore (address);
	      }
	      break;
	    case 0x15f:
	      {
		// LDRSH Rd, [Rn, -#offset]
		int base = (opcode >> 16) & 0x0F;
		unsigned int address =
		  reg[base].I - ((opcode & 0x0F) | ((opcode >> 4) & 0xF0));
		CPUReadHalfWordSignedMoore (address);
	      }
	      break;
	    case 0x17f:
	      {
		// LDRSH Rd, [Rn, -#offset]!
		int base = (opcode >> 16) & 0x0F;
		unsigned int address =
		  reg[base].I - ((opcode & 0x0F) | ((opcode >> 4) & 0xF0));
		CPUReadHalfWordSignedMoore (address);
	      }
	      break;
	    case 0x19f:
	      {
		// LDRSH Rd, [Rn, Rm]
		int base = (opcode >> 16) & 0x0F;
		unsigned int address = reg[base].I + reg[opcode & 0x0F].I;
		CPUReadHalfWordSignedMoore (address);
	      }
	      break;
	    case 0x1bf:
	      {
		// LDRSH Rd, [Rn, Rm]!
		int base = (opcode >> 16) & 0x0F;
		unsigned int address = reg[base].I + reg[opcode & 0x0F].I;
		CPUReadHalfWordSignedMoore (address);
	      }
	      break;
	    case 0x1df:
	      {
		// LDRSH Rd, [Rn, #offset]
		int base = (opcode >> 16) & 0x0F;
		unsigned int address =
		  reg[base].I + ((opcode & 0x0F) | ((opcode >> 4) & 0xF0));
		CPUReadHalfWordSignedMoore (address);
	      }
	      break;
	    case 0x1ff:
	      {
		// LDRSH Rd, [Rn, #offset]!
		int base = (opcode >> 16) & 0x0F;
		unsigned int address =
		  reg[base].I + ((opcode & 0x0F) | ((opcode >> 4) & 0xF0));
		CPUReadHalfWordSignedMoore (address);
	      }
	      break;
	    case 0x109:
	      {
		// SWP Rd, Rm, [Rn]
#ifdef DEBUG
		fprintf(logfile,"SWP Moore swpFsmState=%d\n",swpFsmState.read());
#endif
		unsigned int address = reg[(opcode >> 16) & 15].I;
		switch (swpFsmState) {
		case 0:
#ifdef DEBUG
		  fprintf(logfile,"SWP Moore 0\n");
#endif
		  CPUReadMemoryMoore (address);
		  break;
		case 1:
#ifdef DEBUG
		  fprintf(logfile,"SWP Moore 1 address=%x reg=%x\n",address,reg[opcode & 15].I);
#endif
		  CPUWriteMemoryMoore (address, reg[opcode & 15].I);
		  break;
		case 2:
#ifdef DEBUG
		  fprintf(logfile,"SWP Moore 2\n");
#endif
		  break; 
		default:
		  break;
		}
	      }
	      break;
	    case 0x149:
	      {
		// SWPB Rd, Rm, [Rn]
		unsigned int address = reg[(opcode >> 16) & 15].I;
		CPUWriteByteMoore (address, reg[opcode & 15].B.B0);
	      }
	      break;
	      CASE_16 (0x400)
		// T versions shouldn't be different on GBA      
		CASE_16 (0x420)
		{
		  // STR Rd, [Rn], -#
		  int dest = (opcode >> 12) & 15;
		  int base = (opcode >> 16) & 15;
		  unsigned int address = reg[base].I;
		  CPUWriteMemoryMoore (address, reg[dest].I);
		}
	      break;
	      CASE_16 (0x480)
		// T versions shouldn't be different on GBA
		CASE_16 (0x4a0)
		{
		  // STR Rd, [Rn], #
		  int dest = (opcode >> 12) & 15;
		  int base = (opcode >> 16) & 15;
		  unsigned int address = reg[base].I;
		  CPUWriteMemoryMoore (address, reg[dest].I);
		}
	      break;
	      CASE_16 (0x500)
		{
		  // STR Rd, [Rn, -#]
		  int offset = opcode & 0xFFF;
		  int dest = (opcode >> 12) & 15;
		  int base = (opcode >> 16) & 15;
		  unsigned int address = reg[base].I - offset;
		  CPUWriteMemoryMoore (address, reg[dest].I);
		}
	      break;
	      CASE_16 (0x520)
		{
		  // STR Rd, [Rn, -#]!
		  int offset = opcode & 0xFFF;
		  int dest = (opcode >> 12) & 15;
		  int base = (opcode >> 16) & 15;
		  unsigned int address = reg[base].I - offset;
		  CPUWriteMemoryMoore (address, reg[dest].I);
		}
	      break;
	      CASE_16 (0x580)
		{
		  // STR Rd, [Rn, #]
		  int offset = opcode & 0xFFF;
		  int dest = (opcode >> 12) & 15;
		  int base = (opcode >> 16) & 15;
		  unsigned int address = reg[base].I + offset;
		  CPUWriteMemoryMoore (address, reg[dest].I);
		}
	      break;
	      CASE_16 (0x5a0)
		{
		  // STR Rd, [Rn, #]!
		  int offset = opcode & 0xFFF;
		  int dest = (opcode >> 12) & 15;
		  int base = (opcode >> 16) & 15;
		  unsigned int address = reg[base].I + offset;
		  CPUWriteMemoryMoore (address, reg[dest].I);
		}
	      break;
	      CASE_16 (0x410)
		{
		  // LDR Rd, [Rn], -#
		  int base = (opcode >> 16) & 15;
		  unsigned int address = reg[base].I;
		  CPUReadMemoryMoore (address);
		}
	      break;
	      CASE_16 (0x430)
		{
		  // LDRT Rd, [Rn], -#
		  int base = (opcode >> 16) & 15;
		  unsigned int address = reg[base].I;
		  CPUReadMemoryMoore (address);
		}
	      break;
	      CASE_16 (0x490)
		{
		  // LDR Rd, [Rn], #
		  int base = (opcode >> 16) & 15;
		  unsigned int address = reg[base].I;
		  CPUReadMemoryMoore (address);
		}
	      break;
	      CASE_16 (0x4b0)
		{
		  // LDRT Rd, [Rn], #
		  int base = (opcode >> 16) & 15;
		  unsigned int address = reg[base].I;
		  CPUReadMemoryMoore (address);
		}
	      break;
	      CASE_16 (0x510)
		{
		  // LDR Rd, [Rn, -#]
		  int offset = opcode & 0xFFF;
		  int base = (opcode >> 16) & 15;
		  unsigned int address = reg[base].I - offset;
		  CPUReadMemoryMoore (address);
		}
	      break;
	      CASE_16 (0x530)
		{
		  // LDR Rd, [Rn, -#]!
		  int offset = opcode & 0xFFF;
		  int base = (opcode >> 16) & 15;
		  unsigned int address = reg[base].I - offset;
		  CPUReadMemoryMoore (address);
		}
	      break;
	      CASE_16 (0x590)
		{
#ifdef DEBUG
		  fprintf(logfile,"LDR Rd,[Rn,#]\n");
#endif
		  // LDR Rd, [Rn, #]
		  int offset = opcode & 0xFFF;
		  int base = (opcode >> 16) & 15;
		  unsigned int address = reg[base].I + offset;
		  CPUReadMemoryMoore (address);
		}
	      break;
	      CASE_16 (0x5b0)
		{
		  // LDR Rd, [Rn, #]!
		  int offset = opcode & 0xFFF;
		  int base = (opcode >> 16) & 15;
		  unsigned int address = reg[base].I + offset;
		  CPUReadMemoryMoore (address);
		}
	      break;
	      CASE_16 (0x440)
		// T versions shouldn't be different on GBA      
		CASE_16 (0x460)
		{
		  // STRB Rd, [Rn], -#
		  int dest = (opcode >> 12) & 15;
		  int base = (opcode >> 16) & 15;
		  unsigned int address = reg[base].I;
		  CPUWriteByteMoore (address, reg[dest].B.B0);
		}
	      break;
	      CASE_16 (0x4c0)
		// T versions shouldn't be different on GBA
		CASE_16 (0x4e0)
		// STRB Rd, [Rn], #
		{
		  int dest = (opcode >> 12) & 15;
		  int base = (opcode >> 16) & 15;
		  unsigned int address = reg[base].I;
		  CPUWriteByteMoore (address, reg[dest].B.B0);
		}
	      break;
	      CASE_16 (0x540)
		{
		  // STRB Rd, [Rn, -#]
		  int offset = opcode & 0xFFF;
		  int dest = (opcode >> 12) & 15;
		  int base = (opcode >> 16) & 15;
		  unsigned int address = reg[base].I - offset;
		  CPUWriteByteMoore (address, reg[dest].B.B0);
		}
	      break;
	      CASE_16 (0x560)
		{
		  // STRB Rd, [Rn, -#]!
		  int offset = opcode & 0xFFF;
		  int dest = (opcode >> 12) & 15;
		  int base = (opcode >> 16) & 15;
		  unsigned int address = reg[base].I - offset;
		  reg[base].I = address;
		  CPUWriteByteMoore (address, reg[dest].B.B0);
		}
	      break;
	      CASE_16 (0x5c0)
		{
		  // STRB Rd, [Rn, #]
		  int offset = opcode & 0xFFF;
		  int dest = (opcode >> 12) & 15;
		  int base = (opcode >> 16) & 15;
		  unsigned int address = reg[base].I + offset;
#ifdef DEBUG
		  fprintf(logfile,"address: 0x%x || value: %x\n",address, reg[dest].B.B0);
#endif
		  CPUWriteByteMoore (address, reg[dest].B.B0);
		}
	      break;
	      CASE_16 (0x5e0)
		{
		  // STRB Rd, [Rn, #]!
		  int offset = opcode & 0xFFF;
		  int dest = (opcode >> 12) & 15;
		  int base = (opcode >> 16) & 15;
		  unsigned int address = reg[base].I + offset;
		  CPUWriteByteMoore (address, reg[dest].I);
		}
	      break;
	      CASE_16 (0x450)
		// T versions shouldn't be different
		CASE_16 (0x470)
		{
		  // LDRB Rd, [Rn], -#
		  int base = (opcode >> 16) & 15;
		  unsigned int address = reg[base].I;
		  CPUReadByteMoore (address);
		}
	      break;
	      CASE_16 (0x4d0) CASE_16 (0x4f0)	// T versions should not be different
		{
		  // LDRB Rd, [Rn], #
		  int base = (opcode >> 16) & 15;
		  unsigned int address = reg[base].I;
		  CPUReadByteMoore (address);
		}
	      break;
	      CASE_16 (0x550)
		{
		  // LDRB Rd, [Rn, -#]
		  int offset = opcode & 0xFFF;
		  int base = (opcode >> 16) & 15;
		  unsigned int address = reg[base].I - offset;
		  CPUReadByteMoore (address);
		}
	      break;
	      CASE_16 (0x570)
		{
		  // LDRB Rd, [Rn, -#]!
		  int offset = opcode & 0xFFF;
		  int base = (opcode >> 16) & 15;
		  unsigned int address = reg[base].I - offset;
		  CPUReadByteMoore (address);
		}
	      break;
	      CASE_16 (0x5d0)
		{
		  // LDRB Rd, [Rn, #]
		  int offset = opcode & 0xFFF;
		  int base = (opcode >> 16) & 15;
		  unsigned int address = reg[base].I + offset;
		  CPUReadByteMoore (address);
		}
	      break;
	      CASE_16 (0x5f0)
		{
		  // LDRB Rd, [Rn, #]!
		  int offset = opcode & 0xFFF;
		  int base = (opcode >> 16) & 15;
		  unsigned int address = reg[base].I + offset;
		  CPUReadByteMoore (address);
		}
	      break;
	    case 0x600:
	    case 0x608:
	      // T versions are the same
	    case 0x620:
	    case 0x628:
	      {
		// STR Rd, [Rn], -Rm, LSL #
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I;
		CPUWriteMemoryMoore (address, reg[dest].I);
	      }
	      break;
	    case 0x602:
	    case 0x60a:
	      // T versions are the same
	    case 0x622:
	    case 0x62a:
	      {
		// STR Rd, [Rn], -Rm, LSR #
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I;
		CPUWriteMemoryMoore (address, reg[dest].I);
	      }
	      break;
	    case 0x604:
	    case 0x60c:
	      // T versions are the same
	    case 0x624:
	    case 0x62c:
	      {
		// STR Rd, [Rn], -Rm, ASR #
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I;
		CPUWriteMemoryMoore (address, reg[dest].I);
	      }
	      break;
	    case 0x606:
	    case 0x60e:
	      // T versions are the same
	    case 0x626:
	    case 0x62e:
	      {
		// STR Rd, [Rn], -Rm, ROR #
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I;
		CPUWriteMemoryMoore (address, reg[dest].I);
	      }
	      break;
	    case 0x680:
	    case 0x688:
	      // T versions are the same
	    case 0x6a0:
	    case 0x6a8:
	      {
		// STR Rd, [Rn], Rm, LSL #
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I;
		CPUWriteMemoryMoore (address, reg[dest].I);
	      }
	      break;
	    case 0x682:
	    case 0x68a:
	      // T versions are the same
	    case 0x6a2:
	    case 0x6aa:
	      {
		// STR Rd, [Rn], Rm, LSR #
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I;
		CPUWriteMemoryMoore (address, reg[dest].I);
	      }
	      break;
	    case 0x684:
	    case 0x68c:
	      // T versions are the same
	    case 0x6a4:
	    case 0x6ac:
	      {
		// STR Rd, [Rn], Rm, ASR #
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I;
		CPUWriteMemoryMoore (address, reg[dest].I);
	      }
	      break;
	    case 0x686:
	    case 0x68e:
	      // T versions are the same
	    case 0x6a6:
	    case 0x6ae:
	      {
		// STR Rd, [Rn], Rm, ROR #
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I;
		CPUWriteMemoryMoore (address, reg[dest].I);
	      }
	      break;
	    case 0x700:
	    case 0x708:
	      {
		// STR Rd, [Rn, -Rm, LSL #]
		int offset = reg[opcode & 15].I << ((opcode >> 7) & 31);
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I - offset;
		CPUWriteMemoryMoore (address, reg[dest].I);
	      }
	      break;
	    case 0x702:
	    case 0x70a:
	      {
		// STR Rd, [Rn, -Rm, LSR #]
		int shift = (opcode >> 7) & 31;
		int offset = shift ? reg[opcode & 15].I >> shift : 0;
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I - offset;
		CPUWriteMemoryMoore (address, reg[dest].I);
	      }
	      break;
	    case 0x704:
	    case 0x70c:
	      {
		// STR Rd, [Rn, -Rm, ASR #]
		int shift = (opcode >> 7) & 31;
		int offset;
		if (shift)
		  offset = (int) ((signed int) reg[opcode & 15].I >> shift);
		else if (reg[opcode & 15].I & 0x80000000)
		  offset = 0xFFFFFFFF;
		else
		  offset = 0;
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I - offset;
		CPUWriteMemoryMoore (address, reg[dest].I);
	      }
	      break;
	    case 0x706:
	    case 0x70e:
	      {
		// STR Rd, [Rn, -Rm, ROR #]
		int shift = (opcode >> 7) & 31;
		unsigned int value = reg[opcode & 15].I;
		if (shift)
		  {
		    ROR_VALUE;
		  }
		else
		  {
		    RCR_VALUE;
		  }
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I - value;
		CPUWriteMemoryMoore (address, reg[dest].I);
	      }
	      break;
	    case 0x720:
	    case 0x728:
	      {
		// STR Rd, [Rn, -Rm, LSL #]!
		int offset = reg[opcode & 15].I << ((opcode >> 7) & 31);
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I - offset;
		reg[base].I = address;
		CPUWriteMemoryMoore (address, reg[dest].I);
	      }
	      break;
	    case 0x722:
	    case 0x72a:
	      {
		// STR Rd, [Rn, -Rm, LSR #]!
		int shift = (opcode >> 7) & 31;
		int offset = shift ? reg[opcode & 15].I >> shift : 0;
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I - offset;
		reg[base].I = address;
		CPUWriteMemoryMoore (address, reg[dest].I);
	      }
	      break;
	    case 0x724:
	    case 0x72c:
	      {
		// STR Rd, [Rn, -Rm, ASR #]!
		int shift = (opcode >> 7) & 31;
		int offset;
		if (shift)
		  offset = (int) ((signed int) reg[opcode & 15].I >> shift);
		else if (reg[opcode & 15].I & 0x80000000)
		  offset = 0xFFFFFFFF;
		else
		  offset = 0;
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I - offset;
		CPUWriteMemoryMoore (address, reg[dest].I);
	      }
	      break;
	    case 0x726:
	    case 0x72e:
	      {
		// STR Rd, [Rn, -Rm, ROR #]!
		int shift = (opcode >> 7) & 31;
		unsigned int value = reg[opcode & 15].I;
		if (shift)
		  {
		    ROR_VALUE;
		  }
		else
		  {
		    RCR_VALUE;
		  }
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I - value;
		CPUWriteMemoryMoore (address, reg[dest].I);
	      }
	      break;
	    case 0x780:
	    case 0x788:
	      {
		// STR Rd, [Rn, Rm, LSL #]
		int offset = reg[opcode & 15].I << ((opcode >> 7) & 31);
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I + offset;
		CPUWriteMemoryMoore (address, reg[dest].I);
	      }
	      break;
	    case 0x782:
	    case 0x78a:
	      {
		// STR Rd, [Rn, Rm, LSR #]
		int shift = (opcode >> 7) & 31;
		int offset = shift ? reg[opcode & 15].I >> shift : 0;
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I + offset;
		CPUWriteMemoryMoore (address, reg[dest].I);
	      }
	      break;
	    case 0x784:
	    case 0x78c:
	      {
		// STR Rd, [Rn, Rm, ASR #]
		int shift = (opcode >> 7) & 31;
		int offset;
		if (shift)
		  offset = (int) ((signed int) reg[opcode & 15].I >> shift);
		else if (reg[opcode & 15].I & 0x80000000)
		  offset = 0xFFFFFFFF;
		else
		  offset = 0;
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I + offset;
		CPUWriteMemoryMoore (address, reg[dest].I);
	      }
	      break;
	    case 0x786:
	    case 0x78e:
	      {
		// STR Rd, [Rn, Rm, ROR #]
		int shift = (opcode >> 7) & 31;
		unsigned int value = reg[opcode & 15].I;
		if (shift)
		  {
		    ROR_VALUE;
		  }
		else
		  {
		    RCR_VALUE;
		  }
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I + value;
		CPUWriteMemoryMoore (address, reg[dest].I);
	      }
	      break;
	    case 0x7a0:
	    case 0x7a8:
	      {
		// STR Rd, [Rn, Rm, LSL #]!
		int offset = reg[opcode & 15].I << ((opcode >> 7) & 31);
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I + offset;
		CPUWriteMemoryMoore (address, reg[dest].I);
	      }
	      break;
	    case 0x7a2:
	    case 0x7aa:
	      {
		// STR Rd, [Rn, Rm, LSR #]!
		int shift = (opcode >> 7) & 31;
		int offset = shift ? reg[opcode & 15].I >> shift : 0;
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I + offset;
		CPUWriteMemoryMoore (address, reg[dest].I);
	      }
	      break;
	    case 0x7a4:
	    case 0x7ac:
	      {
		// STR Rd, [Rn, Rm, ASR #]!
		int shift = (opcode >> 7) & 31;
		int offset;
		if (shift)
		  offset = (int) ((signed int) reg[opcode & 15].I >> shift);
		else if (reg[opcode & 15].I & 0x80000000)
		  offset = 0xFFFFFFFF;
		else
		  offset = 0;
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I + offset;
		CPUWriteMemoryMoore (address, reg[dest].I);
	      }
	      break;
	    case 0x7a6:
	    case 0x7ae:
	      {
		// STR Rd, [Rn, Rm, ROR #]!
		int shift = (opcode >> 7) & 31;
		unsigned int value = reg[opcode & 15].I;
		if (shift)
		  {
		    ROR_VALUE;
		  }
		else
		  {
		    RCR_VALUE;
		  }
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I + value;
		CPUWriteMemoryMoore (address, reg[dest].I);
	      }
	      break;
	    case 0x610:
	    case 0x618:
	      // T versions are the same
	    case 0x630:
	    case 0x638:
	      {
		// LDR Rd, [Rn], -Rm, LSL #
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I;
		CPUReadMemoryMoore (address);
	      }
	      break;
	    case 0x612:
	    case 0x61a:
	      // T versions are the same
	    case 0x632:
	    case 0x63a:
	      {
		// LDR Rd, [Rn], -Rm, LSR #
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I;
		CPUReadMemoryMoore (address);
	      }
	      break;
	    case 0x614:
	    case 0x61c:
	      // T versions are the same
	    case 0x634:
	    case 0x63c:
	      {
		// LDR Rd, [Rn], -Rm, ASR #
		int shift = (opcode >> 7) & 31;
		int offset;
		if (shift)
		  offset = (int) ((signed int) reg[opcode & 15].I >> shift);
		else if (reg[opcode & 15].I & 0x80000000)
		  offset = 0xFFFFFFFF;
		else
		  offset = 0;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I;
		CPUReadMemoryMoore (address);
	      }
	      break;
	    case 0x616:
	    case 0x61e:
	      // T versions are the same
	    case 0x636:
	    case 0x63e:
	      {
		// LDR Rd, [Rn], -Rm, ROR #
		int shift = (opcode >> 7) & 31;
		unsigned int value = reg[opcode & 15].I;
		if (shift)
		  {
		    ROR_VALUE;
		  }
		else
		  {
		    RCR_VALUE;
		  }
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I;
		CPUReadMemoryMoore (address);
	      }
	      break;
	    case 0x690:
	    case 0x698:
	      // T versions are the same
	    case 0x6b0:
	    case 0x6b8:
	      {
		// LDR Rd, [Rn], Rm, LSL #
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I;
		CPUReadMemoryMoore (address);
	      }
	      break;
	    case 0x692:
	    case 0x69a:
	      // T versions are the same
	    case 0x6b2:
	    case 0x6ba:
	      {
		// LDR Rd, [Rn], Rm, LSR #
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I;
		CPUReadMemoryMoore (address);
	      }
	      break;
	    case 0x694:
	    case 0x69c:
	      // T versions are the same
	    case 0x6b4:
	    case 0x6bc:
	      {
		// LDR Rd, [Rn], Rm, ASR #
		int shift = (opcode >> 7) & 31;
		int offset;
		if (shift)
		  offset = (int) ((signed int) reg[opcode & 15].I >> shift);
		else if (reg[opcode & 15].I & 0x80000000)
		  offset = 0xFFFFFFFF;
		else
		  offset = 0;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I;
		CPUReadMemoryMoore (address);
	      }
	      break;
	    case 0x696:
	    case 0x69e:
	      // T versions are the same
	    case 0x6b6:
	    case 0x6be:
	      {
		// LDR Rd, [Rn], Rm, ROR #
		int shift = (opcode >> 7) & 31;
		unsigned int value = reg[opcode & 15].I;
		if (shift)
		  {
		    ROR_VALUE;
		  }
		else
		  {
		    RCR_VALUE;
		  }
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I;
		CPUReadMemoryMoore (address);
	      }
	      break;
	    case 0x710:
	    case 0x718:
	      {
		// LDR Rd, [Rn, -Rm, LSL #]
		int offset = reg[opcode & 15].I << ((opcode >> 7) & 31);
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I - offset;
		CPUReadMemoryMoore (address);
	      }
	      break;
	    case 0x712:
	    case 0x71a:
	      {
		// LDR Rd, [Rn, -Rm, LSR #]
		int shift = (opcode >> 7) & 31;
		int offset = shift ? reg[opcode & 15].I >> shift : 0;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I - offset;
		CPUReadMemoryMoore (address);
	      }
	      break;
	    case 0x714:
	    case 0x71c:
	      {
		// LDR Rd, [Rn, -Rm, ASR #]
		int shift = (opcode >> 7) & 31;
		int offset;
		if (shift)
		  offset = (int) ((signed int) reg[opcode & 15].I >> shift);
		else if (reg[opcode & 15].I & 0x80000000)
		  offset = 0xFFFFFFFF;
		else
		  offset = 0;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I - offset;
		CPUReadMemoryMoore (address);
	      }
	      break;
	    case 0x716:
	    case 0x71e:
	      {
		// LDR Rd, [Rn, -Rm, ROR #]
		int shift = (opcode >> 7) & 31;
		unsigned int value = reg[opcode & 15].I;
		if (shift)
		  {
		    ROR_VALUE;
		  }
		else
		  {
		    RCR_VALUE;
		  }
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I - value;
		CPUReadMemoryMoore (address);
	      }
	      break;
	    case 0x730:
	    case 0x738:
	      {
		// LDR Rd, [Rn, -Rm, LSL #]!
		int offset = reg[opcode & 15].I << ((opcode >> 7) & 31);
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I - offset;
		CPUReadMemoryMoore (address);
	      }
	      break;
	    case 0x732:
	    case 0x73a:
	      {
		// LDR Rd, [Rn, -Rm, LSR #]!
		int shift = (opcode >> 7) & 31;
		int offset = shift ? reg[opcode & 15].I >> shift : 0;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I - offset;
		CPUReadMemoryMoore (address);
	      }
	      break;
	    case 0x734:
	    case 0x73c:
	      {
		// LDR Rd, [Rn, -Rm, ASR #]!
		int shift = (opcode >> 7) & 31;
		int offset;
		if (shift)
		  offset = (int) ((signed int) reg[opcode & 15].I >> shift);
		else if (reg[opcode & 15].I & 0x80000000)
		  offset = 0xFFFFFFFF;
		else
		  offset = 0;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I - offset;
		CPUReadMemoryMoore (address);
	      }
	      break;
	    case 0x736:
	    case 0x73e:
	      {
		// LDR Rd, [Rn, -Rm, ROR #]!
		int shift = (opcode >> 7) & 31;
		unsigned int value = reg[opcode & 15].I;
		if (shift)
		  {
		    ROR_VALUE;
		  }
		else
		  {
		    RCR_VALUE;
		  }
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I - value;
		CPUReadMemoryMoore (address);
	      }
	      break;
	    case 0x790:
	    case 0x798:
	      {
		// LDR Rd, [Rn, Rm, LSL #]
		int offset = reg[opcode & 15].I << ((opcode >> 7) & 31);
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I + offset;
		CPUReadMemoryMoore (address);
	      }
	      break;
	    case 0x792:
	    case 0x79a:
	      {
		// LDR Rd, [Rn, Rm, LSR #]
		int shift = (opcode >> 7) & 31;
		int offset = shift ? reg[opcode & 15].I >> shift : 0;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I + offset;
		CPUReadMemoryMoore (address);
	      }
	      break;
	    case 0x794:
	    case 0x79c:
	      {
		// LDR Rd, [Rn, Rm, ASR #]
		int shift = (opcode >> 7) & 31;
		int offset;
		if (shift)
		  offset = (int) ((signed int) reg[opcode & 15].I >> shift);
		else if (reg[opcode & 15].I & 0x80000000)
		  offset = 0xFFFFFFFF;
		else
		  offset = 0;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I + offset;
		CPUReadMemoryMoore (address);
	      }
	      break;
	    case 0x796:
	    case 0x79e:
	      {
		// LDR Rd, [Rn, Rm, ROR #]
		int shift = (opcode >> 7) & 31;
		unsigned int value = reg[opcode & 15].I;
		if (shift)
		  {
		    ROR_VALUE;
		  }
		else
		  {
		    RCR_VALUE;
		  }
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I + value;
		CPUReadMemoryMoore (address);
	      }
	      break;
	    case 0x7b0:
	    case 0x7b8:
	      {
		// LDR Rd, [Rn, Rm, LSL #]!
		int offset = reg[opcode & 15].I << ((opcode >> 7) & 31);
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I + offset;
		CPUReadMemoryMoore (address);
	      }
	      break;
	    case 0x7b2:
	    case 0x7ba:
	      {
		// LDR Rd, [Rn, Rm, LSR #]!
		int shift = (opcode >> 7) & 31;
		int offset = shift ? reg[opcode & 15].I >> shift : 0;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I + offset;
		CPUReadMemoryMoore (address);
	      }
	      break;
	    case 0x7b4:
	    case 0x7bc:
	      {
		// LDR Rd, [Rn, Rm, ASR #]!
		int shift = (opcode >> 7) & 31;
		int offset;
		if (shift)
		  offset = (int) ((signed int) reg[opcode & 15].I >> shift);
		else if (reg[opcode & 15].I & 0x80000000)
		  offset = 0xFFFFFFFF;
		else
		  offset = 0;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I + offset;
		CPUReadMemoryMoore (address);
	      }
	      break;
	    case 0x7b6:
	    case 0x7be:
	      {
		// LDR Rd, [Rn, Rm, ROR #]!
		int shift = (opcode >> 7) & 31;
		unsigned int value = reg[opcode & 15].I;
		if (shift)
		  {
		    ROR_VALUE;
		  }
		else
		  {
		    RCR_VALUE;
		  }
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I + value;
		CPUReadMemoryMoore (address);
	      }
	      break;
	    case 0x640:
	    case 0x648:
	      // T versions are the same
	    case 0x660:
	    case 0x668:
	      {
		// STRB Rd, [Rn], -Rm, LSL #
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I;
		CPUWriteByteMoore (address, reg[dest].B.B0);
	      }
	      break;
	    case 0x642:
	    case 0x64a:
	      // T versions are the same
	    case 0x662:
	    case 0x66a:
	      {
		// STRB Rd, [Rn], -Rm, LSR #
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I;
		CPUWriteByteMoore (address, reg[dest].B.B0);
	      }
	      break;
	    case 0x644:
	    case 0x64c:
	      // T versions are the same
	    case 0x664:
	    case 0x66c:
	      {
		// STRB Rd, [Rn], -Rm, ASR #
		int shift = (opcode >> 7) & 31;
		int offset;
		if (shift)
		  offset = (int) ((signed int) reg[opcode & 15].I >> shift);
		else if (reg[opcode & 15].I & 0x80000000)
		  offset = 0xFFFFFFFF;
		else
		  offset = 0;
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I;
		CPUWriteByteMoore (address, reg[dest].B.B0);
	      }
	      break;
	    case 0x646:
	    case 0x64e:
	      // T versions are the same
	    case 0x666:
	    case 0x66e:
	      {
		// STRB Rd, [Rn], -Rm, ROR #
		int shift = (opcode >> 7) & 31;
		unsigned int value = reg[opcode & 15].I;
		if (shift)
		  {
		    ROR_VALUE;
		  }
		else
		  {
		    RCR_VALUE;
		  }
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I;
		CPUWriteByteMoore (address, reg[dest].B.B0);
	      }
	      break;
	    case 0x6c0:
	    case 0x6c8:
	      // T versions are the same
	    case 0x6e0:
	    case 0x6e8:
	      {
		// STRB Rd, [Rn], Rm, LSL #
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I;
		CPUWriteByteMoore (address, reg[dest].B.B0);
	      }
	      break;
	    case 0x6c2:
	    case 0x6ca:
	      // T versions are the same
	    case 0x6e2:
	    case 0x6ea:
	      {
		// STRB Rd, [Rn], Rm, LSR #
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I;
		CPUWriteByteMoore (address, reg[dest].B.B0);
	      }
	      break;
	    case 0x6c4:
	    case 0x6cc:
	      // T versions are the same
	    case 0x6e4:
	    case 0x6ec:
	      {
		// STR Rd, [Rn], Rm, ASR #
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I;
		CPUWriteByteMoore (address, reg[dest].B.B0);
	      }
	      break;
	    case 0x6c6:
	    case 0x6ce:
	      // T versions are the same
	    case 0x6e6:
	    case 0x6ee:
	      {
		// STRB Rd, [Rn], Rm, ROR #
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I;
		CPUWriteByteMoore (address, reg[dest].B.B0);
	      }
	      break;
	    case 0x740:
	    case 0x748:
	      {
		// STRB Rd, [Rn, -Rm, LSL #]
		int offset = reg[opcode & 15].I << ((opcode >> 7) & 31);
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I - offset;
		CPUWriteByteMoore (address, reg[dest].B.B0);
	      }
	      break;
	    case 0x742:
	    case 0x74a:
	      {
		// STRB Rd, [Rn, -Rm, LSR #]
		int shift = (opcode >> 7) & 31;
		int offset = shift ? reg[opcode & 15].I >> shift : 0;
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I - offset;
		CPUWriteByteMoore (address, reg[dest].B.B0);
	      }
	      break;
	    case 0x744:
	    case 0x74c:
	      {
		// STRB Rd, [Rn, -Rm, ASR #]
		int shift = (opcode >> 7) & 31;
		int offset;
		if (shift)
		  offset = (int) ((signed int) reg[opcode & 15].I >> shift);
		else if (reg[opcode & 15].I & 0x80000000)
		  offset = 0xFFFFFFFF;
		else
		  offset = 0;
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I - offset;
		CPUWriteByteMoore (address, reg[dest].B.B0);
	      }
	      break;
	    case 0x746:
	    case 0x74e:
	      {
		// STRB Rd, [Rn, -Rm, ROR #]
		int shift = (opcode >> 7) & 31;
		unsigned int value = reg[opcode & 15].I;
		if (shift)
		  {
		    ROR_VALUE;
		  }
		else
		  {
		    RCR_VALUE;
		  }
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I - value;
		CPUWriteByteMoore (address, reg[dest].B.B0);
	      }
	      break;
	    case 0x760:
	    case 0x768:
	      {
		// STRB Rd, [Rn, -Rm, LSL #]!
		int offset = reg[opcode & 15].I << ((opcode >> 7) & 31);
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I - offset;
		CPUWriteByteMoore (address, reg[dest].B.B0);
	      }
	      break;
	    case 0x762:
	    case 0x76a:
	      {
		// STRB Rd, [Rn, -Rm, LSR #]!
		int shift = (opcode >> 7) & 31;
		int offset = shift ? reg[opcode & 15].I >> shift : 0;
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I - offset;
		CPUWriteByteMoore (address, reg[dest].B.B0);
	      }
	      break;
	    case 0x764:
	    case 0x76c:
	      {
		// STRB Rd, [Rn, -Rm, ASR #]!
		int shift = (opcode >> 7) & 31;
		int offset;
		if (shift)
		  offset = (int) ((signed int) reg[opcode & 15].I >> shift);
		else if (reg[opcode & 15].I & 0x80000000)
		  offset = 0xFFFFFFFF;
		else
		  offset = 0;
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I - offset;
		CPUWriteByteMoore (address, reg[dest].B.B0);
	      }
	      break;
	    case 0x766:
	    case 0x76e:
	      {
		// STRB Rd, [Rn, -Rm, ROR #]!
		int shift = (opcode >> 7) & 31;
		unsigned int value = reg[opcode & 15].I;
		if (shift)
		  {
		    ROR_VALUE;
		  }
		else
		  {
		    RCR_VALUE;
		  }
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I - value;
		CPUWriteByteMoore (address, reg[dest].B.B0);
	      }
	      break;
	    case 0x7c0:
	    case 0x7c8:
	      {
		// STRB Rd, [Rn, Rm, LSL #]
		int offset = reg[opcode & 15].I << ((opcode >> 7) & 31);
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I + offset;
		CPUWriteByteMoore (address, reg[dest].B.B0);
	      }
	      break;
	    case 0x7c2:
	    case 0x7ca:
	      {
		// STRB Rd, [Rn, Rm, LSR #]
		int shift = (opcode >> 7) & 31;
		int offset = shift ? reg[opcode & 15].I >> shift : 0;
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I + offset;
		CPUWriteByteMoore (address, reg[dest].B.B0);
	      }
	      break;
	    case 0x7c4:
	    case 0x7cc:
	      {
		// STRB Rd, [Rn, Rm, ASR #]
		int shift = (opcode >> 7) & 31;
		int offset;
		if (shift)
		  offset = (int) ((signed int) reg[opcode & 15].I >> shift);
		else if (reg[opcode & 15].I & 0x80000000)
		  offset = 0xFFFFFFFF;
		else
		  offset = 0;
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I + offset;
		CPUWriteByteMoore (address, reg[dest].B.B0);
	      }
	      break;
	    case 0x7c6:
	    case 0x7ce:
	      {
		// STRB Rd, [Rn, Rm, ROR #]
		int shift = (opcode >> 7) & 31;
		unsigned int value = reg[opcode & 15].I;
		if (shift)
		  {
		    ROR_VALUE;
		  }
		else
		  {
		    RCR_VALUE;
		  }
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I + value;
		CPUWriteByteMoore (address, reg[dest].B.B0);
	      }
	      break;
	    case 0x7e0:
	    case 0x7e8:
	      {
		// STRB Rd, [Rn, Rm, LSL #]!
		int offset = reg[opcode & 15].I << ((opcode >> 7) & 31);
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I + offset;
		CPUWriteByteMoore (address, reg[dest].B.B0);
	      }
	      break;
	    case 0x7e2:
	    case 0x7ea:
	      {
		// STRB Rd, [Rn, Rm, LSR #]!
		int shift = (opcode >> 7) & 31;
		int offset = shift ? reg[opcode & 15].I >> shift : 0;
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I + offset;
		CPUWriteByteMoore (address, reg[dest].B.B0);
	      }
	      break;
	    case 0x7e4:
	    case 0x7ec:
	      {
		// STRB Rd, [Rn, Rm, ASR #]!
		int shift = (opcode >> 7) & 31;
		int offset;
		if (shift)
		  offset = (int) ((signed int) reg[opcode & 15].I >> shift);
		else if (reg[opcode & 15].I & 0x80000000)
		  offset = 0xFFFFFFFF;
		else
		  offset = 0;
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I + offset;
		CPUWriteByteMoore (address, reg[dest].B.B0);
	      }
	      break;
	    case 0x7e6:
	    case 0x7ee:
	      {
		// STRB Rd, [Rn, Rm, ROR #]!
		int shift = (opcode >> 7) & 31;
		unsigned int value = reg[opcode & 15].I;
		if (shift)
		  {
		    ROR_VALUE;
		  }
		else
		  {
		    RCR_VALUE;
		  }
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I + value;
		CPUWriteByteMoore (address, reg[dest].B.B0);
	      }
	      break;
	    case 0x650:
	    case 0x658:
	      // T versions are the same
	    case 0x670:
	    case 0x678:
	      {
		// LDRB Rd, [Rn], -Rm, LSL #
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I;
		CPUReadByteMoore (address);
	      }
	      break;
	    case 0x652:
	    case 0x65a:
	      // T versions are the same
	    case 0x672:
	    case 0x67a:
	      {
		// LDRB Rd, [Rn], -Rm, LSR #
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I;
		CPUReadByteMoore (address);
	      }
	      break;
	    case 0x654:
	    case 0x65c:
	      // T versions are the same
	    case 0x674:
	    case 0x67c:
	      {
		// LDRB Rd, [Rn], -Rm, ASR #
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I;
		CPUReadByteMoore (address);
	      }
	      break;
	    case 0x656:
	    case 0x65e:
	      // T versions are the same
	    case 0x676:
	    case 0x67e:
	      {
		// LDRB Rd, [Rn], -Rm, ROR #
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I;
		CPUReadByteMoore (address);
	      }
	      break;
	    case 0x6d0:
	    case 0x6d8:
	      // T versions are the same
	    case 0x6f0:
	    case 0x6f8:
	      {
		// LDRB Rd, [Rn], Rm, LSL #
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I;
		CPUReadByteMoore (address);
	      }
	      break;
	    case 0x6d2:
	    case 0x6da:
	      // T versions are the same
	    case 0x6f2:
	    case 0x6fa:
	      {
		// LDRB Rd, [Rn], Rm, LSR #
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I;
		CPUReadByteMoore (address);
	      }
	      break;
	    case 0x6d4:
	    case 0x6dc:
	      // T versions are the same
	    case 0x6f4:
	    case 0x6fc:
	      {
		// LDRB Rd, [Rn], Rm, ASR #
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I;
		CPUReadByteMoore (address);
	      }
	      break;
	    case 0x6d6:
	    case 0x6de:
	      // T versions are the same
	    case 0x6f6:
	    case 0x6fe:
	      {
		// LDRB Rd, [Rn], Rm, ROR #
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I;
		CPUReadByteMoore (address);
	      }
	      break;
	    case 0x750:
	    case 0x758:
	      {
		// LDRB Rd, [Rn, -Rm, LSL #]
		int offset = reg[opcode & 15].I << ((opcode >> 7) & 31);
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I - offset;
		CPUReadByteMoore (address);
	      }
	      break;
	    case 0x752:
	    case 0x75a:
	      {
		// LDRB Rd, [Rn, -Rm, LSR #]
		int shift = (opcode >> 7) & 31;
		int offset = shift ? reg[opcode & 15].I >> shift : 0;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I - offset;
		CPUReadByteMoore (address);
	      }
	      break;
	    case 0x754:
	    case 0x75c:
	      {
		// LDRB Rd, [Rn, -Rm, ASR #]
		int shift = (opcode >> 7) & 31;
		int offset;
		if (shift)
		  offset = (int) ((signed int) reg[opcode & 15].I >> shift);
		else if (reg[opcode & 15].I & 0x80000000)
		  offset = 0xFFFFFFFF;
		else
		  offset = 0;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I - offset;
		CPUReadByteMoore (address);
	      }
	      break;
	    case 0x756:
	    case 0x75e:
	      {
		// LDRB Rd, [Rn, -Rm, ROR #]
		int shift = (opcode >> 7) & 31;
		unsigned int value = reg[opcode & 15].I;
		if (shift)
		  {
		    ROR_VALUE;
		  }
		else
		  {
		    RCR_VALUE;
		  }
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I - value;
		CPUReadByteMoore (address);
	      }
	      break;
	    case 0x770:
	    case 0x778:
	      {
		// LDRB Rd, [Rn, -Rm, LSL #]!
		int offset = reg[opcode & 15].I << ((opcode >> 7) & 31);
		int dest = (opcode >> 12) & 15;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I - offset;
		CPUReadByteMoore (address);
		if (dest != base)
		  reg[base].I = address;
	      }
	      break;
	    case 0x772:
	    case 0x77a:
	      {
		// LDRB Rd, [Rn, -Rm, LSR #]!
		int shift = (opcode >> 7) & 31;
		int offset = shift ? reg[opcode & 15].I >> shift : 0;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I - offset;
		CPUReadByteMoore (address);
	      }
	      break;
	    case 0x774:
	    case 0x77c:
	      {
		// LDRB Rd, [Rn, -Rm, ASR #]!
		int shift = (opcode >> 7) & 31;
		int offset;
		if (shift)
		  offset = (int) ((signed int) reg[opcode & 15].I >> shift);
		else if (reg[opcode & 15].I & 0x80000000)
		  offset = 0xFFFFFFFF;
		else
		  offset = 0;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I - offset;
		CPUReadByteMoore (address);
	      }
	      break;
	    case 0x776:
	    case 0x77e:
	      {
		// LDRB Rd, [Rn, -Rm, ROR #]!
		int shift = (opcode >> 7) & 31;
		unsigned int value = reg[opcode & 15].I;
		if (shift)
		  {
		    ROR_VALUE;
		  }
		else
		  {
		    RCR_VALUE;
		  }
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I - value;
		CPUReadByteMoore (address);
	      }
	      break;
	    case 0x7d0:
	    case 0x7d8:
	      {
		// LDRB Rd, [Rn, Rm, LSL #]
		int offset = reg[opcode & 15].I << ((opcode >> 7) & 31);
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I + offset;
		CPUReadByteMoore (address);
	      }
	      break;
	    case 0x7d2:
	    case 0x7da:
	      {
		// LDRB Rd, [Rn, Rm, LSR #]
		int shift = (opcode >> 7) & 31;
		int offset = shift ? reg[opcode & 15].I >> shift : 0;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I + offset;
		CPUReadByteMoore (address);
	      }
	      break;
	    case 0x7d4:
	    case 0x7dc:
	      {
		// LDRB Rd, [Rn, Rm, ASR #]
		int shift = (opcode >> 7) & 31;
		int offset;
		if (shift)
		  offset = (int) ((signed int) reg[opcode & 15].I >> shift);
		else if (reg[opcode & 15].I & 0x80000000)
		  offset = 0xFFFFFFFF;
		else
		  offset = 0;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I + offset;
		CPUReadByteMoore (address);
	      }
	      break;
	    case 0x7d6:
	    case 0x7de:
	      {
		// LDRB Rd, [Rn, Rm, ROR #]
		int shift = (opcode >> 7) & 31;
		unsigned int value = reg[opcode & 15].I;
		if (shift)
		  {
		    ROR_VALUE;
		  }
		else
		  {
		    RCR_VALUE;
		  }
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I + value;
		CPUReadByteMoore (address);
	      }
	      break;
	    case 0x7f0:
	    case 0x7f8:
	      {
		// LDRB Rd, [Rn, Rm, LSL #]!
		int offset = reg[opcode & 15].I << ((opcode >> 7) & 31);
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I + offset;
		CPUReadByteMoore (address);
	      }
	      break;
	    case 0x7f2:
	    case 0x7fa:
	      {
		// LDRB Rd, [Rn, Rm, LSR #]!
		int shift = (opcode >> 7) & 31;
		int offset = shift ? reg[opcode & 15].I >> shift : 0;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I + offset;
		CPUReadByteMoore (address);
	      }
	      break;
	    case 0x7f4:
	    case 0x7fc:
	      {
		// LDRB Rd, [Rn, Rm, ASR #]!
		int shift = (opcode >> 7) & 31;
		int offset;
		if (shift)
		  offset = (int) ((signed int) reg[opcode & 15].I >> shift);
		else if (reg[opcode & 15].I & 0x80000000)
		  offset = 0xFFFFFFFF;
		else
		  offset = 0;
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I + offset;
		CPUReadByteMoore (address);
	      }
	      break;
	    case 0x7f6:
	    case 0x7fe:
	      {
		// LDRB Rd, [Rn, Rm, ROR #]!
		int shift = (opcode >> 7) & 31;
		unsigned int value = reg[opcode & 15].I;
		if (shift)
		  {
		    ROR_VALUE;
		  }
		else
		  {
		    RCR_VALUE;
		  }
		int base = (opcode >> 16) & 15;
		unsigned int address = reg[base].I + value;
		CPUReadByteMoore (address);
	      }
	      break;

	      CASE_16 (0x800)   // STMDA Rn, {Rlist}
		CASE_16 (0x820) // STMDA Rn!, {Rlist}
		{
		switch(mFsmState) {
		case COMPUTEADDRESS:
		  break;
		case REGISTER:
		  if (num_register != -1)
		    CPUWriteMemoryMoore(dm_address-4, reg[num_register].I);
		  break;
		case END:
		  break;		
		}
	      }
	      break;

// 	      CASE_16 (0x820) {
// 		// STMDA Rn!, {Rlist}
// 		switch(mFsmState) {
// 		case COMPUTEADDRESS:
// 		  break;
// 		case REGISTER:
// 		  if (num_register != -1)
// 		    CPUWriteMemoryMoore(dm_address-4, reg[num_register].I);
// 		  break;
// 		case END:
// 		  break;		
// 		}
// 	      }
// 	      break;

	      CASE_16 (0x840)   // STMDA Rn, {Rlist}^
		CASE_16 (0x860) // STMDA Rn!, {Rlist}^
		{
		int num;
		switch(mFsmState) {
		case COMPUTEADDRESS:
		  break;
		case REGISTER:
		  if (num_register != -1) {
		    if ((num_register >= 8) && (num_register <= 12)) {
		      if (armMode == 0x11) 
			num = num_register+OFFSET_FIQ;
		      else
			num = num_register;
		    }
		    else if ((num_register == 13) || (num_register==14)) {
		      if ((armMode != 0x10) && (armMode != 0x1f)) {
			num = num_register+OFFSET_USR;
		      }
		      else
			num = num_register;
		    }
		    else
		      num = num_register;
		    CPUWriteMemoryMoore(dm_address-4,reg[num].I);
		  }
		  break;
		case END:
		  break;
		}
	      }
	      break;

// 	      CASE_16 (0x860) {
// 		// STMDA Rn!, {Rlist}^
// 		switch(mFsmState) {
// 		case COMPUTEADDRESS:
// 		  break;
// 		case REGISTER:
// 		  if ((num_register >= 8) && (num_register <= 12)) {
// 		    if (armMode == 0x11) 
// 		      CPUWriteMemoryMoore(dm_address-4,reg[num_register+OFFSET_FIQ].I);
// 		    else
// 		      CPUWriteMemoryMoore(dm_address-4,reg[num_register].I);
// 		  }
// 		  else if ((num_register == 13) || (num_register==14)) {
// 		    if ((armMode != 0x10) && (armMode != 0x1f)) {
// 		      CPUWriteMemoryMoore(dm_address-4,reg[num_register+OFFSET_USR].I);
// 		    }
// 		    else
// 		      CPUWriteMemoryMoore(dm_address-4,reg[num_register].I);
// 		  }
// 		  else
// 		    CPUWriteMemoryMoore(dm_address-4,reg[num_register].I);
// 		  break;
// 		case END:
// 		  break;
// 		}
// 	      }
// 	      break;
	  
	      CASE_16 (0x880)   // STMIA Rn, {Rlist}
		CASE_16 (0x8a0) // STMIA Rn!, {Rlist}
		{
		switch(mFsmState) {
		case COMPUTEADDRESS:
		  break;
		case REGISTER:
		  if (num_register != -1)
		    CPUWriteMemoryMoore(dm_address-4, reg[num_register].I);
		  break;
		case END:
		  break;		
		}
	      }
	      break;

// 	      CASE_16 (0x8a0) {
// 		// STMIA Rn!, {Rlist}
// 		switch(mFsmState) {
// 		case COMPUTEADDRESS:
// 		  break;
// 		case REGISTER:
// 		  CPUWriteMemoryMoore(dm_address-4, reg[num_register].I);
// 		  break;
// 		case END:
// 		  break;		
// 		}
// 	      }
// 	      break;

	      CASE_16 (0x8c0)   // STMIA Rn, {Rlist}^
		CASE_16 (0x8e0) // STMIA Rn!, {Rlist}^
		{
		  int num;
		switch(mFsmState) {
		case COMPUTEADDRESS:
		  break;
		case REGISTER:
		  if (num_register != -1) {
		    if ((num_register >= 8) && (num_register <= 12)) {
		      if (armMode == 0x11) 
			num = num_register + OFFSET_FIQ;
		      else
			num = num_register;
		    }
		    else if ((num_register == 13) || (num_register==14)) {
		      if ((armMode != 0x10) && (armMode != 0x1f)) {
			num = num_register+OFFSET_USR;
		      }
		      else
			num = num_register;
		    }
		    else
		      num = num_register;
		    CPUWriteMemoryMoore(dm_address-4,reg[num].I);
		  }
		  break;
		case END:
		  break;
		}
	      }
	      break;
	  
// 	      CASE_16 (0x8e0) {
// 		// STMIA Rn!, {Rlist}^
// 		switch(mFsmState) {
// 		case COMPUTEADDRESS:
// 		  break;
// 		case REGISTER:
// 		  if ((num_register >= 8) && (num_register <= 12)) {
// 		    if (armMode == 0x11) 
// 		      CPUWriteMemoryMoore(dm_address-4,reg[num_register+OFFSET_FIQ].I);
// 		    else
// 		      CPUWriteMemoryMoore(dm_address-4,reg[num_register].I);
// 		  }
// 		  else if ((num_register == 13) || (num_register==14)) {
// 		    if ((armMode != 0x10) && (armMode != 0x1f)) {
// 		      CPUWriteMemoryMoore(dm_address-4,reg[num_register+OFFSET_USR].I);
// 		    }
// 		    else
// 		      CPUWriteMemoryMoore(dm_address-4,reg[num_register].I);
// 		  }
// 		  else
// 		    CPUWriteMemoryMoore(dm_address-4,reg[num_register].I);
// 		  break;
// 		case END:
// 		  CPUWriteMemoryMoore(dm_address-4,reg[num_register].I);
// 		  break;
// 		}
// 	      }
// 	      break;

	      CASE_16 (0x900)   // STMDB Rn, {Rlist}
		CASE_16 (0x920) // STMDB Rn!, {Rlist}
		{
		  switch(mFsmState) {
		  case COMPUTEADDRESS:
		    break;
		  case REGISTER:
		    if (num_register != -1)
		      CPUWriteMemoryMoore(dm_address-4, reg[num_register].I);
		    break;
		  case END:
		    break;		
		  }
		}
	      break;

// 	      CASE_16 (0x920) {
// 		// STMDB Rn!, {Rlist}
// 		switch(mFsmState) {
// 		case COMPUTEADDRESS:
// 		  break;
// 		case REGISTER:
// 		  if (num_register != -1)
// 		    CPUWriteMemoryMoore(dm_address-4, reg[num_register].I);
// 		  break;
// 		case END:
// 		  break;		
// 		}
// 	      }
// 	      break;

	      CASE_16 (0x940)   // STMDB Rn, {Rlist}^
		CASE_16 (0x960) // STMDB Rn!, {Rlist}^
		{
		  int num;
		  switch(mFsmState) {
		  case COMPUTEADDRESS:
		  break;
		  case REGISTER:
		    if (num_register != -1) {
		    if ((num_register >= 8) && (num_register <= 12)) {
		      if (armMode == 0x11) 
			num = num_register+OFFSET_FIQ;
		      else
			num = num_register;
		  }
		  else if ((num_register == 13) || (num_register==14)) {
		    if ((armMode != 0x10) && (armMode != 0x1f)) {
		      num = num_register+OFFSET_USR;
		    }
		    else
		      num = num_register;
		  }
		    else
		      num = num_register;
		    CPUWriteMemoryMoore(dm_address-4,reg[num].I);
		    }
		  break;
		case END:
		  break;
		}
	      }
	      break;

// 	      CASE_16 (0x960) {
// 		// STMDB Rn!, {Rlist}^
// 		switch(mFsmState) {
// 		case COMPUTEADDRESS:
// 		  break;
// 		case REGISTER:
// 		  if ((num_register >= 8) && (num_register <= 12)) {
// 		    if (armMode == 0x11) 
// 		      CPUWriteMemoryMoore(dm_address-4,reg[num_register+OFFSET_FIQ].I);
// 		    else
// 		      CPUWriteMemoryMoore(dm_address-4,reg[num_register].I);
// 		  }
// 		  else if ((num_register == 13) || (num_register==14)) {
// 		    if ((armMode != 0x10) && (armMode != 0x1f)) {
// 		      CPUWriteMemoryMoore(dm_address-4,reg[num_register+OFFSET_USR].I);
// 		    }
// 		    else
// 		      CPUWriteMemoryMoore(dm_address-4,reg[num_register].I);
// 		  }
// 		  else
// 		    CPUWriteMemoryMoore(dm_address-4,reg[num_register].I);
// 		  break;
// 		case END:
// 		  break;
// 		}
// 	      }
// 	      break;

	      CASE_16 (0x980)   // STMIB Rn, {Rlist}
		CASE_16 (0x9a0) // STMIB Rn!, {Rlist}
		{
		switch(mFsmState) {
		case COMPUTEADDRESS:
		  break;
		case REGISTER:
		  if (num_register != -1)
		    CPUWriteMemoryMoore(dm_address-4, reg[num_register].I);
		  break;
		case END:
		  break;		
		}
	      }
	      break;
	  
// 	      CASE_16 (0x9a0) {
// 		// STMIB Rn!, {Rlist}
// 		switch(mFsmState) {
// 		case COMPUTEADDRESS:
// 		  break;
// 		case REGISTER:
// 		  CPUWriteMemoryMoore(dm_address-4, reg[num_register].I);
// 		  break;
// 		case END:
// 		  break;		
// 		}
// 	      }
// 	      break;

	      CASE_16 (0x9c0)   // STMIB Rn, {Rlist}^
		CASE_16 (0x9e0) // STMIB Rn!, {Rlist}^
		{
		  int num;
		switch(mFsmState) {
		case COMPUTEADDRESS:
		  break;
		case REGISTER:
		  if (num_register != -1) {
		  if ((num_register >= 8) && (num_register <= 12)) {
		    if (armMode == 0x11) 
		    num = num_register+OFFSET_FIQ;
		    else
		      num = num_register;
		  }
		  else if ((num_register == 13) || (num_register==14)) {
		    if ((armMode != 0x10) && (armMode != 0x1f)) {
		      num = num_register+OFFSET_USR;
		    }
		    else
		      num = num_register;
		  }
		  else
		    num = num_register;
		  CPUWriteMemoryMoore(dm_address-4,reg[num].I);
		  }
		  break;
		case END:
		  break;
		}
	      }
	      break;

// 	      CASE_16 (0x9e0) {
// 		// STMIB Rn!, {Rlist}^
// 		switch(mFsmState) {
// 		case COMPUTEADDRESS:
// 		  break;
// 		case REGISTER:
// 		  if ((num_register >= 8) && (num_register <= 12)) {
// 		    if (armMode == 0x11) 
// 		      CPUWriteMemoryMoore(dm_address-4,reg[num_register+OFFSET_FIQ].I);
// 		    else
// 		      CPUWriteMemoryMoore(dm_address-4,reg[num_register].I);
// 		  }
// 		  else if ((num_register == 13) || (num_register==14)) {
// 		    if ((armMode != 0x10) && (armMode != 0x1f)) {
// 		      CPUWriteMemoryMoore(dm_address-4,reg[num_register+OFFSET_USR].I);
// 		    }
// 		    else
// 		      CPUWriteMemoryMoore(dm_address-4,reg[num_register].I);
// 		  }
// 		  else
// 		    CPUWriteMemoryMoore(dm_address-4,reg[num_register].I);
// 		  break;
// 		case END:
// 		  break;
// 		}
// 	      }
// 	      break;
	  
	      CASE_16 (0x810)   //LDMDA Rn, {Rlist}
		CASE_16 (0x830) //LDMDA Rn!, {Rlist}
		CASE_16 (0x850) //LDMDA Rn, {Rlist}^
		CASE_16 (0x870) //LDMDA Rn!, {Rlist}^
		CASE_16 (0x890) //LDMIA Rn, {Rlist}
		CASE_16 (0x8B0) //LDMIA Rn!, {Rlist}
		CASE_16 (0x8D0) //LDMIA Rn, {Rlist}^
		CASE_16 (0x8F0) //LDMIA Rn!, {Rlist}^
		CASE_16 (0x910) //LDMDB Rn, {Rlist}
		CASE_16 (0x930) //LDMDB Rn!, {Rlist}
		CASE_16 (0x950) //LDMDB Rn, {Rlist}^
		CASE_16 (0x970) //LDMDB Rn!, {Rlist}^
		CASE_16 (0x990) //LDMIB Rn, {Rlist}
		CASE_16 (0x9B0) //LDMIB Rn!, {Rlist}
		CASE_16 (0x9D0) //LDMIB Rn, {Rlist}^
		CASE_16 (0x9F0) //LDMIB Rn!, {Rlist}^
		{
		  switch(mFsmState) {
		  case COMPUTEADDRESS:
		    CPUReadMemoryMoore(dm_address);
		  case REGISTER:
		    CPUReadMemoryMoore(dm_address);
		    break;
		  case END:
		    break;
		  }
		}
	      break;

	    default:
	      DCACHE.REQ = false;
	      DCACHE.UNC = false;
	      DCACHE.ADR   = 0;
	      DCACHE.TYPE  = 0;
	      DCACHE.WDATA = 0;
	      break;
	      // END
	    }
	}
    }
#ifdef DEBUG
    if (ICACHE.FRZ==false && DCACHE.FRZ==false)
      fprintf(logfile,"Moore R[15]: %x\n",GPR[15].read());
#endif
    ICACHE.REQ = true;		// instruction request always valid
    ICACHE.ADR = (sc_uint < 32 >) GPR[15];
  };				// end genMoore()

  void SOCLIB_ARM7::CPUSwitchMode (int mode, bool saveState, bool breakLoop) {

    CPUUpdateCPSR();

    switch(armMode) {
    case 0x10: // User mode
    case 0x1F: // System mode
      reg[R13_USR].I = reg[13].I;
      reg[R14_USR].I = reg[14].I;
      reg[17].I = reg[16].I;
      break;
    case 0x11: // FIQ mode
      CPUSwap(&reg[R8_FIQ].I, &reg[8].I);
      CPUSwap(&reg[R9_FIQ].I, &reg[9].I);
      CPUSwap(&reg[R10_FIQ].I, &reg[10].I);
      CPUSwap(&reg[R11_FIQ].I, &reg[11].I);
      CPUSwap(&reg[R12_FIQ].I, &reg[12].I);
      reg[R13_FIQ].I = reg[13].I;
      reg[R14_FIQ].I = reg[14].I;
      reg[SPSR_FIQ].I = reg[17].I;
      break;
    case 0x12: // IRQ mode
      reg[R13_IRQ].I  = reg[13].I;
      reg[R14_IRQ].I  = reg[14].I;
      reg[SPSR_IRQ].I =  reg[17].I;
      break;
    case 0x13: //Supervisor mode
      reg[R13_SVC].I  = reg[13].I;
      reg[R14_SVC].I  = reg[14].I;
      reg[SPSR_SVC].I =  reg[17].I;
      break;
    case 0x17: // Abort mode
      reg[R13_ABT].I  = reg[13].I;
      reg[R14_ABT].I  = reg[14].I;
      reg[SPSR_ABT].I =  reg[17].I;
      break;
    case 0x1b: // Undefined mode
      reg[R13_UND].I  = reg[13].I;
      reg[R14_UND].I  = reg[14].I;
      reg[SPSR_UND].I =  reg[17].I;
      break;
    }

    unsigned int CPSR = reg[16].I;
    unsigned int SPSR = reg[17].I;
  
    switch(mode) {
    case 0x10:
    case 0x1F:
      reg[13].I = reg[R13_USR].I;
      reg[14].I = reg[R14_USR].I;
      reg[16].I = SPSR;
      break;
    case 0x11:
      CPUSwap(&reg[8].I, &reg[R8_FIQ].I);
      CPUSwap(&reg[9].I, &reg[R9_FIQ].I);
      CPUSwap(&reg[10].I, &reg[R10_FIQ].I);
      CPUSwap(&reg[11].I, &reg[R11_FIQ].I);
      CPUSwap(&reg[12].I, &reg[R12_FIQ].I);
      reg[13].I = reg[R13_FIQ].I;
      reg[14].I = reg[R14_FIQ].I;
      if(saveState)
	reg[17].I = CPSR;
      else
	reg[17].I = reg[SPSR_FIQ].I;
      break;
    case 0x12:
      reg[13].I = reg[R13_IRQ].I;
      reg[14].I = reg[R14_IRQ].I;
      reg[16].I = SPSR;
      if(saveState)
	reg[17].I = CPSR;
      else
	reg[17].I = reg[SPSR_IRQ].I;
      break;
    case 0x13:
      reg[13].I = reg[R13_SVC].I;
      reg[14].I = reg[R14_SVC].I;
      reg[16].I = SPSR;
      if(saveState)
	reg[17].I = CPSR;
      else
	reg[17].I = reg[SPSR_SVC].I;
      break;
    case 0x17:
      reg[13].I = reg[R13_ABT].I;
      reg[14].I = reg[R14_ABT].I;
      reg[16].I = SPSR;
      if(saveState)
	reg[17].I = CPSR;
      else
	reg[17].I = reg[SPSR_ABT].I;
      break;    
    case 0x1b:
      reg[13].I = reg[R13_UND].I;
      reg[14].I = reg[R14_UND].I;
      reg[16].I = SPSR;
      if(saveState)
	reg[17].I = CPSR;
      else
	reg[17].I = reg[SPSR_UND].I;
      break;    
    default:
      break;
    }
    armMode = mode;
    CPUUpdateFlags(breakLoop);
    CPUUpdateCPSR();
  }

  inline void SOCLIB_ARM7::CPUSwap(unsigned int *a, unsigned int *b)
  {
    unsigned int *c = b;
    //     *b = *a;
    //     *a = c;
    b = a;
    a = c;
  }

  inline void SOCLIB_ARM7::CPUUpdateFlags(bool breakLoop)
  {
    unsigned int CPSR = reg[16].I;

    N_FLAG = (CPSR & 0x80000000) ? true: false;
    Z_FLAG = (CPSR & 0x40000000) ? true: false;
    C_FLAG = (CPSR & 0x20000000) ? true: false;
    V_FLAG = (CPSR & 0x10000000) ? true: false;
    armState = (CPSR & 0x20) ? false : true;
    armIrqEnable = (CPSR & 0x80) ? false : true;
    armFiqEnable = (CPSR & 0x40) ? false : true;
    //  if(breakLoop) {
    //    if(armIrqEnable && (IF & IE) && (IME & 1)) {
    //      CPU_BREAK_LOOP_2;
    //    }
    //  }
  }

  inline void SOCLIB_ARM7::CPUSwitchMode (int mode, bool saveState)
  {
    CPUSwitchMode (mode, saveState, true);
  }

  inline void SOCLIB_ARM7::CPUUpdateCPSR ()
  {
    unsigned int CPSR = reg[16].I & 0x40;
    if(N_FLAG)
      CPSR |= 0x80000000;
    if(Z_FLAG)
      CPSR |= 0x40000000;
    if(C_FLAG)
      CPSR |= 0x20000000;
    if(V_FLAG)
      CPSR |= 0x10000000;
    if(!armState)
      CPSR |= 0x00000020;
    if(!armFiqEnable)
      CPSR |= 0x00000040;
    if(!armIrqEnable)
      CPSR |= 0x80;
    CPSR |= (armMode & 0x1F);
    reg[16].I = CPSR;

  }

  inline void SOCLIB_ARM7::CPUUpdateFlags ()
  {
    CPUUpdateFlags(true);
  }

  inline unsigned int SOCLIB_ARM7::CPUReadMemoryTransition (unsigned int address)
  {
    return (unsigned int)DCACHE.RDATA.read();
  }

  inline void SOCLIB_ARM7::CPUReadMemoryMoore (unsigned int address)
  {
    int index = (address >> (32 - MSB_NUMBER)) & MSB_MASK;
    DCACHE.UNC = UNCACHED_TABLE[index];
    DCACHE.REQ = true;
    DCACHE.ADR = (sc_uint<32>)address;
    DCACHE.TYPE = (sc_uint<3>)DTYPE_RW;
    DCACHE.WDATA = (sc_uint<32>)0x0;
  }

  inline void SOCLIB_ARM7::CPUWriteMemoryMoore (unsigned int address, unsigned int value)
  {
    int index = (address >> (32 - MSB_NUMBER)) & MSB_MASK;
    DCACHE.UNC = UNCACHED_TABLE[index];
    DCACHE.REQ = true;
    DCACHE.ADR = (sc_uint<32>)address;
    DCACHE.TYPE = (sc_uint<3>)DTYPE_WW;
    DCACHE.WDATA = (sc_uint<32>)value;
  }

  inline unsigned short SOCLIB_ARM7::CPUReadHalfWordTransition (unsigned int address)
  {
    int temp = DCACHE.RDATA.read();
    return (unsigned short) ((temp >> (8 * (address & 0x2))) & 0x0000FFFF);
  }

  inline void SOCLIB_ARM7::CPUReadHalfWordMoore (unsigned int address)
  {
    int index = (address >> (32 - MSB_NUMBER)) & MSB_MASK;
    DCACHE.UNC = UNCACHED_TABLE[index];
    DCACHE.REQ = true;
    DCACHE.ADR = (sc_uint<32>)(address & 0xFFFFFFFD);
    DCACHE.TYPE = (sc_uint<3>)DTYPE_RW;
    DCACHE.WDATA = (sc_uint<32>)0x0;
  }

  inline signed short SOCLIB_ARM7::CPUReadHalfWordSignedTransition (unsigned int address)
  {
    int temp = DCACHE.RDATA.read();
    return (signed short) ((temp >> (8 * (address & 0x2))) & 0x0000FFFF);
  }

  inline void SOCLIB_ARM7::CPUReadHalfWordSignedMoore (unsigned int address)
  {
    int index = (address >> (32 - MSB_NUMBER)) & MSB_MASK;
    DCACHE.UNC = UNCACHED_TABLE[index];
    DCACHE.REQ = true;
    DCACHE.ADR = (sc_uint<32>)(address & 0xFFFFFFFD);
    DCACHE.TYPE = (sc_uint<3>)DTYPE_RW;
    DCACHE.WDATA = (sc_uint<32>)0x0;
  }

  inline void SOCLIB_ARM7::CPUWriteHalfWordMoore (unsigned int address, unsigned short value)
  {
    int index = (address >> (32 - MSB_NUMBER)) & MSB_MASK;
    DCACHE.UNC = UNCACHED_TABLE[index];
    DCACHE.REQ = true;
    DCACHE.ADR = (sc_uint<32>)address;
    DCACHE.TYPE = (sc_uint<3>)DTYPE_WH;
    DCACHE.WDATA = (sc_uint<32>)(((value & 0x0000FFFF)) |
				 ((value & 0x0000FFFF) << 16));
  }

  inline unsigned char SOCLIB_ARM7::CPUReadByteTransition (unsigned int address)
  {
    int temp = DCACHE.RDATA.read();
    return (unsigned char) ((temp >> (8* (address & 0x3))) & 0x000000FF);
  }

  inline void SOCLIB_ARM7::CPUReadByteMoore (unsigned int address)
  {
    int index = (address >> (32 - MSB_NUMBER)) & MSB_MASK;
    DCACHE.UNC = UNCACHED_TABLE[index];
    DCACHE.REQ = true;
    DCACHE.ADR = (sc_uint<32>)(address & 0xFFFFFFFC);
    DCACHE.TYPE = (sc_uint<3>)DTYPE_RW;
    DCACHE.WDATA = (sc_uint<32>)0x0;
  }

  inline void SOCLIB_ARM7::CPUWriteByteMoore (unsigned int address, unsigned char value)
  {
#ifdef DEBUG
    fprintf(logfile,"I sent something...@:%x, d:%x\n", address, value);
#endif
    int index = (address >> (32 - MSB_NUMBER)) & MSB_MASK;
    DCACHE.UNC = UNCACHED_TABLE[index];
    DCACHE.REQ = true;
    DCACHE.ADR = (sc_uint<32>)address;
    DCACHE.TYPE = (sc_uint<3>)DTYPE_WB;
    DCACHE.WDATA = (sc_uint<32>)(((value & 0x000000FF)) |
				 ((value & 0x000000FF) << 8) |
				 ((value & 0x000000FF) << 16) |
				 ((value & 0x000000FF) << 24));
  }

  void SOCLIB_ARM7::CPUSoftwareInterrupt (unsigned int value) {
    int_vector |= (1<<SWI_INT);
  }

  void SOCLIB_ARM7::CPUUndefinedException () {
    int_vector |= (1<<UNDEF_INSTR_INT);
  }

  inline int SOCLIB_ARM7::findNextReg(int opcode) {
    int i;
    for (i=num_register+1; i<16; i++) {
      if (opcode & (1<<i)) {
	num_register = i;
	return i;
      }
    }
    return -1;
  }

  inline int SOCLIB_ARM7::findNextRegThumb(int opcode) {
    int i;
    for (i=num_register+1; i<8; i++) {
      if (opcode & (1<<i)) {
	num_register = i;
	return i;
      }
    }
    
    if ((i == 8) && (opcode & (1<<8))) {
      num_register = 14;
      return 14; // check if we send the good register in every mode
    }
    return -1;
  }
}; // end struct SOCLIB_ARM7

#endif
