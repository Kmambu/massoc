
//////////////////////////////////////////////////////////////////////////////////
// File: soclib_multi_mips.h
// Authors: Alain Greiner, Hervé Charlery, Boris Boutillier 
// Date : 13/02/2004
// This program is released under the GNU Public License.
// Copyright : UPMC-LIP6
// 
// This model is derived from the PIMIPS.C model written by Frederic Petrot 
// and Denis Hommais. All the instruction decoding has been reused.
// The data and instruction caches are external.
// The processor model is a two stage pipe-line :
// - the first stage is the instruction fetch, with access to the external 
// instruction cache, and writes into the IR register.
// - the second stage is the instruction execution, with possible access to
// the data cache, and writes in the GR[i] registers. This stage implements
// in one cycle the DC,EX,MM and WB stages of the physical MIPS implementation.
// 
// The delayed branch is accurately modelized, but not the delayed load.
// 
// The main functionnal specifications are :
// 
// - The floating point instructions are not supported.
// 
// - There is no TLB, and no hardware support for virtual memory.
//
// - All MIPS R3000 exceptions are handled, including the memory errors
//  X_IBE and X_DBE, but the WRITE errors are not precise (due to the
//  posted write buffer)
//
// - Five "system" registers are supported : 
// 	 $0  (IDENT)   => processor id register
// 	 $8  (BADVADDR)=> bad address register
// 	 $12 (STATUS)  => status register
// 	 $13 (CAUSE)   => exception or interruption cause register
// 	 $14 (EPC)     => exception PC register
//   
// - This model support a data cache line invalidation feature :
// When the LW instruction is executed with the R0 destination register,
// the corresponding cache line is invalidated.
// 
// - This model provides an optional support for hardware multi-threading:
// All internal registers are replicated N times. Each set of register defines
// an hardware context. The MC_CTXT register defines the running hardware
// context. An internal "context controler" uses the i_frz and d_frz
// ports to switch the running hardware context in case of MISS.
// The maximal number of hardware context is 32.
//
// This component has two "constructor" parameters :
// - char*			insname  : instance name
// - int			ident	 : processor id
//
// This component has three "template" parameters :
// - bool MULTI_CONTEXT  : hardware support for multi-threading if true
// - char N_CTXT : number of hardware contexts
// - int  TIME_OUT       : maximal number of cycles for a given context
//
//////////////////////////////////////////////////////////////////////////////////

#ifndef SOCLIB_MULTI_MIPS_H
#define SOCLIB_MULTI_MIPS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <systemc.h>
//#include "shared/soclib_mapping_table.h"
#include "cache/soclib_xcache_interface.h"

//	Exception codes for the CAUSE register 

#define X_MOD  0x1
#define X_TLBL 0x2
#define X_TLBS 0x3
#define X_ADEL 0x4
#define X_ADES 0x5
#define X_IBE  0x6
#define X_DBE  0x7
#define X_SYS  0x8
#define X_BP   0x9
#define X_RI   0xA
#define X_CPU  0xB
#define X_OVF  0xC

//	EXCEPTION & RESET addresses

#define R3000_GENERAL_EXCEPTION_VECTOR  0x80000080 
#define R3000_RESET_VECTOR              0xBFC00000

//	OPCOD definition
//	consecutives starting from zero in the hope that the compiler will
//	generate something really trivial for the case access

#define OP_ADD     0x00
#define OP_ADDI    0x01
#define OP_ADDIU   0x02
#define OP_ADDU    0x03
#define OP_AND     0x04
#define OP_ANDI    0x05
#define OP_BC0F    0x06
#define OP_BC1F    0x07
#define OP_BC2F    0x08
#define OP_BC3F    0x09
#define OP_BC0T    0x0A
#define OP_BC1T    0x0B
#define OP_BC2T    0x0C
#define OP_BC3T    0x0D
#define OP_BEQ     0x0E
#define OP_BGEZ    0x0F
#define OP_BGEZAL  0x10
#define OP_BGTZ    0x11
#define OP_BLEZ    0x12
#define OP_BLTZ    0x13
#define OP_BLTZAL  0x14
#define OP_BNE     0x15
#define OP_BREAK   0x16
#define OP_CFC0    0x17
#define OP_CFC1    0x18
#define OP_CFC2    0x19
#define OP_CFC3    0x1A
#define OP_COP0    0x1B
#define OP_COP1    0x1C
#define OP_COP2    0x1D
#define OP_COP3    0x1E
#define OP_CTC0    0x1F
#define OP_CTC1    0x20
#define OP_CTC2    0x21
#define OP_CTC3    0x22
#define OP_DIV     0x23
#define OP_DIVU    0x24
#define OP_J       0x25
#define OP_JAL     0x26
#define OP_JALR    0x27
#define OP_JR      0x28
#define OP_LB      0x29
#define OP_LBU     0x2A
#define OP_LH      0x2B
#define OP_LHU     0x2C
#define OP_LUI     0x2D
#define OP_LW      0x2E
#define OP_LWC0    0x2F
#define OP_LWC1    0x30
#define OP_LWC2    0x31
#define OP_LWC3    0x32
#define OP_LWL     0x33
#define OP_LWR     0x34
#define OP_MFC0    0x35
#define OP_MFC1    0x36
#define OP_MFC2    0x37
#define OP_MFC3    0x38
#define OP_MFHI    0x39
#define OP_MFLO    0x3A
#define OP_MTC0    0x3B
#define OP_MTC1    0x3C
#define OP_MTC2    0x3D
#define OP_MTC3    0x3E
#define OP_MTHI    0x3F
#define OP_MTLO    0x40
#define OP_MULT    0x41
#define OP_MULTU   0x42
#define OP_NOR     0x43
#define OP_OR      0x44
#define OP_ORI     0x45
#define OP_RFE     0x46
#define OP_SB      0x47
#define OP_SH      0x48
#define OP_SLL     0x49
#define OP_SLLV    0x4A
#define OP_SLT     0x4B
#define OP_SLTI    0x4C
#define OP_SLTIU   0x4D
#define OP_SLTU    0x4E
#define OP_SRA     0x4F
#define OP_SRAV    0x50
#define OP_SRL     0x51
#define OP_SRLV    0x52
#define OP_SUB     0x53
#define OP_SUBU    0x54
#define OP_SW      0x55
#define OP_SWC0    0x56
#define OP_SWC1    0x57
#define OP_SWC2    0x58
#define OP_SWC3    0x59
#define OP_SWL     0x5A
#define OP_SWR     0x5B
#define OP_SYSCALL 0x5C
#define OP_XOR     0x5D
#define OP_XORI    0x5E
#define OP_TLBP    0x5F
#define OP_TLBR    0x60
#define OP_TLBWI   0x61
#define OP_TLBWR   0x62
#define OP_RES     0x63
#define OP_ILG     0x64
#define OP_LL      0x65
#define OP_SC      0x66

//	special OPCODs :

#define SPECIAL    0x80
#define BCOND      0x81

//	NOP definition : (addu $0 $0 $0)  */

#define IR_NOP  0x00000021

// masks for the hardware contexts

int	CTXT_MASK[] =   {0x00000001, 0x00000002, 0x00000004, 0x00000008,
                    	 0x00000010, 0x00000020, 0x00000040, 0x00000080,
                    	 0x00000100, 0x00000200, 0x00000400, 0x00000800,
                    	 0x00001000, 0x00002000, 0x00004000, 0x00008000,
                    	 0x00010000, 0x00020000, 0x00040000, 0x00080000,
                    	 0x00100000, 0x00200000, 0x00400000, 0x00800000,
                    	 0x01000000, 0x02000000, 0x04000000, 0x08000000,
                    	 0x10000000, 0x20000000, 0x40000000, 0x80000000};
			 
///////////////////////////////////////////////////////
//	Instruction decode & service functions
///////////////////////////////////////////////////////
 
enum {ITYPE, JTYPE, RTYPE, NTYPE, STYPE, BTYPE, CTYPE};

typedef struct {
    int opcode;         /* Internal op code. */
    int format;         /* Format type */
} IFormat;

// Tables: Kane's description, page A-87

IFormat OpcodeTable[] = {
  {SPECIAL,    STYPE}, {BCOND,    BTYPE}, {OP_J,    JTYPE}, {OP_JAL,   JTYPE},
  {OP_BEQ,     ITYPE}, {OP_BNE,   ITYPE}, {OP_BLEZ, ITYPE}, {OP_BGTZ,  ITYPE},
  {OP_ADDI,    ITYPE}, {OP_ADDIU, ITYPE}, {OP_SLTI, ITYPE}, {OP_SLTIU, ITYPE},
  {OP_ANDI,    ITYPE}, {OP_ORI,   ITYPE}, {OP_XORI, ITYPE}, {OP_LUI,   ITYPE},
  {OP_COP0,    CTYPE}, {OP_COP1,  CTYPE}, {OP_COP2, CTYPE}, {OP_COP3,  CTYPE},
  {OP_RES,     NTYPE}, {OP_RES,   NTYPE}, {OP_RES,  NTYPE}, {OP_RES,   NTYPE},
  {OP_RES,     NTYPE}, {OP_RES,   NTYPE}, {OP_RES,  NTYPE}, {OP_RES,   NTYPE},
  {OP_RES,     NTYPE}, {OP_RES,   NTYPE}, {OP_RES,  NTYPE}, {OP_RES,   NTYPE},
  {OP_LB,      ITYPE}, {OP_LH,    ITYPE}, {OP_LWL,  ITYPE}, {OP_LW,    ITYPE},
  {OP_LBU,     ITYPE}, {OP_LHU,   ITYPE}, {OP_LWR,  ITYPE}, {OP_RES,   NTYPE},
  {OP_SB,      ITYPE}, {OP_SH,    ITYPE}, {OP_SWL,  ITYPE}, {OP_SW,    ITYPE},
  {OP_RES,     NTYPE}, {OP_RES,   NTYPE}, {OP_SWR,  ITYPE}, {OP_RES,   NTYPE},
  {OP_LL,      ITYPE}, {OP_RES,   ITYPE}, {OP_RES,  ITYPE}, {OP_RES,   ITYPE},
  {OP_RES,     NTYPE}, {OP_RES,   NTYPE}, {OP_RES,  NTYPE}, {OP_RES,   NTYPE},
  {OP_SC,      ITYPE}, {OP_RES,   ITYPE}, {OP_RES,  ITYPE}, {OP_RES,   ITYPE},
  {OP_RES,     NTYPE}, {OP_RES,   NTYPE}, {OP_RES,  NTYPE}, {OP_RES,   NTYPE}};

int SPECIALTable[] = {
  OP_SLL,  OP_RES,   OP_SRL,  OP_SRA,  OP_SLLV,    OP_RES,   OP_SRLV, OP_SRAV,
  OP_JR,   OP_JALR,  OP_RES,  OP_RES,  OP_SYSCALL, OP_BREAK, OP_RES,  OP_RES,
  OP_MFHI, OP_MTHI,  OP_MFLO, OP_MTLO, OP_RES,     OP_RES,   OP_RES,  OP_RES,
  OP_MULT, OP_MULTU, OP_DIV,  OP_DIVU, OP_RES,     OP_RES,   OP_RES,  OP_RES,
  OP_ADD,  OP_ADDU,  OP_SUB,  OP_SUBU, OP_AND,     OP_OR,    OP_XOR,  OP_NOR,
  OP_RES,  OP_RES,   OP_SLT,  OP_SLTU, OP_RES,     OP_RES,   OP_RES,  OP_RES,
  OP_RES,  OP_RES,   OP_RES,  OP_RES,  OP_RES,     OP_RES,   OP_RES,  OP_RES,
  OP_RES,  OP_RES,   OP_RES,  OP_RES,  OP_RES,     OP_RES,   OP_RES,  OP_RES};

int BCONDTable[] = {
  OP_BLTZ,   OP_BGEZ,   OP_ILG, OP_ILG, OP_ILG, OP_ILG, OP_ILG, OP_ILG,
  OP_ILG,    OP_ILG,    OP_ILG, OP_ILG, OP_ILG, OP_ILG, OP_ILG, OP_ILG,
  OP_BLTZAL, OP_BGEZAL, OP_ILG, OP_ILG, OP_ILG, OP_ILG, OP_ILG, OP_ILG};

int COPzTable[4][64] = {
  {
  OP_MFC0, OP_MTC0, OP_BC0F, OP_BC0F, OP_COP0, OP_COP0, OP_COP0, OP_COP0,
  OP_MFC0, OP_MTC0, OP_BC0T, OP_BC0T, OP_COP0, OP_COP0, OP_COP0, OP_COP0,
  OP_ILG,  OP_ILG,  OP_ILG,  OP_ILG,  OP_COP0, OP_COP0, OP_COP0, OP_COP0,
  OP_ILG,  OP_ILG,  OP_ILG,  OP_ILG,  OP_COP0, OP_COP0, OP_COP0, OP_COP0,
  OP_CFC0, OP_CTC0, OP_ILG,  OP_ILG,  OP_COP0, OP_COP0, OP_COP0, OP_COP0,
  OP_CFC0, OP_CTC0, OP_ILG,  OP_ILG,  OP_COP0, OP_COP0, OP_COP0, OP_COP0,
  OP_ILG,  OP_ILG,  OP_ILG,  OP_ILG,  OP_COP0, OP_COP0, OP_COP0, OP_COP0,
  OP_ILG,  OP_ILG,  OP_ILG,  OP_ILG,  OP_COP0, OP_COP0, OP_COP0, OP_COP0},
  {
  OP_MFC1, OP_MTC1, OP_BC1F, OP_BC1F, OP_COP1, OP_COP1, OP_COP1, OP_COP1,
  OP_MFC1, OP_MTC1, OP_BC1T, OP_BC1T, OP_COP1, OP_COP1, OP_COP1, OP_COP1,
  OP_ILG,  OP_ILG,  OP_ILG,  OP_ILG,  OP_COP1, OP_COP1, OP_COP1, OP_COP1,
  OP_ILG,  OP_ILG,  OP_ILG,  OP_ILG,  OP_COP1, OP_COP1, OP_COP1, OP_COP1,
  OP_CFC1, OP_CTC1, OP_ILG,  OP_ILG,  OP_COP1, OP_COP1, OP_COP1, OP_COP1,
  OP_CFC1, OP_CTC1, OP_ILG,  OP_ILG,  OP_COP1, OP_COP1, OP_COP1, OP_COP1,
  OP_ILG,  OP_ILG,  OP_ILG,  OP_ILG,  OP_COP1, OP_COP1, OP_COP1, OP_COP1,
  OP_ILG,  OP_ILG,  OP_ILG,  OP_ILG,  OP_COP1, OP_COP1, OP_COP1, OP_COP1},
  {
  OP_MFC2, OP_MTC2, OP_BC2F, OP_BC2F, OP_COP2, OP_COP2, OP_COP2, OP_COP2,
  OP_MFC2, OP_MTC2, OP_BC2T, OP_BC2T, OP_COP2, OP_COP2, OP_COP2, OP_COP2,
  OP_ILG,  OP_ILG,  OP_ILG,  OP_ILG,  OP_COP2, OP_COP2, OP_COP2, OP_COP2,
  OP_ILG,  OP_ILG,  OP_ILG,  OP_ILG,  OP_COP2, OP_COP2, OP_COP2, OP_COP2,
  OP_CFC2, OP_CTC2, OP_ILG,  OP_ILG,  OP_COP2, OP_COP2, OP_COP2, OP_COP2,
  OP_CFC2, OP_CTC2, OP_ILG,  OP_ILG,  OP_COP2, OP_COP2, OP_COP2, OP_COP2,
  OP_ILG,  OP_ILG,  OP_ILG,  OP_ILG,  OP_COP2, OP_COP2, OP_COP2, OP_COP2,
  OP_ILG,  OP_ILG,  OP_ILG,  OP_ILG,  OP_COP2, OP_COP2, OP_COP2, OP_COP2},
  {
  OP_MFC3, OP_MTC3, OP_BC3F, OP_BC3F, OP_COP3, OP_COP3, OP_COP3, OP_COP3,
  OP_MFC3, OP_MTC3, OP_BC3T, OP_BC3T, OP_COP3, OP_COP3, OP_COP3, OP_COP3,
  OP_ILG,  OP_ILG,  OP_ILG,  OP_ILG,  OP_COP3, OP_COP3, OP_COP3, OP_COP3,
  OP_ILG,  OP_ILG,  OP_ILG,  OP_ILG,  OP_COP3, OP_COP3, OP_COP3, OP_COP3,
  OP_CFC3, OP_CTC3, OP_ILG,  OP_ILG,  OP_COP3, OP_COP3, OP_COP3, OP_COP3,
  OP_CFC3, OP_CTC3, OP_ILG,  OP_ILG,  OP_COP3, OP_COP3, OP_COP3, OP_COP3,
  OP_ILG,  OP_ILG,  OP_ILG,  OP_ILG,  OP_COP3, OP_COP3, OP_COP3, OP_COP3,
  OP_ILG,  OP_ILG,  OP_ILG,  OP_ILG,  OP_COP3, OP_COP3, OP_COP3, OP_COP3}
};

int COP0Table[] = {
  OP_COP0, OP_TLBR, OP_TLBWI, OP_ILG, OP_ILG,  OP_ILG,  OP_TLBWR, OP_ILG,
  OP_TLBP, OP_ILG,  OP_ILG,   OP_ILG, OP_ILG,  OP_ILG,  OP_ILG,   OP_ILG,
  OP_RFE,  OP_ILG,  OP_ILG,   OP_ILG, OP_ILG,  OP_ILG,  OP_ILG,   OP_ILG,
  OP_ILG,  OP_ILG,  OP_ILG,   OP_ILG, OP_ILG,  OP_ILG,  OP_ILG,   OP_ILG
};

//	instruction decode function

inline void IDecode(int ins, char *opcode, int *rs, int *rt, int *rd, int *misc)
{
  IFormat *Code;

  Code = &OpcodeTable[(ins >> 26) & 0x3F];

  /* Note:
     it looks like all RTYPE instructions do belong to the SPECIAL
     opcode.  So testing for SPECIAL is equivalent to checking for R-Type.
     This must correspond to some specific hardware decoding, so I'm
     quite confident it is ok to do so.
     The order of the 'ifs' is the order of most used ins
     types, from the DLX book. */
     
  if (Code->format == STYPE) {
     *opcode = SPECIALTable[ins & 0x3F];
     /* misc is 0 for RTYPE but for shifts where it is shamt */
     *misc   = (ins >>  6) & 0x1F;
     *rd     = (ins >> 11) & 0x1F;
     *rt     = (ins >> 16) & 0x1F;
     *rs     = (ins >> 21) & 0x1F;
  } else if (Code->format == ITYPE) {
     *opcode = Code->opcode;
     *rt     = (ins >> 16) & 0x1F;
     *rs     = (ins >> 21) & 0x1F;
     *misc   = ins & 0xFFFF;
     /* Sign:
        extension of 16 bit immediat value into 32 bits.
        All immediat instructions but the logical ones use sign
        extended values. */
     if (*misc & 0x8000)
        *misc |= 0xFFFF0000;
  } else if (Code->format == BTYPE) {
     *opcode = BCONDTable[(ins >> 16) & 0x1F];
     *rs     = (ins >> 21) & 0x1F;
     *misc   = ins & 0xFFFF;
     /* sign extension of 16 bit immediat value */
     if (*misc & 0x8000)
        *misc |= 0xFFFF0000;
  } else if (Code->format == JTYPE) {
     *opcode = Code->opcode;
     /* all jump type insns have this shift performed, so I better do
        it in here where, if miss rates are correct, the calculation
        is performed seldom. */
     *misc   = (ins << 2) & 0x0FFFFFFF;
  } else if (Code->format == CTYPE) {
     if (ins & 0x02000000) {
        if (Code->opcode == OP_COP0)
           *opcode = COP0Table[ins & 0x1F];
        else {
           *opcode = Code->opcode;
           *misc   = ins & 0x01FFFFFF;
        }
     } else {
        *opcode = COPzTable
                 [((ins >> 26) & 0x3)]
                 [((ins >> 17) & 0x30)
                  | ((ins >> 13) & 0x8)
                  | ((ins >> 23) & 0x7)];
        *rd     = (ins >> 11) & 0x1F;
        *rt     = (ins >> 16) & 0x1F;
        *misc   = ins & 0xFFFF;
        /* sign extension of 16 bit immediat value */
        if (*misc & 0x8000)
           *misc |= 0xFFFF0000;
     }
  } else
     *opcode = Code->opcode;
};  // end Idecode

//	Unimplemented function

static void Unimplemented(int pc, char *name)
{
  std::cerr << "Instruction at pc 0x" << std::hex << pc << std::dec
	    << " not implemented in simulator: " << name << "\n";
   sc_stop();
}; // end Unimplemented

///////////////////////////////////////////////////////
//	Processor structure definition
///////////////////////////////////////////////////////
 
template<bool	MULTI_CONTEXT,
	 int 	N_CTXT,
 	 int	TIME_OUT>

struct SOCLIB_MULTI_MIPS:sc_module {
				
	//	I/O PORTS  
	
	sc_in<bool> 			CLK;
	sc_in<bool> 			RESETN;
	ICACHE_PROCESSOR_PORTS 		ICACHE; // instruction cache
	DCACHE_PROCESSOR_PORTS 		DCACHE; // data cache
	sc_in<bool>   			IT_5;	// IRQ 5
	sc_in<bool>   			IT_4;	// IRQ 4
	sc_in<bool>   			IT_3;	// IRQ 3
	sc_in<bool>   			IT_2;	// IRQ 2
	sc_in<bool>   			IT_1;	// IRQ 1
	sc_in<bool>   			IT_0;	// IRQ 0
  	sc_in<int>			I_FRZ;  // for multi-context only
  	sc_in<int>			D_FRZ;  // for multi-context only
   
	//	STRUCTURAL PARAMETERS
	
	const	char 			*NAME;		 // Name of the current instance 
	int				NBC;	 // number of harware contexts
	int				MSB_NUMBER;	 // number of address MSB bits
	int				MSB_MASK;	 // mask for the uncached table index
   	int				 IDENT[N_CTXT];		// Processor Identification 

	//	REGISTERS
	
   	sc_signal<int> GPR[32][N_CTXT];	// General Purpose Registers 
   	sc_signal<int> PC[N_CTXT];		// Program Counter 
   	sc_signal<int> HI[N_CTXT];		// HI and LO part of the multiplication results 
   	sc_signal<int> LO[N_CTXT];		// or result of Division and Modulo 
   	sc_signal<int> IR[N_CTXT];		// Instruction Register   
   
   	sc_signal<int> STATUS[N_CTXT];	// Status register 
   	sc_signal<int> CAUSE[N_CTXT];		// Cause of the last interupt/exception 
   	sc_signal<int> EPC[N_CTXT];		// PC where the exception occured 
   	sc_signal<int> BADVADDR[N_CTXT];	// Addr when an Address error occurs 
   
   	sc_signal<int> MC_CTXT;         // FSM state for the context controler
   	sc_signal<int> MC_COUNT;	// TIME_OUT counter for the context controler

///////////////////////////////////////////////////////////
//	constructor
///////////////////////////////////////////////////////////

SC_HAS_PROCESS(SOCLIB_MULTI_MIPS);

SOCLIB_MULTI_MIPS(	sc_module_name 		insname, 
			int 			ident)
{
	SC_METHOD (transition);
  dont_initialize();
	sensitive_pos << CLK;

	SC_METHOD (genMoore);
  dont_initialize();
	sensitive_neg << CLK;
	
if(MULTI_CONTEXT == true) {
	if((N_CTXT > 32) || (N_CTXT < 1)) {
	       perror("Error in the SOCLIB_GENMIPSR3000 component\n");
	       perror("The N_CTXT parameter must be larger than 0\n");
	       perror("and cannot be larger than 32\n");
	       sc_stop();
	}
	NBC = N_CTXT;
} else {
	NBC = 1;
}

  NAME  = (const char*) insname;
  for (int i = 0 ; i < NBC ; i++) 
{ 
		IDENT[i] = ident + i;
#ifdef NONAME_RENAME

	char name[100];

	for (int j=0 ; j < 32; j++)
	  {
		  sprintf(name,"GPR_%2.2d_%2.2d",j,i);
		  GPR[i][j].rename(name);
	  }
	  sprintf(name,"PC_%2.2d",i);
	  PC[i].rename(name);
	  sprintf(name,"HI_%2.2d",i);
	  HI[i].rename(name);
	  sprintf(name,"LO_%2.2d",i);
	  LO[i].rename(name);
	  sprintf(name,"IR_%2.2d",i);
	  IR[i].rename(name);
	  sprintf(name,"STATUS_%2.2d",i);
	  STATUS[i].rename(name);
	  sprintf(name,"CAUSE_%2.2d",i);
	  CAUSE[i].rename(name);
	 sprintf(name,"EPC_%2.2d",i);
	  EPC[i].rename(name);
	  sprintf(name,"BADVADDR_%2.2d",i);
	  BADVADDR[i].rename(name);
	  MC_CTXT.rename("MC_CTXT");
	  MC_COUNT.rename("MC_COUNT");
#endif

}

  printf("Successful Instanciation of SOCLIB_MULTI_MIPS : %s\n",NAME);

};  // end constructor

void print_regs()
{
	printf("PC=%8.8x IR=%8.8x\n",PC[0].read(),IR[0].read());
	for (int i=0;i<4;i++)
	{
		for (int j=0;j<8;j++)
			printf("R[%2.2d]=%8.8x ",i*8+j,(unsigned int)GPR[0][i*8+j].read());
		puts("");
	}
}

///////////////////////////////////////////////////////
//	Transition()
///////////////////////////////////////////////////////

void transition()
{
  // next address 
  int next_pc = 0;
  
  // Variables for instruction decode
  char ins_opcode;
  int  ins_rs, ins_rt, ins_rd, ins_misc;
  
  // Variables for exception/interrution/trap handling
  int exception_signal = 0;
  int exception_adress = 0;
  int interrupt_signal = 0;
  int status_modified  = 0;

/////////////////////////////
//	RESET 
/////////////////////////////

if(RESETN == false) {
    for (int i = 0 ; i < NBC ; i++) {
	  	for (int j = 0; j < 32; j++) { GPR[j][i] = 0; }
	    	STATUS[i]   = 0; 
		HI[i]       = 0;
		LO[i]       = 0;
		IR[i]       = 0; 
	    	CAUSE[i]    = 0;
	    	EPC[i]      = 0;
	    	BADVADDR[i] = 0;
    		PC[i]       = R3000_RESET_VECTOR;
	}
    MC_CTXT  = 0;
    MC_COUNT = 0;
    return;
};  // end RESET

////////////////////////////////////////////////////////
//	CONTEXT CONTROLER
// The Context Controler contains 2 registers :
// - MC_CTXT   : the controler can be in NBC states	
// In state i (with i in [0,1...,NBC -1], the
// hardware context i is running.
// - MC_COUNT  : the timeout counter is set to 0 at each
// context switch. It is incremented at each cycle.
// A context switch is forced when MC_COUNT = TIME_OUT
////////////////////////////////////////////////////////

if(MULTI_CONTEXT == true) {
	
	if((MC_COUNT >= TIME_OUT) ||
	   (((int)I_FRZ.read() & CTXT_MASK[MC_CTXT]) != 0) ||
	   (((int)D_FRZ.read() & CTXT_MASK[MC_CTXT]) != 0)) { // context switch
		MC_COUNT = 0;
		for(int i = MC_CTXT + 1 ; i < MC_CTXT + NBC ; i++) {
	     		if((((int)I_FRZ.read() & CTXT_MASK[i]) == 0) &&
	                   (((int)D_FRZ.read() & CTXT_MASK[i]) == 0)) {
				MC_CTXT = i % NBC;
				break;
			} // end if
		} // end for 
	} else {				  // no context switch
		MC_COUNT = MC_COUNT + 1;
	}
}  // end if MULTI_CONTEXT
	
//////////////////////////////////////////////
//	NO REGISTER WRITE IN CASE OF MISS
//////////////////////////////////////////////
  
 if ((ICACHE.FRZ == true) || (DCACHE.FRZ == true)) {
     if(ICACHE.BERR == true)
         printf("Ignored Instruction BERR @ 0x%08x\n", (int)PC[MC_CTXT].read() );
     if(DCACHE.BERR == true)
         printf("Ignored Data BERR @ 0x%08x (PC=0x%08x)\n",
                (int)DCACHE.ADR.read(), (int)PC[MC_CTXT].read() );
     return;
 }

printf("PC=%x\n",(int)PC[MC_CTXT].read());
//puts("Trans");
//print_regs();
//////////////////////////////////////
//	INSTRUCTION DECODE
//////////////////////////////////////

IDecode(IR[MC_CTXT], &ins_opcode, &ins_rs, &ins_rt, &ins_rd, &ins_misc);
  
  /* Prise en compte des interruptions */
  
  interrupt_signal = (int)IT_0.read() << 10 |
                     (int)IT_1.read() << 11 | 
                     (int)IT_2.read() << 12 | 
                     (int)IT_3.read() << 13 | 
                     (int)IT_4.read() << 14 | 
                     (int)IT_5.read() << 15 ;

  /*
   * Cette section correspond au calcul de la valeur à écrire dans les 
   * registres généraux, c'est à dire à l'exécution de l'instruction
   * qui a lieu dans le 2e étage du pipeline :
   * Suivant l'opcode obtenu après décodage de l'instruction, 
   * on exécute la portion de code associée à celui-ci,
   * on détecte les exceptions, et on écrit dans le registre général
   * lorsqu'il n'y a pas exception.
   * On calcule aussi la valeur future du PC.
   */

  switch (ins_opcode) {
    
    case OP_ADD:
    {
      int s = GPR[ins_rs][MC_CTXT] + GPR[ins_rt][MC_CTXT];
      if (!((GPR[ins_rs][MC_CTXT] ^ GPR[ins_rt][MC_CTXT]) & 0x80000000)
           && ((GPR[ins_rs][MC_CTXT] ^ s) & 0x80000000)) {
       		exception_signal = X_OVF;
         	break;
      }
      // register update only if no exception occurs
      if(ins_rd != 0) { GPR[ins_rd][MC_CTXT] = s; }
      next_pc = PC[MC_CTXT] + 4;
      break;
    }
    
    case OP_ADDI:
    {
      int s = GPR[ins_rs][MC_CTXT] + ins_misc;
      if (!((GPR[ins_rs][MC_CTXT] ^ ins_misc) & 0x80000000)
           && ((GPR[ins_rs][MC_CTXT] ^ s) & 0x80000000)) {
            	exception_signal = X_OVF;
            	break;
      }
      // register update only if no exception occurs
      if(ins_rt != 0) { GPR[ins_rt][MC_CTXT] = s; }
      next_pc = PC[MC_CTXT] + 4;
      break;
    }

    case OP_ADDIU:
      if(ins_rt != 0) { GPR[ins_rt][MC_CTXT] = GPR[ins_rs][MC_CTXT] + ins_misc; }
      next_pc = PC[MC_CTXT] + 4;
      break;

    case OP_ADDU:
      if(ins_rd != 0) { GPR[ins_rd][MC_CTXT] = GPR[ins_rs][MC_CTXT] + GPR[ins_rt][MC_CTXT]; }
      next_pc = PC[MC_CTXT] + 4;
      break;

    case OP_AND:
      if(ins_rd != 0) { GPR[ins_rd][MC_CTXT] = GPR[ins_rs][MC_CTXT] & GPR[ins_rt][MC_CTXT]; }
      next_pc = PC[MC_CTXT] + 4;
      break;

    case OP_ANDI:
      if(ins_rt != 0) { GPR[ins_rt][MC_CTXT] = GPR[ins_rs][MC_CTXT] & ins_misc & 0xFFFF; }
      next_pc = PC[MC_CTXT] + 4;
      break;

    case OP_BC0F:
      Unimplemented(PC[MC_CTXT], "bc0f");
      break;

    case OP_BC1F:
      Unimplemented(PC[MC_CTXT], "bc1f");
      break;

    case OP_BC2F:
      Unimplemented(PC[MC_CTXT], "bc2f");
      break;

    case OP_BC3F:
      Unimplemented(PC[MC_CTXT], "bc3f");
      break;

    case OP_BC0T:
      Unimplemented(PC[MC_CTXT], "bc0t");
      break;

    case OP_BC1T:
      Unimplemented(PC[MC_CTXT], "bc1t");
      break;

    case OP_BC2T:
      Unimplemented(PC[MC_CTXT], "bc2t");
      break;

    case OP_BC3T:
      Unimplemented(PC[MC_CTXT], "bc3t");
      break;

    case OP_BEQ:        
      if (GPR[ins_rs][MC_CTXT] == GPR[ins_rt][MC_CTXT]) { 
          next_pc = PC[MC_CTXT] + (ins_misc << 2);
      } else {
          next_pc = PC[MC_CTXT] + 4;
      }
      break;

    case OP_BGEZ:
      if ((GPR[ins_rs][MC_CTXT] & 0x80000000) == 0x0) { 
          next_pc = PC[MC_CTXT] + (ins_misc << 2);
      } else {
          next_pc = PC[MC_CTXT] + 4;
      }
      break;

    case OP_BGEZAL:
      if ((GPR[ins_rs][MC_CTXT] & 0x80000000) == 0x0) { 
          next_pc = PC[MC_CTXT] + (ins_misc << 2);
          GPR[31][MC_CTXT] = PC[MC_CTXT] + 4;
      } else {
          next_pc = PC[MC_CTXT] + 4;
      }
      break;

    case OP_BGTZ:
      if (((GPR[ins_rs][MC_CTXT] & 0x80000000) == 0x0) && 
           (GPR[ins_rs][MC_CTXT] != 0)) { 
          next_pc = PC[MC_CTXT] + (ins_misc << 2);
      } else {
          next_pc = PC[MC_CTXT] + 4;
      }
      break;

    case OP_BLEZ:
      if (((GPR[ins_rs][MC_CTXT] & 0x80000000) == 0x80000000) || 
            (GPR[ins_rs][MC_CTXT] == 0)) { 
          next_pc = PC[MC_CTXT] + (ins_misc << 2);
      } else {
          next_pc = PC[MC_CTXT] + 4;
      }
      break;

    case OP_BLTZ:
      if ((GPR[ins_rs][MC_CTXT] & 0x80000000) == 0x80000000) {
          next_pc = PC[MC_CTXT] + (ins_misc << 2);
      } else {
          next_pc = PC[MC_CTXT] + 4;
      }
      break;

    case OP_BLTZAL:
      if ((GPR[ins_rs][MC_CTXT] & 0x80000000) == 0x80000000) {
          next_pc = PC[MC_CTXT] + (ins_misc << 2);
          GPR[31][MC_CTXT] = PC[MC_CTXT] + 4;
      } else {
          next_pc = PC[MC_CTXT] + 4;
      }
      break;
         
    case OP_BNE:
      if (GPR[ins_rs][MC_CTXT] != GPR[ins_rt][MC_CTXT]) { 
          next_pc = PC[MC_CTXT] + (ins_misc << 2);
      } else {
          next_pc = PC[MC_CTXT] + 4;
      }
      break;

    case OP_BREAK:
      next_pc = PC[MC_CTXT] - 4;
      exception_signal = X_BP;
      break;

    case OP_CFC0:
      Unimplemented(PC[MC_CTXT], "cfc0");
      break;

    case OP_CFC1:
      Unimplemented(PC[MC_CTXT], "cfc1");
      break;

    case OP_CFC2:
      Unimplemented(PC[MC_CTXT], "cfc2");
      break;

    case OP_CFC3:
      Unimplemented(PC[MC_CTXT], "cfc3");
      break;

    case OP_COP0:
      Unimplemented(PC[MC_CTXT], "cop0");
      break;

    case OP_COP1:
      Unimplemented(PC[MC_CTXT], "cop1");
      break;

    case OP_COP2:
      Unimplemented(PC[MC_CTXT], "cop2");
      break;

    case OP_COP3:
      Unimplemented(PC[MC_CTXT], "cop3");
      break;

    case OP_CTC0:
      Unimplemented(PC[MC_CTXT], "ctc0");
      break;

    case OP_CTC1:
      Unimplemented(PC[MC_CTXT], "ctc1");
      break;

    case OP_CTC2:
      Unimplemented(PC[MC_CTXT], "ctc2");
      break;

    case OP_CTC3:
      Unimplemented(PC[MC_CTXT], "ctc3");
      break;
         
    case OP_DIV:
      if (GPR[ins_rt][MC_CTXT]) {
            LO[MC_CTXT] = GPR[ins_rs][MC_CTXT] / GPR[ins_rt][MC_CTXT];
            HI[MC_CTXT] = GPR[ins_rs][MC_CTXT] % GPR[ins_rt][MC_CTXT];
      } else {
            LO[MC_CTXT] = random();
            HI[MC_CTXT] = random();
      }
      next_pc = PC[MC_CTXT] + 4;
      break;

    case OP_DIVU:
      {
      unsigned int rs, rt;
      rs = (unsigned int)GPR[ins_rs][MC_CTXT];
      rt = (unsigned int)GPR[ins_rt][MC_CTXT];
      if (rt) {
            LO[MC_CTXT] = (int) ((unsigned int)(rs / rt));
            HI[MC_CTXT] = (int) ((unsigned int)(rs % rt));
      } else {
            LO[MC_CTXT] = random();
            HI[MC_CTXT] = random();
      }
      next_pc = PC[MC_CTXT] + 4;
      break;
      }

    case OP_J:
      // ins_misc  is multiplied by 4 in the Idecode function
      next_pc = (PC[MC_CTXT] & 0XF0000000) | ins_misc;
      break;

    case OP_JAL:
      // ins_misc  is multiplied by 4 in the Idecode function
      next_pc = (PC[MC_CTXT] & 0XF0000000) | ins_misc;
      GPR[31][MC_CTXT] = PC[MC_CTXT] + 4;
      break;

    case OP_JALR:
      if (ins_rd != 0) { GPR[ins_rd][MC_CTXT] = PC[MC_CTXT] + 4; }
      next_pc = GPR[ins_rs][MC_CTXT];
      break;

    case OP_JR:
      next_pc = GPR[ins_rs][MC_CTXT];
      break;

    case OP_LB:
      {
      // address computation is only for exception 
      int adr, data;
      adr = GPR[ins_rs][MC_CTXT] + ins_misc;
      if ((STATUS[MC_CTXT] & 0x2) && (adr & 0x80000000)) {
       		exception_signal = X_ADEL;
       		exception_adress = adr;
       		break;
      } else if (DCACHE.BERR == true) {
		exception_signal = X_DBE;
       		exception_adress = adr;
	 	break;
      }
      if (ins_rt != 0) {
      		data = (int)DCACHE.RDATA.read() >> (8 * (adr & 0x3));
         	GPR[ins_rt][MC_CTXT] = ((data & 0x00000080)
                             ? (data | 0xFFFFFF00)
                             : (data & 0x000000FF)); } 
      next_pc = PC[MC_CTXT] + 4;
      break;
      }

    case OP_LBU:
      {
      // address computation is only for exception 
      int adr, data;
      adr = GPR[ins_rs][MC_CTXT] + ins_misc;
      if ((STATUS[MC_CTXT] & 0x2) && (adr & 0x80000000)) {
         	exception_signal = X_ADEL;
         	exception_adress = adr;
         	break;
      } else if (DCACHE.BERR == true) {
	 	exception_signal = X_DBE;
         	exception_adress = adr;
	 	break;
      }
      if (ins_rt != 0){
         	data = (int)DCACHE.RDATA.read() >> (8 * (adr & 0x3));
         	GPR[ins_rt][MC_CTXT] = data & 0x000000FF; } 
      next_pc = PC[MC_CTXT] + 4;
      break;
      }

    case OP_LH:
      {
      // address computation is only for exception 
      int adr, data;
      adr = GPR[ins_rs][MC_CTXT] + ins_misc;
      if ((adr & 0x1) || ((STATUS[MC_CTXT] & 0x2) && (adr & 0x80000000))) {
         	exception_signal = X_ADEL;
         	exception_adress = adr;
         	break;
      } else if (DCACHE.BERR == true) {
	 	exception_signal = X_DBE;
         	exception_adress = adr;
		break;
      }
      if (ins_rt != 0){
         	data = (int)DCACHE.RDATA.read() >> (8 * (adr & 0x2));
         	GPR[ins_rt][MC_CTXT] = ((data & 0x00008000)
                             ? (data | 0xFFFF0000)
                             : (data & 0x0000FFFF)); } 
      next_pc = PC[MC_CTXT] + 4;
      break;
      }

      case OP_LHU:
      {
      // address computation is only for exception 
      int adr, data;
      adr = GPR[ins_rs][MC_CTXT] + ins_misc;
      if ((adr & 0x1) || ((STATUS[MC_CTXT] & 0x2) && (adr & 0x80000000))) {
         	exception_signal = X_ADEL;
         	exception_adress = adr;
         	break;
      } else if (DCACHE.BERR == true) {
	 	exception_signal = X_DBE;
         	exception_adress = adr;
	 	break;
      }
      if (ins_rt != 0){
         	data = (int)DCACHE.RDATA.read() >> (8 * (adr & 0x2));
         	GPR[ins_rt][MC_CTXT] = data & 0x0000FFFF; } 
      next_pc = PC[MC_CTXT] + 4;
      break;
      }

    case OP_LUI:
      if (ins_rt != 0){ GPR[ins_rt][MC_CTXT] = ins_misc << 16; } 
      next_pc = PC[MC_CTXT] + 4;
      break;

    case OP_LW:
      {
      // address computation is only for exception 
      int adr = GPR[ins_rs][MC_CTXT] + ins_misc;
      if ((adr & 0x3) || ((STATUS[MC_CTXT] & 0x2) && (adr & 0x80000000))) {
         	exception_signal = X_ADEL;
         	exception_adress = adr;
         	break;
      } else if (DCACHE.BERR == true) {
	 	exception_signal = X_DBE;
         	exception_adress = adr;
	 	break;
      }
      if (ins_rt != 0){ GPR[ins_rt][MC_CTXT] = (int)DCACHE.RDATA.read(); }
      next_pc = PC[MC_CTXT] + 4;
      break;
      }

    case OP_LWC0:
      Unimplemented(PC[MC_CTXT], "lwc0");
      break;

    case OP_LWC1:
      Unimplemented(PC[MC_CTXT], "lwc1");
      break;

    case OP_LWC2:
      Unimplemented(PC[MC_CTXT], "lwc2");
      break;

    case OP_LWC3:
      Unimplemented(PC[MC_CTXT], "lwc3");
      break;

    case OP_LWL:
      Unimplemented(PC[MC_CTXT], "lwl");
      break;

    case OP_LWR:
      Unimplemented(PC[MC_CTXT], "lwr");
      break;
     
    case OP_MFC0:
      if (STATUS[MC_CTXT] & 0x2) { // User mode 
           exception_signal = X_CPU;
           break;
      }
      next_pc = PC[MC_CTXT] + 4;
      if (ins_rt != 0) {
          switch (ins_rd) {
            case 0:
            GPR[ins_rt][MC_CTXT] = IDENT[MC_CTXT];
            break;
            case 8:
            GPR[ins_rt][MC_CTXT] = BADVADDR[MC_CTXT];
            break;
            case 12:
            GPR[ins_rt][MC_CTXT] = STATUS[MC_CTXT];
            break;
            case 13:
            GPR[ins_rt][MC_CTXT] = CAUSE[MC_CTXT];
            break;
            case 14:
            GPR[ins_rt][MC_CTXT] = EPC[MC_CTXT];
            break;
            default:
            Unimplemented(PC[MC_CTXT], "mfc0");
          } // end switch
      } // end if
      break;

    case OP_MFC1:
      Unimplemented(PC[MC_CTXT], "mfc1");
      break;

    case OP_MFC2:
      Unimplemented(PC[MC_CTXT], "mfc2");
      break;

    case OP_MFC3:
      Unimplemented(PC[MC_CTXT], "mfc3");
      break;

    case OP_MFHI:
      next_pc = PC[MC_CTXT] + 4;
      if (ins_rd != 0) { GPR[ins_rd][MC_CTXT] =  HI[MC_CTXT]; }
      break;

    case OP_MFLO:
      next_pc = PC[MC_CTXT] + 4;
      if (ins_rd != 0) { GPR[ins_rd][MC_CTXT] =  LO[MC_CTXT]; }
      break;
         
    case OP_MTC0:
      if (STATUS[MC_CTXT] & 0x2) {     /* User mode */
        	exception_signal = X_CPU;
        	break;
      }
      next_pc = PC[MC_CTXT] + 4;
      status_modified = OP_MTC0;
      break;

    case OP_MTC1:
      Unimplemented(PC[MC_CTXT], "mtc1");
      break;

    case OP_MTC2:
      Unimplemented(PC[MC_CTXT], "mtc2");
      break;

    case OP_MTC3:
      Unimplemented(PC[MC_CTXT], "mtc3");
      break;

    case OP_MTHI:
      next_pc = PC[MC_CTXT] + 4;
      HI[MC_CTXT] =  GPR[ins_rs][MC_CTXT];
      break;

    case OP_MTLO:
      next_pc = PC[MC_CTXT] + 4;
      LO[MC_CTXT] =  GPR[ins_rs][MC_CTXT];
      break;
         
    case OP_MULT:
      { 
      long long unsigned int a,b,c;
      a = GPR[ins_rs][MC_CTXT];
      b = GPR[ins_rt][MC_CTXT];
      c = a*b;
      LO[MC_CTXT] = (unsigned int)c;
      c >>= 32;
      HI[MC_CTXT] = (unsigned int)c;
      next_pc = PC[MC_CTXT] + 4;
      break;
      }

    case OP_MULTU:
      {
      int xlo = GPR[ins_rs][MC_CTXT];
      int ylo = GPR[ins_rt][MC_CTXT];
      unsigned int rhi, rlo;            // result 
      unsigned int nb;      
      unsigned int yhi;     

      rlo = 0;
      rhi = 0;
      yhi = 0;
      nb = 31;
      while (nb > 0) {
            if (xlo & 1) {
               unsigned int ihi, ilo;
               ilo = rlo + ylo;
               ihi = ilo < rlo ? 1 : 0;
               ihi += rhi + yhi;
               rhi = ihi;
               rlo = ilo;
            }
            xlo >>= 1;
            yhi <<= 1;
            yhi |= ((ylo >> 31) & 0x1);
            ylo <<= 1;
            nb--;
      }
      HI[MC_CTXT] = rhi;
      LO[MC_CTXT] = rlo;
      next_pc = PC[MC_CTXT] + 4;
      break;
      }

    case OP_NOR:
      next_pc = PC[MC_CTXT] + 4;
      if (ins_rd != 0) { GPR[ins_rd][MC_CTXT] = ~(GPR[ins_rs][MC_CTXT] | GPR[ins_rt][MC_CTXT]); }
      break;

    case OP_OR:
      next_pc = PC[MC_CTXT] + 4;
      if (ins_rd != 0) { GPR[ins_rd][MC_CTXT] = GPR[ins_rs][MC_CTXT] | GPR[ins_rt][MC_CTXT]; }
      break;

    case OP_ORI:
      next_pc = PC[MC_CTXT] + 4;
      if (ins_rt != 0) { GPR[ins_rt][MC_CTXT] = GPR[ins_rs][MC_CTXT] | (ins_misc & 0xFFFF); }
      break;
    
    case OP_RFE:
      if (STATUS[MC_CTXT] & 0x2) {      // User mode 
            exception_signal = X_CPU;
            break;
      }
      status_modified = OP_RFE;
      next_pc = PC[MC_CTXT] + 4;
      break;

    case OP_SB:
      {
      int adr = GPR[ins_rs][MC_CTXT] + ins_misc;
      if ((STATUS[MC_CTXT] & 0x2) && (adr & 0x80000000)) {
           	exception_signal = X_ADES;
          	exception_adress = adr;
          	break;
      }
      next_pc = PC[MC_CTXT] + 4;
      break;
      }
      
    case OP_SH:
      {
      int adr = GPR[ins_rs][MC_CTXT] + ins_misc;
      if ((STATUS[MC_CTXT] & 0x2) && (adr & 0x80000000)) {
           	exception_signal = X_ADES;
          	exception_adress = adr;
          	break;
      }
      next_pc = PC[MC_CTXT] + 4;
      break;
      }

    case OP_SLL:
      if (ins_rd != 0) { GPR[ins_rd][MC_CTXT] = GPR[ins_rt][MC_CTXT] << ins_misc;} 
      next_pc = PC[MC_CTXT] + 4;
      break;

    case OP_SLLV:
      if (ins_rd != 0) { GPR[ins_rd][MC_CTXT] = GPR[ins_rt][MC_CTXT] << (GPR[ins_rs][MC_CTXT] & 0x1F);}
      next_pc = PC[MC_CTXT] + 4;
      break;

    case OP_SLT:
      if (ins_rd != 0) { GPR[ins_rd][MC_CTXT] = (GPR[ins_rs][MC_CTXT] < GPR[ins_rt][MC_CTXT] ? 1 : 0);}
      next_pc = PC[MC_CTXT] + 4;
      break;

    case OP_SLTI:
      if (ins_rt != 0) { GPR[ins_rt][MC_CTXT] = (GPR[ins_rs][MC_CTXT] < ins_misc ? 1 : 0);}
      next_pc = PC[MC_CTXT] + 4;
      break;

    case OP_SLTIU:
      if (ins_rt != 0) { GPR[ins_rt][MC_CTXT] = 
	      ((unsigned int)GPR[ins_rs][MC_CTXT] < (unsigned int)ins_misc ? 1 : 0);}
      next_pc = PC[MC_CTXT] + 4;
      break;

    case OP_SLTU:
      if (ins_rd != 0) { GPR[ins_rd][MC_CTXT] = 
	      ((unsigned int)GPR[ins_rs][MC_CTXT] < (unsigned int)GPR[ins_rt][MC_CTXT] ? 1 : 0);}
      next_pc = PC[MC_CTXT] + 4;
      break;

    case OP_SRA:
      if (ins_rd != 0) { GPR[ins_rd][MC_CTXT] = GPR[ins_rt][MC_CTXT] >> ins_misc; }
      next_pc = PC[MC_CTXT] + 4;
      break;

    case OP_SRAV:
      if (ins_rd != 0) { GPR[ins_rd][MC_CTXT] = GPR[ins_rt][MC_CTXT] >> (GPR[ins_rs][MC_CTXT] & 0x1F);}
      next_pc = PC[MC_CTXT] + 4;
      break;

    case OP_SRL:
      if (ins_rd != 0) { GPR[ins_rd][MC_CTXT] = ( ((unsigned int) (GPR[ins_rt][MC_CTXT])) >> ins_misc) 
                                                  & (0x7FFFFFFF >> (ins_misc - 1));}
      next_pc = PC[MC_CTXT] + 4;
      break;

    case OP_SRLV:
      if (ins_rd != 0) { GPR[ins_rd][MC_CTXT] = (((unsigned int) (GPR[ins_rt][MC_CTXT])) >> (GPR[ins_rs][MC_CTXT] & 0x1F))
                                                  & (0x7FFFFFFF >> ((GPR[ins_rs][MC_CTXT] & 0x1F) - 1));}
      next_pc = PC[MC_CTXT] + 4;
      break;

    case OP_SUB:
      {
      int s = GPR[ins_rs][MC_CTXT] - GPR[ins_rt][MC_CTXT];
      if (!((GPR[ins_rs][MC_CTXT] ^ GPR[ins_rt][MC_CTXT]) & 0x80000000)
           && ((GPR[ins_rs][MC_CTXT] ^ s) & 0x80000000)) {
        	exception_signal = X_OVF;
         	break;
      }
      // register update only if no exception occurs 
      if(ins_rd != 0) { GPR[ins_rd][MC_CTXT] = s; }
      next_pc = PC[MC_CTXT] + 4;
      break;
      }
  
    case OP_SUBU:
      if(ins_rd != 0) { GPR[ins_rd][MC_CTXT] = GPR[ins_rs][MC_CTXT] - GPR[ins_rt][MC_CTXT]; }
      next_pc = PC[MC_CTXT] + 4;
      break;

    case OP_SW:
      {
      int adr = GPR[ins_rs][MC_CTXT] + ins_misc;
      if ((STATUS[MC_CTXT] & 0x2) && (adr & 0x80000000)) {
           	exception_signal = X_ADES;  
          	exception_adress = adr;
          	break;
      }
      next_pc = PC[MC_CTXT] + 4;
      break;
      }
           
      case OP_SC:
      {
      int adr = GPR[ins_rs][MC_CTXT] + ins_misc;
      if ((STATUS[MC_CTXT] & 0x2) && (adr & 0x80000000)) {
           	exception_signal = X_ADES;  
          	exception_adress = adr;
          	break;
      }
      next_pc = PC[MC_CTXT] + 4;
      break;
      }
          
    case OP_SWC0:
      Unimplemented(PC[MC_CTXT], "swc0");
      break;

    case OP_SWC1:
      Unimplemented(PC[MC_CTXT], "swc1");
      break;

    case OP_SWC2:
      Unimplemented(PC[MC_CTXT], "swc2");
      break;

    case OP_SWC3:
      Unimplemented(PC[MC_CTXT], "swc3");
      break;

    case OP_SWL:
      Unimplemented(PC[MC_CTXT], "swl");
      break;

    case OP_SWR:
      Unimplemented(PC[MC_CTXT], "swr");
      break;

    case OP_SYSCALL:
      next_pc = PC[MC_CTXT] - 4;
      exception_signal = X_SYS;
      break;

    case OP_XOR:
      next_pc = PC[MC_CTXT] + 4;
      if (ins_rd != 0) { GPR[ins_rd][MC_CTXT] = GPR[ins_rs][MC_CTXT] ^ GPR[ins_rt][MC_CTXT]; }
      break;

    case OP_XORI:
      next_pc = PC[MC_CTXT] + 4;
      if (ins_rt != 0) { GPR[ins_rt][MC_CTXT] = GPR[ins_rs][MC_CTXT] ^ (ins_misc & 0xFFFF); }
      break;

    case OP_TLBP:
      Unimplemented(PC[MC_CTXT], "tlbp");
      break;

    case OP_TLBR:
      Unimplemented(PC[MC_CTXT], "tlbr");
      break;

    case OP_TLBWI:
      Unimplemented(PC[MC_CTXT], "tlbwi");
      break;

    case OP_TLBWR:
      Unimplemented(PC[MC_CTXT], "tlbwr");
      break;

    case OP_RES:
      exception_signal = X_RI;
      std::cerr << "Error in component MIPSR3000 : Reserved operation\n";
      std::cerr << "at PC = " << std::hex <<PC[MC_CTXT] << std::dec <<"\n";
      //sc_stop();
      break;

    case OP_ILG: 
      exception_signal = X_RI;
      std::cerr << "Error in component MIPSR3000 : Illegal instruction\n";
      std::cerr << "at PC = " << std::hex << PC[MC_CTXT] << std::dec << "\n";
      //sc_stop();
      break;

    default:
      exception_signal = X_RI;
      std::cerr << "Error in component MIPSR3000 : Illegal codop\n";
      std::cerr << "at PC = " << std::hex << PC[MC_CTXT] << std::dec << "\n";
      //sc_stop();
   } // end switch codop
//if (IDENT[MC_CTXT] == 0) printf("IDENT = %d   PC = %x\n", IDENT[MC_CTXT], PC[MC_CTXT].read());
  /**********************************************************************
   * On calcule maintenant les valeurs des registres PC, IR, et des
   * registres du CP0 IDENT, STATUS, CAUSE, EPC et BADVADR
   *
   * Le registre IR recopie la valeur venant du cache instruction
   * ICACHE.INS, sauf si il y a une exception, une interruption ou un
   * appel système. Dans ce cas, on force le code NOP (0<=0+0) car
   * l'instruction qui entre dans le pipeline ne doit pas etre exécutée
   * ********************************************************************/

   if( (exception_signal == 0) && (ICACHE.BERR == true)) {
	exception_signal = X_IBE;
	}
   if( (exception_signal == 0) && (DCACHE.BERR == true)) {
	exception_signal = X_DBE;
	}
 
   if (exception_signal != 0) {

  /*  Exception handling
   *
   *  On regarde si une exception a été levée.
   *  L'exception X_DBE (erreur mémoire lors d'une lecture ou d'une écriture
   *  de donnée) est asynchrone", à cause du tampon d'écritures postées.
   *  Toutes les autres exceptions sont "synchrones", et l'instruction fautive
   *  peut être signalée de façon précise. Parmi les exceptions précises,
   *  on ne prend en compte l'exception X_IBE (erreur mémoire lors du fetch
   *  instruction) que si aucune autre exception n'a été levée. En effet 
   *  l'exception X_IBE est détectée dans le premier étage du pipe-line, alors
   *  que toutes les autres sont détectées dans le deuxième étage.
   *  En cas d'exception, on sauvegarde l'adresse de l'instruction fautive dans EPC,
   *  on branche au gestionnaire d'exceptions, on indique la cause
   *  dans le registre de cause, et on force un NOP dans le registre instruction.
   */ 

    switch (exception_signal) {
      case X_SYS :
        CAUSE[MC_CTXT]  = interrupt_signal | (X_SYS << 2);
        STATUS[MC_CTXT] = (STATUS[MC_CTXT] & ~0x3F) | ((STATUS[MC_CTXT] << 2) & 0x3C); 
        EPC[MC_CTXT]    = PC[MC_CTXT]; 
        PC[MC_CTXT]     = R3000_GENERAL_EXCEPTION_VECTOR;
        IR[MC_CTXT]     = IR_NOP; 
      break;
      
      case X_BP  :
        CAUSE[MC_CTXT]  = interrupt_signal | (X_BP  << 2);
        STATUS[MC_CTXT] = (STATUS[MC_CTXT] & ~0x3F) | ((STATUS[MC_CTXT] << 2) & 0x3C); 
        EPC[MC_CTXT]    = PC[MC_CTXT]; 
        PC[MC_CTXT]     = R3000_GENERAL_EXCEPTION_VECTOR;
        IR[MC_CTXT]     = IR_NOP; 
      break;
      
      case X_OVF :
        CAUSE[MC_CTXT]  = interrupt_signal | (X_OVF  << 2);
        STATUS[MC_CTXT] = (STATUS[MC_CTXT] & ~0x3F) | ((STATUS[MC_CTXT] << 2) & 0x3C); 
        EPC[MC_CTXT]    = PC[MC_CTXT] - 4; 
        PC[MC_CTXT]     = R3000_GENERAL_EXCEPTION_VECTOR;
        IR[MC_CTXT]     = IR_NOP; 
      break;
      
      case X_ADEL :
        CAUSE[MC_CTXT]  = interrupt_signal | (X_ADEL  << 2);
        STATUS[MC_CTXT] = (STATUS[MC_CTXT] & ~0x3F) | ((STATUS[MC_CTXT] << 2) & 0x3C); 
        EPC[MC_CTXT]    = PC[MC_CTXT] - 4; 
        PC[MC_CTXT]     = R3000_GENERAL_EXCEPTION_VECTOR;
        IR[MC_CTXT]     = IR_NOP; 
        BADVADDR[MC_CTXT] = exception_adress;
      break;

      case X_ADES :
        CAUSE[MC_CTXT]  = interrupt_signal | (X_ADES  << 2);
        STATUS[MC_CTXT] = (STATUS[MC_CTXT] & ~0x3F) | ((STATUS[MC_CTXT] << 2) & 0x3C); 
        EPC[MC_CTXT]    = PC[MC_CTXT] - 4; 
        PC[MC_CTXT]     = R3000_GENERAL_EXCEPTION_VECTOR;
        IR[MC_CTXT]     = IR_NOP; 
        BADVADDR[MC_CTXT] = exception_adress;
      break;
      
      case X_RI :
        CAUSE[MC_CTXT]  = interrupt_signal | (X_RI  << 2);
        STATUS[MC_CTXT] = (STATUS[MC_CTXT] & ~0x3F) | ((STATUS[MC_CTXT] << 2) & 0x3C); 
        EPC[MC_CTXT]    = PC[MC_CTXT] - 4; 
        PC[MC_CTXT]     = R3000_GENERAL_EXCEPTION_VECTOR;
        IR[MC_CTXT]     = IR_NOP; 
      break;

      case X_CPU :
        CAUSE[MC_CTXT]  = interrupt_signal | (X_CPU  << 2);
        STATUS[MC_CTXT] = (STATUS[MC_CTXT] & ~0x3F) | ((STATUS[MC_CTXT] << 2) & 0x3C); 
        EPC[MC_CTXT]    = PC[MC_CTXT] - 4; 
        PC[MC_CTXT]     = R3000_GENERAL_EXCEPTION_VECTOR;
        IR[MC_CTXT]     = IR_NOP; 
      break;

      case X_IBE :
        CAUSE[MC_CTXT]  = interrupt_signal | (X_IBE  << 2);
        STATUS[MC_CTXT] = (STATUS[MC_CTXT] & ~0x3F) | ((STATUS[MC_CTXT] << 2) & 0x3C); 
        EPC[MC_CTXT]    = PC[MC_CTXT] - 4; 
        PC[MC_CTXT]     = R3000_GENERAL_EXCEPTION_VECTOR;
        IR[MC_CTXT]     = IR_NOP; 
      break;

      case X_DBE :
        CAUSE[MC_CTXT]  = interrupt_signal | (X_DBE  << 2);
        STATUS[MC_CTXT] = (STATUS[MC_CTXT] & ~0x3F) | ((STATUS[MC_CTXT] << 2) & 0x3C); 
        EPC[MC_CTXT]    = PC[MC_CTXT] - 4; 
        PC[MC_CTXT]     = R3000_GENERAL_EXCEPTION_VECTOR;
        IR[MC_CTXT]     = IR_NOP; 
      break;

    } // end switch
  } // end if exception

  else  if ((STATUS[MC_CTXT] & 0x0000FC00 & interrupt_signal) && 
            (STATUS[MC_CTXT] & 0x00000001)) {

      /* Interrupt handling
       * 
       * On regarde si une interruption non masquée a été levée 
       * Dans ce cas, on sauvegarde l'adresse de retour dans EPC,
       * on branche au gestionnaire d'exceptions, on indique la cause
       * dans le registre de cause, et on force un NOP dans le registre instruction.
       * Il faut un traitement particulier dans le cas des deux instructions
       * RFE et MTC0 qui modifient les registres du coprocesseur 0.
       * Ces écritures n'ont pas été effectuées dans le grand switch
       * d'exécution des instructions, mais la variable status_modified 
       * est différente de zéro. Ces écritures sont effectuées ici 
       * pour un meilleur controle des écritures dans les registres de CP0.
       * Dans le cas MTCO suivi d'une interruption, il faut copier  
       * le registre GPR(rt) dans le registre cible        
       * puis modifier normalement les registres STATUS, EPC et CAUSE.
       * Dans le cas RFE suivi d'une interruption, il faut simplement 
       * forcer à zéro les deux bits de poids faible du registre STATUS.
       * puis modifier normalement les registres EPC et CAUSE.
       */

      if (status_modified == OP_RFE) {
          STATUS[MC_CTXT] = STATUS[MC_CTXT] & 0xFFFFFFFC;
          CAUSE[MC_CTXT]  = interrupt_signal;  
          EPC[MC_CTXT]    = PC[MC_CTXT];
          PC[MC_CTXT]     = R3000_GENERAL_EXCEPTION_VECTOR;
          IR[MC_CTXT]     = IR_NOP; 
      } // end if OP_RFE

      else if (status_modified == OP_MTC0) {
          CAUSE[MC_CTXT]  = interrupt_signal;  
          EPC[MC_CTXT]    = PC[MC_CTXT];
          PC[MC_CTXT]     = R3000_GENERAL_EXCEPTION_VECTOR;
          IR[MC_CTXT]     = IR_NOP; 
          switch (ins_rd) {
          	case 0: // MTC0 -> IDENT
            	IDENT[MC_CTXT]  = GPR[ins_rt][MC_CTXT]; 
                STATUS[MC_CTXT] = (STATUS[MC_CTXT] & ~0x3F) | ((STATUS[MC_CTXT] << 2) & 0x3C);
            	break;

            	case 12: // MTC0 -> STATUS
                STATUS[MC_CTXT] = (GPR[ins_rt][MC_CTXT] & ~0x3F) | ((GPR[ins_rt][MC_CTXT] << 2) & 0x3C);
           	break;

           	case 13: // MTC0 -> CAUSE
                STATUS[MC_CTXT] = (STATUS[MC_CTXT] & ~0x3F) | ((STATUS[MC_CTXT] << 2) & 0x3C);
           	break;

           	case 14: // MTC0 -> EPC
                STATUS[MC_CTXT] = (STATUS[MC_CTXT] & ~0x3F) | ((STATUS[MC_CTXT] << 2) & 0x3C);
           	break;

           	default:
           	Unimplemented(PC[MC_CTXT], "mtc0");
      		}
	} // end if MTC0

        else {
		switch(ins_opcode) { // pas de depart en interruption en cas de branchement
		case OP_JAL:
		case OP_J :
		case OP_JALR :
		case OP_JR :
		case OP_BEQ :
		case OP_BGEZAL :
		case OP_BGEZ :
		case OP_BGTZ :
		case OP_BLEZ :
		case OP_BLTZ :
		case OP_BNE :
          		CAUSE[MC_CTXT] = (CAUSE[MC_CTXT] & 0x0000003C) | interrupt_signal;
          		PC[MC_CTXT]    = next_pc;
          		IR[MC_CTXT]    = (int)ICACHE.INS.read();
			break;
		default :   // interrupt
          		STATUS[MC_CTXT] = (STATUS[MC_CTXT] & ~0x3F) | ((STATUS[MC_CTXT] << 2) & 0x3C);
          		CAUSE[MC_CTXT]  = interrupt_signal;  
          		EPC[MC_CTXT]    = PC[MC_CTXT];
          		PC[MC_CTXT]     = R3000_GENERAL_EXCEPTION_VECTOR;
          		IR[MC_CTXT]     = IR_NOP;
			break;
		} // end switch ins_opcode 
	}  // end else status_modified
  } // end if interrupt

  else {
      /* No interrupt, no exception
       * 
       * si une instruction MTC0 a cherché à écrire dans un registre    
       * du coprocesseur 0, ou si une instruction RFE a cherché
       * à modifier la valeur du registre STATUS, ces écritures 
       * n'ont pas été effectuées dans le grand switch
       * d'exécution des instructions, mais la variable status_modified 
       * est différente de zéro. Ces écritures sont effectuées ici 
       * pour un meilleur controle des écritures dans les registres CP0.
       * Le registre CAUSE est mis à jour systematiquement pour 
       * échantillonner les interruptions.
       */
       if (status_modified == 0) {
          CAUSE[MC_CTXT] = (CAUSE[MC_CTXT] & 0x0000003C) | interrupt_signal;
          PC[MC_CTXT]    = next_pc;
          IR[MC_CTXT]    = (int)ICACHE.INS.read();
       } // end if status_modified = 0
       
	else if (status_modified == OP_RFE) {
          STATUS[MC_CTXT] = (STATUS[MC_CTXT] & 0xFFFFFFF0) | ((STATUS[MC_CTXT] >> 2) & 0xF);
          CAUSE[MC_CTXT]  = (CAUSE[MC_CTXT] & 0x0000003C) | interrupt_signal;
          PC[MC_CTXT]     = next_pc;
          IR[MC_CTXT]     = (int)ICACHE.INS.read();
       } // end if OP_RFE
          
	else if (status_modified == OP_MTC0) {  
          PC[MC_CTXT]    = next_pc;
          IR[MC_CTXT]    = (int)ICACHE.INS.read();
          switch (ins_rd) {
            case 0: // MTC0 -> IDENT
               CAUSE[MC_CTXT] = (CAUSE[MC_CTXT] & 0x0000003C) | interrupt_signal;
               IDENT[MC_CTXT] = GPR[ins_rt][MC_CTXT]; 
               break;
	    case 12: // MTC0 -> STATUS
               CAUSE[MC_CTXT]  = (CAUSE[MC_CTXT] & 0x0000003C) | interrupt_signal;
               STATUS[MC_CTXT] = GPR[ins_rt][MC_CTXT]; 
               break;
            case 13: // MTC0 -> CAUSE
               CAUSE[MC_CTXT] = GPR[ins_rt][MC_CTXT] | interrupt_signal;
               break;
            case 14: // MTC0 -> EPC
               CAUSE[MC_CTXT] = (CAUSE[MC_CTXT] & 0x0000003C) | interrupt_signal;
               EPC[MC_CTXT]   = GPR[ins_rt][MC_CTXT]; 
               break;
            default:
               Unimplemented(PC[MC_CTXT], "mtc0");
	    } // end switch   
       } // end if OP_MTC0

  } // end else no interrupt & no exception
 
};  // end transition()

////////////////////////////////////////////////////////////
//	genMoore()
////////////////////////////////////////////////////////////

void genMoore()
{
  char ins_opcode;
  int  ins_rs, ins_rt, ins_rd, ins_misc;
  int  adr;
//  int index;
  
//puts("Moore");
//print_regs();
 IDecode(IR[MC_CTXT], &ins_opcode, &ins_rs, &ins_rt, &ins_rd, &ins_misc);

   // switch on read/write memory instructions

  switch (ins_opcode) {

      case OP_LB:
      adr   = GPR[ins_rs][MC_CTXT] + ins_misc;
//      index = (adr >> (32 - MSB_NUMBER)) & MSB_MASK;
      DCACHE.REQ = true;
      DCACHE.ADR = (sc_uint<32>)adr;
      DCACHE.TYPE = (sc_uint<3>)DTYPE_RW;
      DCACHE.WDATA = (sc_uint<32>)0x0;
      break;

      case OP_LBU:
      adr   = GPR[ins_rs][MC_CTXT] + ins_misc;
//      index = (adr >> (32 - MSB_NUMBER)) & MSB_MASK;
      DCACHE.REQ = true;
      DCACHE.ADR = (sc_uint<32>)adr;
      DCACHE.TYPE = (sc_uint<3>)DTYPE_RW;
      DCACHE.WDATA = (sc_uint<32>)0x0;
      break;

      case OP_LH:
      adr   = GPR[ins_rs][MC_CTXT] + ins_misc;
//      index = (adr >> (32 - MSB_NUMBER)) & MSB_MASK;
      DCACHE.REQ = true;
      DCACHE.ADR = (sc_uint<32>)adr;
      DCACHE.TYPE = (sc_uint<3>)DTYPE_RW;
      DCACHE.WDATA = (sc_uint<32>)0x0;
      break;

      case OP_LHU:
      adr   = GPR[ins_rs][MC_CTXT] + ins_misc;
//      index = (adr >> (32 - MSB_NUMBER)) & MSB_MASK;
      DCACHE.REQ = true;
      DCACHE.ADR = (sc_uint<32>)adr;
      DCACHE.TYPE = (sc_uint<3>)DTYPE_RW;
      DCACHE.WDATA = (sc_uint<32>)0x0;
      break;

      case OP_LW:
      adr   = GPR[ins_rs][MC_CTXT] + ins_misc;
//      index = (adr >> (32 - MSB_NUMBER)) & MSB_MASK;
      DCACHE.REQ = true;
      DCACHE.ADR = (sc_uint<32>)adr;
	// cache line invalidate
      if (ins_rt == 0) {
            DCACHE.TYPE = (sc_uint<3>)DTYPE_RZ;
      } else {
	      DCACHE.TYPE = (sc_uint<3>)DTYPE_RW;
      }
      DCACHE.WDATA = (sc_uint<32>)0x0;
      break;
      case OP_SB:
      adr   = GPR[ins_rs][MC_CTXT] + ins_misc;
//      index = (adr >> (32 - MSB_NUMBER)) & MSB_MASK;
      DCACHE.REQ = true;
      DCACHE.ADR = (sc_uint<32>)adr;
      DCACHE.TYPE = (sc_uint<3>)DTYPE_WB;
      DCACHE.WDATA = (sc_uint<32>)((GPR[ins_rt][MC_CTXT] & 0X000000FF))      |
                                  ((GPR[ins_rt][MC_CTXT] & 0X000000FF) << 8) |
                                  ((GPR[ins_rt][MC_CTXT] & 0X000000FF) << 16)|
                                  ((GPR[ins_rt][MC_CTXT] & 0X000000FF) << 24);
      break;

      case OP_SH:
      adr   = GPR[ins_rs][MC_CTXT] + ins_misc;
//      index = (adr >> (32 - MSB_NUMBER)) & MSB_MASK;
      DCACHE.REQ = true;
      DCACHE.ADR = (sc_uint<32>)adr;
      DCACHE.TYPE = (sc_uint<3>)DTYPE_WH;
      DCACHE.WDATA = (sc_uint<32>)((GPR[ins_rt][MC_CTXT] & 0X0000FFFF))      |
                                  ((GPR[ins_rt][MC_CTXT] & 0X0000FFFF) << 16);
      break;

      case OP_SW:
      adr   = GPR[ins_rs][MC_CTXT] + ins_misc;
//      index = (adr >> (32 - MSB_NUMBER)) & MSB_MASK;
      DCACHE.REQ = true;
      DCACHE.ADR = (sc_uint<32>)adr;
      DCACHE.TYPE = (sc_uint<3>)DTYPE_WW;
      DCACHE.WDATA = (sc_uint<32>)GPR[ins_rt][MC_CTXT];
      break;

      case OP_SC:
      adr   = GPR[ins_rs][MC_CTXT] + ins_misc;
//      index = (adr >> (32 - MSB_NUMBER)) & MSB_MASK;
      DCACHE.REQ = true;
      DCACHE.ADR = (sc_uint<32>)adr;
      DCACHE.TYPE = (sc_uint<3>)DTYPE_WW;
      DCACHE.WDATA = (sc_uint<32>)GPR[ins_rt][MC_CTXT];
      break;

      default :
      DCACHE.REQ = false;
      DCACHE.ADR   = 0;
      DCACHE.TYPE  = 0;
      DCACHE.WDATA = 0;
      break;
  
  }// end switch
		

   ICACHE.REQ = true;  // instruction request always valid
   DCACHE.UNC = false; // not used -> set to false
   ICACHE.ADR = (sc_uint<32>)PC[MC_CTXT];

}; // end genMoore()

}; // end struct SOCLIB_MULTI_MIPS

#endif
