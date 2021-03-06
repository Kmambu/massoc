#include <systemc.h>
#include "cache/soclib_xcache_interface.h"

#define ACCESS_READ_W 0
#define ACCESS_READ_H 1
#define ACCESS_READ_HU 11
#define ACCESS_READ_B 2
#define ACCESS_READ_BU 22
#define ACCESS_STORE_W 3
#define ACCESS_STORE_H 4
#define ACCESS_STORE_B 5
#define ACCESS_INVALIDATE 6
#define ACCESS_NONE 7

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

///////////////////////////////////////////////////////
//	Processor structure definition
///////////////////////////////////////////////////////
 
struct MIPSR3000 {
				
	sc_signal<int> IDENT;	// Processor ID

	int IT_0;
	int IT_1;
	int IT_2;
	int IT_3;
	int IT_4;
	int IT_5;

	int GPR[32];	// General Purpose Registers
   	int PC;		// Program Counter 
   	int HI;		// HI and LO part of the multiplication results 
   	int LO;		// or result of Division and Modulo 
   	int IR;		// Instruction Register   
  
   	int STATUS;	// Status register 
   	int CAUSE;		// Cause of the last interupt/exception 
   	int EPC;		// PC where the exception occured 
   	int BADVADDR;	// Addr when an Address error occurs 

	int IRDATA;
	int RDATA;
	int WDATA;
	int INTERRUPT_SIGNAL;
	int CPU_ACCESS;
	int CPU_OPMEM;
	int CPU_ADDRESS;
	int CPU_REG;

	bool delayed_slot;
	int delayed_slot_real_pc;

///////////////////////////////////////////////////////////
//	constructor
///////////////////////////////////////////////////////////

MIPSR3000()
{
	delayed_slot=false;
};  // end constructor

void set_ident(int ident)
{
	IDENT = ident;
};  // end constructor

void print_regs()
{
	printf("PC=%8.8x IR=%8.8x\n",PC,IR);

	for (int i=0;i<4;i++)
	{
		for (int j=0;j<8;j++)
			printf("R[%2.2d]=%8.8x ",i*8+j,(unsigned int)GPR[i*8+j]);
		puts("");
	}

}

///////////////////////////////////////////////////////
//	Transition()
///////////////////////////////////////////////////////

void reset()
{
        for (int j = 0; j < 32; j++) 
		GPR[j] = 0;
        STATUS   = 0;
        HI       = 0;
        LO       = 0;
        IR       = 0;
        CAUSE    = 0;
        EPC      = 0;
        BADVADDR = 0;
        PC       = R3000_RESET_VECTOR;

	INTERRUPT_SIGNAL=0;
        IRDATA=0;
        RDATA=0;
        WDATA=0;
        INTERRUPT_SIGNAL=0;
        CPU_ACCESS=DTYPE_RW;
        CPU_OPMEM=ACCESS_NONE;
        CPU_ADDRESS=0;
        CPU_REG=0;
        CPU_REG=0;
}

void step()
{
printf("PC=%x\n",PC);
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

  int delayed=false;

//print_regs();
  switch (CPU_OPMEM)
  {
	case ACCESS_READ_W:
//printf("ACESS_READ_W %x %d %x\n",GPR[CPU_REG], CPU_REG, RDATA);
		GPR[CPU_REG]=RDATA;
		break;
	case ACCESS_READ_B:
		{
                int data = RDATA >> (8 * (CPU_ADDRESS & 0x3));
                GPR[CPU_REG] = ((data & 0x00000080)
                             ? (data | 0xFFFFFF00)
                             : (data & 0x000000FF));
		}
		break;
	case ACCESS_READ_BU:
		{
                int data = RDATA >> (8 * (CPU_ADDRESS & 0x3));
                GPR[CPU_REG] = data & 0x000000FF;
		}
		break;
	case ACCESS_READ_H:
		{
                int data = RDATA >> (8 * (CPU_ADDRESS & 0x2));
                GPR[CPU_REG] = ((data & 0x00008000)
                             ? (data | 0xFFFF0000)
                             : (data & 0x0000FFFF));
		}
		break;
	case ACCESS_READ_HU:
		{
                int data = RDATA >> (8 * (CPU_ADDRESS & 0x2));
                GPR[CPU_REG] = data & 0x0000FFFF;
		}
		break;
	case ACCESS_NONE:
		break;
	default:
		break;
  }

  CPU_ACCESS=DTYPE_RW;
  CPU_OPMEM=ACCESS_NONE;
  CPU_ADDRESS=0;
  CPU_REG=0;

//////////////////////////////////////
//	INSTRUCTION DECODE
//////////////////////////////////////

  IDecode(IR, &ins_opcode, &ins_rs, &ins_rt, &ins_rd, &ins_misc);
  
  interrupt_signal = INTERRUPT_SIGNAL;

  switch (ins_opcode) {
    
    case OP_ADD:
    {
      int s = GPR[ins_rs] + GPR[ins_rt];
      if (!((GPR[ins_rs] ^ GPR[ins_rt]) & 0x80000000)
           && ((GPR[ins_rs] ^ s) & 0x80000000)) {
       		exception_signal = X_OVF;
         	break;
      }
      // register update only if no exception occurs
      if(ins_rd != 0) { GPR[ins_rd] = s; }
      next_pc = PC + 4;
      break;
    }
    
    case OP_ADDI:
    {
      int s = GPR[ins_rs] + ins_misc;
      if (!((GPR[ins_rs] ^ ins_misc) & 0x80000000)
           && ((GPR[ins_rs] ^ s) & 0x80000000)) {
            	exception_signal = X_OVF;
            	break;
      }
      // register update only if no exception occurs
      if(ins_rt != 0) { GPR[ins_rt] = s; }
      next_pc = PC + 4;
      break;
    }

    case OP_ADDIU:
      if(ins_rt != 0) { GPR[ins_rt] = GPR[ins_rs] + ins_misc; }
      next_pc = PC + 4;
      break;

    case OP_ADDU:
      if(ins_rd != 0) { GPR[ins_rd] = GPR[ins_rs] + GPR[ins_rt]; }
      next_pc = PC + 4;
      break;

    case OP_AND:
      if(ins_rd != 0) { GPR[ins_rd] = GPR[ins_rs] & GPR[ins_rt]; }
      next_pc = PC + 4;
      break;

    case OP_ANDI:
      if(ins_rt != 0) { GPR[ins_rt] = GPR[ins_rs] & ins_misc & 0xFFFF; }
      next_pc = PC + 4;
      break;

    case OP_BC0F:
      Unimplemented(PC, "bc0f");
      break;

    case OP_BC1F:
      Unimplemented(PC, "bc1f");
      break;

    case OP_BC2F:
      Unimplemented(PC, "bc2f");
      break;

    case OP_BC3F:
      Unimplemented(PC, "bc3f");
      break;

    case OP_BC0T:
      Unimplemented(PC, "bc0t");
      break;

    case OP_BC1T:
      Unimplemented(PC, "bc1t");
      break;

    case OP_BC2T:
      Unimplemented(PC, "bc2t");
      break;

    case OP_BC3T:
      Unimplemented(PC, "bc3t");
      break;

    case OP_BEQ:        
      if (GPR[ins_rs] == GPR[ins_rt]) 
		delayed=true;
      next_pc = PC + 4;
      delayed_slot_real_pc = PC + (ins_misc << 2)+4;
      break;

    case OP_BGEZ:
      if ((GPR[ins_rs] & 0x80000000) == 0x0)
		delayed=true;
      next_pc = PC + 4;
      delayed_slot_real_pc = PC + (ins_misc << 2)+4;
      break;

    case OP_BGEZAL:
      if ((GPR[ins_rs] & 0x80000000) == 0x0) { 
		delayed=true;
          GPR[31] = PC + 8;
	}
          next_pc = PC + 4;
      delayed_slot_real_pc = PC + (ins_misc << 2)+4;
      break;

    case OP_BGTZ:
      if (((GPR[ins_rs] & 0x80000000) == 0x0) && 
           (GPR[ins_rs] != 0))
		delayed=true;
          next_pc = PC + 4;
      delayed_slot_real_pc = PC + (ins_misc << 2)+4;
      break;

    case OP_BLEZ:
      if (((GPR[ins_rs] & 0x80000000) == 0x80000000) || 
            (GPR[ins_rs] == 0))
		delayed=true;
          next_pc = PC + 4;
      delayed_slot_real_pc = PC + (ins_misc << 2)+4;
      break;

    case OP_BLTZ:
      if ((GPR[ins_rs] & 0x80000000) == 0x80000000)
		delayed=true;
          next_pc = PC + 4;
      delayed_slot_real_pc = PC + (ins_misc << 2)+4;
      break;

    case OP_BLTZAL:
      if ((GPR[ins_rs] & 0x80000000) == 0x80000000) {
		delayed=true;
          GPR[31] = PC + 8;
      }
          next_pc = PC + 4;
      delayed_slot_real_pc = PC + (ins_misc << 2)+4;
      break;
         
    case OP_BNE:
      if (GPR[ins_rs] != GPR[ins_rt])
		delayed=true;
          next_pc = PC + 4;
      delayed_slot_real_pc = PC + (ins_misc << 2)+4;
      break;

    case OP_BREAK:
      next_pc = PC - 4;
      exception_signal = X_BP;
      break;

    case OP_CFC0:
      Unimplemented(PC, "cfc0");
      break;

    case OP_CFC1:
      Unimplemented(PC, "cfc1");
      break;

    case OP_CFC2:
      Unimplemented(PC, "cfc2");
      break;

    case OP_CFC3:
      Unimplemented(PC, "cfc3");
      break;

    case OP_COP0:
      Unimplemented(PC, "cop0");
      break;

    case OP_COP1:
      Unimplemented(PC, "cop1");
      break;

    case OP_COP2:
      Unimplemented(PC, "cop2");
      break;

    case OP_COP3:
      Unimplemented(PC, "cop3");
      break;

    case OP_CTC0:
      Unimplemented(PC, "ctc0");
      break;

    case OP_CTC1:
      Unimplemented(PC, "ctc1");
      break;

    case OP_CTC2:
      Unimplemented(PC, "ctc2");
      break;

    case OP_CTC3:
      Unimplemented(PC, "ctc3");
      break;
         
    case OP_DIV:
      if (GPR[ins_rt]) {
            LO = GPR[ins_rs] / GPR[ins_rt];
            HI = GPR[ins_rs] % GPR[ins_rt];
      } else {
            LO = random();
            HI = random();
      }
      next_pc = PC + 4;
      break;

    case OP_DIVU:
      {
      unsigned int rs, rt;
      rs = (unsigned int)GPR[ins_rs];
      rt = (unsigned int)GPR[ins_rt];
      if (rt) {
            LO = (int) ((unsigned int)(rs / rt));
            HI = (int) ((unsigned int)(rs % rt));
      } else {
            LO = random();
            HI = random();
      }
      next_pc = PC + 4;
      break;
      }

    case OP_J:
      // ins_misc  is multiplied by 4 in the Idecode function
	next_pc= PC+4;
      delayed_slot_real_pc = (PC & 0XF0000000) | ins_misc;
      delayed=true;
      break;

    case OP_JAL:
      // ins_misc  is multiplied by 4 in the Idecode function
	next_pc= PC+4;
      delayed_slot_real_pc = (PC & 0XF0000000) | ins_misc;
      GPR[31] = PC + 8;
	delayed=true;
      break;

    case OP_JALR:
      if (ins_rd != 0) { GPR[ins_rd] = PC + 4 + 4; }
	next_pc= PC+4;
      delayed_slot_real_pc = GPR[ins_rs];
      delayed=true;
      break;

    case OP_JR:
	next_pc= PC+4;
      delayed_slot_real_pc = GPR[ins_rs];
      delayed=true;
      break;

    case OP_LB:
      {
      // address computation is only for exception 
      int adr;
      adr = GPR[ins_rs] + ins_misc;
      if ((STATUS & 0x2) && (adr & 0x80000000)) {
       		exception_signal = X_ADEL;
       		exception_adress = adr;
       		break;
      }
	CPU_ACCESS=DTYPE_RW;
	CPU_OPMEM=ACCESS_READ_B;
	CPU_ADDRESS=adr;
	CPU_REG=ins_rt;

      next_pc = PC + 4;
      break;
      }

    case OP_LBU:
      {
      // address computation is only for exception 
      int adr;
      adr = GPR[ins_rs] + ins_misc;
      if ((STATUS & 0x2) && (adr & 0x80000000)) {
         	exception_signal = X_ADEL;
         	exception_adress = adr;
         	break;
      }
	CPU_ACCESS=DTYPE_RW;
	CPU_OPMEM=ACCESS_READ_BU;
	CPU_ADDRESS=adr;
	CPU_REG=ins_rt;

      next_pc = PC + 4;
      break;
      }

    case OP_LH:
      {
      // address computation is only for exception 
      int adr;
      adr = GPR[ins_rs] + ins_misc;
      if ((adr & 0x1) || ((STATUS & 0x2) && (adr & 0x80000000))) {
         	exception_signal = X_ADEL;
         	exception_adress = adr;
         	break;
      }
	CPU_ACCESS=DTYPE_RW;
	CPU_OPMEM=ACCESS_READ_H;
	CPU_ADDRESS=adr;
	CPU_REG=ins_rt;

      next_pc = PC + 4;
      break;
      }

      case OP_LHU:
      {
      // address computation is only for exception 
      int adr;
      adr = GPR[ins_rs] + ins_misc;
      if ((adr & 0x1) || ((STATUS & 0x2) && (adr & 0x80000000))) {
         	exception_signal = X_ADEL;
         	exception_adress = adr;
         	break;
      }
	CPU_ACCESS=DTYPE_RW;
	CPU_OPMEM=ACCESS_READ_HU;
	CPU_ADDRESS=adr;
	CPU_REG=ins_rt;

      next_pc = PC + 4;
      break;
      }

    case OP_LUI:
      if (ins_rt != 0){ GPR[ins_rt] = ins_misc << 16; } 
      next_pc = PC + 4;
      break;

    case OP_LW:
      {
      // address computation is only for exception 
      int adr = GPR[ins_rs] + ins_misc;
      if ((adr & 0x3) || ((STATUS & 0x2) && (adr & 0x80000000))) {
         	exception_signal = X_ADEL;
         	exception_adress = adr;
         	break;
      }
      if (ins_rt)
	{
		CPU_ACCESS=DTYPE_RW;
		CPU_OPMEM=ACCESS_READ_W;
		CPU_ADDRESS=adr;
		CPU_REG=ins_rt;
	}
	else
	{
		CPU_ACCESS=DTYPE_RZ;
		CPU_ADDRESS=adr;
	}
      next_pc = PC + 4;
      break;
      }

    case OP_LWC0:
      Unimplemented(PC, "lwc0");
      break;

    case OP_LWC1:
      Unimplemented(PC, "lwc1");
      break;

    case OP_LWC2:
      Unimplemented(PC, "lwc2");
      break;

    case OP_LWC3:
      Unimplemented(PC, "lwc3");
      break;

    case OP_LWL:
      Unimplemented(PC, "lwl");
      break;

    case OP_LWR:
      Unimplemented(PC, "lwr");
      break;
     
    case OP_MFC0:

     if (STATUS & 0x2) { // User mode 
           exception_signal = X_CPU;
           break;
      }

      next_pc = PC + 4;
      if (ins_rt != 0) {
          switch (ins_rd) {
            case 0:
            GPR[ins_rt] = IDENT;
            break;
            case 8:
            GPR[ins_rt] = BADVADDR;
            break;
            case 12:
            GPR[ins_rt] = STATUS;
            break;
            case 13:
            GPR[ins_rt] = CAUSE;
            break;
            case 14:
            GPR[ins_rt] = EPC;
            break;
            default:
            Unimplemented(PC, "mfc0");
          } // end switch
      } // end if
      break;

    case OP_MFC1:
      Unimplemented(PC, "mfc1");
      break;

    case OP_MFC2:
      Unimplemented(PC, "mfc2");
      break;

    case OP_MFC3:
      Unimplemented(PC, "mfc3");
      break;

    case OP_MFHI:
      next_pc = PC + 4;
      if (ins_rd != 0) { GPR[ins_rd] =  HI; }
      break;

    case OP_MFLO:
      next_pc = PC + 4;
      if (ins_rd != 0) { GPR[ins_rd] =  LO; }
      break;
         
    case OP_MTC0:
      if (STATUS & 0x2) {     /* User mode */
        	exception_signal = X_CPU;
        	break;
      }
      next_pc = PC + 4;
      status_modified = OP_MTC0;
      break;

    case OP_MTC1:
      Unimplemented(PC, "mtc1");
      break;

    case OP_MTC2:
      Unimplemented(PC, "mtc2");
      break;

    case OP_MTC3:
      Unimplemented(PC, "mtc3");
      break;

    case OP_MTHI:
      next_pc = PC + 4;
      HI =  GPR[ins_rs];
      break;

    case OP_MTLO:
      next_pc = PC + 4;
      LO =  GPR[ins_rs];
      break;
         
    case OP_MULT:
      { 
      long long unsigned int a,b,c;
      a = GPR[ins_rs];
      b = GPR[ins_rt];
      c = a*b;
      LO = (unsigned int)c;
      c >>= 32;
      HI = (unsigned int)c;
      next_pc = PC + 4;
      break;
      }

    case OP_MULTU:
      {
      int xlo = GPR[ins_rs];
      int ylo = GPR[ins_rt];
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
      HI = rhi;
      LO = rlo;
      next_pc = PC + 4;
      break;
      }

    case OP_NOR:
      next_pc = PC + 4;
      if (ins_rd != 0) { GPR[ins_rd] = ~(GPR[ins_rs] | GPR[ins_rt]); }
      break;

    case OP_OR:
      next_pc = PC + 4;
      if (ins_rd != 0) { GPR[ins_rd] = GPR[ins_rs] | GPR[ins_rt]; }
      break;

    case OP_ORI:
      next_pc = PC + 4;
      if (ins_rt != 0) { GPR[ins_rt] = GPR[ins_rs] | (ins_misc & 0xFFFF); }
      break;
    
    case OP_RFE:
      if (STATUS & 0x2) {      // User mode 
            exception_signal = X_CPU;
            break;
      }
      status_modified = OP_RFE;
      next_pc = PC + 4;
      break;

    case OP_SB:
      {
      int adr = GPR[ins_rs] + ins_misc;
      if ((STATUS & 0x2) && (adr & 0x80000000)) {
           	exception_signal = X_ADES;
          	exception_adress = adr;
          	break;
      }
	CPU_ACCESS=DTYPE_WB;
        CPU_OPMEM=ACCESS_STORE_B;
	CPU_ADDRESS=adr;
      WDATA = ((GPR[ins_rt] & 0X000000FF))      |
              ((GPR[ins_rt] & 0X000000FF) << 8) |
              ((GPR[ins_rt] & 0X000000FF) << 16)|
              ((GPR[ins_rt] & 0X000000FF) << 24);
      next_pc = PC + 4;
      break;
      }
      
    case OP_SH:
      {
      int adr = GPR[ins_rs] + ins_misc;
      if ((STATUS & 0x2) && (adr & 0x80000000)) {
           	exception_signal = X_ADES;
          	exception_adress = adr;
          	break;
      }
	CPU_ACCESS=DTYPE_WH;
     	CPU_OPMEM=ACCESS_STORE_H;
	CPU_ADDRESS=adr;
      WDATA = ((GPR[ins_rt] & 0X0000FFFF))      |
              ((GPR[ins_rt] & 0X0000FFFF) << 16);
      next_pc = PC + 4;
      break;
      }

    case OP_SLL:
      if (ins_rd != 0) { GPR[ins_rd] = GPR[ins_rt] << ins_misc;} 
      next_pc = PC + 4;
      break;

    case OP_SLLV:
      if (ins_rd != 0) { GPR[ins_rd] = GPR[ins_rt] << (GPR[ins_rs] & 0x1F);}
      next_pc = PC + 4;
      break;

    case OP_SLT:
      if (ins_rd != 0) { GPR[ins_rd] = (GPR[ins_rs] < GPR[ins_rt] ? 1 : 0);}
      next_pc = PC + 4;
      break;

    case OP_SLTI:
      if (ins_rt != 0) { GPR[ins_rt] = (GPR[ins_rs] < ins_misc ? 1 : 0);}
      next_pc = PC + 4;
      break;

    case OP_SLTIU:
      if (ins_rt != 0) { GPR[ins_rt] = 
	      ((unsigned int)GPR[ins_rs] < (unsigned int)ins_misc ? 1 : 0);}
      next_pc = PC + 4;
      break;

    case OP_SLTU:
      if (ins_rd != 0) { GPR[ins_rd] = 
	      ((unsigned int)GPR[ins_rs] < (unsigned int)GPR[ins_rt] ? 1 : 0);}
      next_pc = PC + 4;
      break;

    case OP_SRA:
      if (ins_rd != 0) { GPR[ins_rd] = GPR[ins_rt] >> ins_misc; }
      next_pc = PC + 4;
      break;

    case OP_SRAV:
      if (ins_rd != 0) { GPR[ins_rd] = GPR[ins_rt] >> (GPR[ins_rs] & 0x1F);}
      next_pc = PC + 4;
      break;

    case OP_SRL:
      if (ins_rd != 0) { GPR[ins_rd] = ( ((unsigned int) (GPR[ins_rt])) >> ins_misc) 
                                                  & (0x7FFFFFFF >> (ins_misc - 1));}
      next_pc = PC + 4;
      break;

    case OP_SRLV:
      if (ins_rd != 0) { GPR[ins_rd] = (((unsigned int) (GPR[ins_rt])) >> (GPR[ins_rs] & 0x1F))
                                                  & (0x7FFFFFFF >> ((GPR[ins_rs] & 0x1F) - 1));}
      next_pc = PC + 4;
      break;

    case OP_SUB:
      {
      int s = GPR[ins_rs] - GPR[ins_rt];
      if (!((GPR[ins_rs] ^ GPR[ins_rt]) & 0x80000000)
           && ((GPR[ins_rs] ^ s) & 0x80000000)) {
        	exception_signal = X_OVF;
         	break;
      }
      // register update only if no exception occurs 
      if(ins_rd != 0) { GPR[ins_rd] = s; }
      next_pc = PC + 4;
      break;
      }
  
    case OP_SUBU:
      if(ins_rd != 0) { GPR[ins_rd] = GPR[ins_rs] - GPR[ins_rt]; }
      next_pc = PC + 4;
      break;

    case OP_SW:
      {
      int adr = GPR[ins_rs] + ins_misc;
      if ((STATUS & 0x2) && (adr & 0x80000000)) {
           	exception_signal = X_ADES;  
          	exception_adress = adr;
          	break;
      }
      CPU_ACCESS=DTYPE_WW;
      CPU_OPMEM=ACCESS_STORE_W;
      CPU_ADDRESS=adr;
      WDATA = GPR[ins_rt];
      next_pc = PC + 4;
      break;
      }
           
      case OP_SC:
      {
      int adr = GPR[ins_rs] + ins_misc;
      if ((STATUS & 0x2) && (adr & 0x80000000)) {
           	exception_signal = X_ADES;  
          	exception_adress = adr;
          	break;
      }
      WDATA = GPR[ins_rt];
      next_pc = PC + 4;
      break;
      }
          
    case OP_SWC0:
      Unimplemented(PC, "swc0");
      break;

    case OP_SWC1:
      Unimplemented(PC, "swc1");
      break;

    case OP_SWC2:
      Unimplemented(PC, "swc2");
      break;

    case OP_SWC3:
      Unimplemented(PC, "swc3");
      break;

    case OP_SWL:
      Unimplemented(PC, "swl");
      break;

    case OP_SWR:
      Unimplemented(PC, "swr");
      break;

    case OP_SYSCALL:
      next_pc = PC+4;
      exception_signal = X_SYS;
      break;

    case OP_XOR:
      next_pc = PC + 4;
      if (ins_rd != 0) { GPR[ins_rd] = GPR[ins_rs] ^ GPR[ins_rt]; }
      break;

    case OP_XORI:
      next_pc = PC + 4;
      if (ins_rt != 0) { GPR[ins_rt] = GPR[ins_rs] ^ (ins_misc & 0xFFFF); }
      break;

    case OP_TLBP:
      Unimplemented(PC, "tlbp");
      break;

    case OP_TLBR:
      Unimplemented(PC, "tlbr");
      break;

    case OP_TLBWI:
      Unimplemented(PC, "tlbwi");
      break;

    case OP_TLBWR:
      Unimplemented(PC, "tlbwr");
      break;

    case OP_RES:
      exception_signal = X_RI;
      std::cerr << "Error in component MIPSR3000 : Reserved operation\n";
      std::cerr << "at PC = " << std::hex <<PC << std::dec <<"\n";
      //sc_stop();
      break;

    case OP_ILG: 
      exception_signal = X_RI;
      std::cerr << "Error in component MIPSR3000 : Illegal instruction\n";
      std::cerr << "at PC = " << std::hex << PC << std::dec << "\n";
      //sc_stop();
      break;

    default:
      exception_signal = X_RI;
      std::cerr << "Error in component MIPSR3000 : Illegal codop\n";
      std::cerr << "at PC = " << std::hex << PC << std::dec << "\n";
      //sc_stop();
   } // end switch codop

	if (exception_signal!=0)
	{
		switch (exception_signal)
		{
			case X_SYS:
				CAUSE  = interrupt_signal | (X_SYS << 2);
        			STATUS = (STATUS & ~0x3F) | ((STATUS << 2) & 0x3C);
				EPC=next_pc;
				PC=R3000_GENERAL_EXCEPTION_VECTOR;
				break;
			default:
				break;
		}
	}

  /**********************************************************************
   * On calcule maintenant les valeurs des registres PC, IR, et des
   * registres du CP0 IDENT, STATUS, CAUSE, EPC et BADVADR
   *
   * Le registre IR recopie la valeur venant du cache instruction
   * ICACHE.INS, sauf si il y a une exception, une interruption ou un
   * appel syst�me. Dans ce cas, on force le code NOP (0<=0+0) car
   * l'instruction qui entre dans le pipeline ne doit pas etre ex�cut�e
   * ********************************************************************/

   else if ((STATUS & 0x0000FC00 & interrupt_signal) && 
            (STATUS & 0x00000001)) {

      /* Interrupt handling
       * 
       * On regarde si une interruption non masqu�e a �t� lev�e 
       * Dans ce cas, on sauvegarde l'adresse de retour dans EPC,
       * on branche au gestionnaire d'exceptions, on indique la cause
       * dans le registre de cause, et on force un NOP dans le registre instruction.
       * Il faut un traitement particulier dans le cas des deux instructions
       * RFE et MTC0 qui modifient les registres du coprocesseur 0.
       * Ces �critures n'ont pas �t� effectu�es dans le grand switch
       * d'ex�cution des instructions, mais la variable status_modified 
       * est diff�rente de z�ro. Ces �critures sont effectu�es ici 
       * pour un meilleur controle des �critures dans les registres de CP0.
       * Dans le cas MTCO suivi d'une interruption, il faut copier  
       * le registre GPR(rt) dans le registre cible        
       * puis modifier normalement les registres STATUS, EPC et CAUSE.
       * Dans le cas RFE suivi d'une interruption, il faut simplement 
       * forcer � z�ro les deux bits de poids faible du registre STATUS.
       * puis modifier normalement les registres EPC et CAUSE.
       */

      if (status_modified == OP_RFE) {
          STATUS = STATUS & 0xFFFFFFFC;
          CAUSE  = interrupt_signal;  
	if (delayed_slot)
          EPC    = PC-4;
	else
		EPC=PC;
          PC     = R3000_GENERAL_EXCEPTION_VECTOR;
      } // end if OP_RFE

      else if (status_modified == OP_MTC0) {
          CAUSE  = interrupt_signal;  
	if (delayed_slot)
          EPC    = PC-4;
	else
          EPC    = PC;
          PC     = R3000_GENERAL_EXCEPTION_VECTOR;
          switch (ins_rd) {
          	case 0: // MTC0 -> IDENT
            	IDENT  = GPR[ins_rt]; 
                STATUS = (STATUS & ~0x3F) | ((STATUS << 2) & 0x3C);
            	break;

            	case 12: // MTC0 -> STATUS
                STATUS = (GPR[ins_rt] & ~0x3F) | ((GPR[ins_rt] << 2) & 0x3C);
           	break;

           	case 13: // MTC0 -> CAUSE
                STATUS = (STATUS & ~0x3F) | ((STATUS << 2) & 0x3C);
           	break;

           	case 14: // MTC0 -> EPC
                STATUS = (STATUS & ~0x3F) | ((STATUS << 2) & 0x3C);
           	break;

           	default:
           	Unimplemented(PC, "mtc0");
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
          		CAUSE = (CAUSE & 0x0000003C) | interrupt_signal;
          		PC    = next_pc;
			delayed_slot=delayed;
			break;
		default :   // interrupt
          		STATUS = (STATUS & ~0x3F) | ((STATUS << 2) & 0x3C);
          		CAUSE  = interrupt_signal;  
			if (delayed_slot) {
				EPC=delayed_slot_real_pc;
				delayed_slot=false;
			}
			else
          		EPC    = PC;
          		PC     = R3000_GENERAL_EXCEPTION_VECTOR;
			break;
		} // end switch ins_opcode 
	}  // end else status_modified
  } // end if interrupt

  else {
      /* No interrupt, no exception
       * 
       * si une instruction MTC0 a cherch� � �crire dans un registre    
       * du coprocesseur 0, ou si une instruction RFE a cherch�
       * � modifier la valeur du registre STATUS, ces �critures 
       * n'ont pas �t� effectu�es dans le grand switch
       * d'ex�cution des instructions, mais la variable status_modified 
       * est diff�rente de z�ro. Ces �critures sont effectu�es ici 
       * pour un meilleur controle des �critures dans les registres CP0.
       * Le registre CAUSE est mis � jour systematiquement pour 
       * �chantillonner les interruptions.
       */
	if (delayed_slot) {
		PC=delayed_slot_real_pc;
		delayed_slot=false;
	}
	else
	{
		PC=next_pc;
		delayed_slot=delayed;
	}

       if (status_modified == 0) {
          CAUSE = (CAUSE & 0x0000003C) | interrupt_signal;
       } // end if status_modified = 0
       
	else if (status_modified == OP_RFE) {
          STATUS = (STATUS & 0xFFFFFFF0) | ((STATUS >> 2) & 0xF);
          CAUSE  = (CAUSE & 0x0000003C) | interrupt_signal;
       } // end if OP_RFE
          
	else if (status_modified == OP_MTC0) {  
          switch (ins_rd) {
            case 0: // MTC0 -> IDENT
               CAUSE = (CAUSE & 0x0000003C) | interrupt_signal;
               IDENT = GPR[ins_rt]; 
               break;
	    case 12: // MTC0 -> STATUS
               CAUSE  = (CAUSE & 0x0000003C) | interrupt_signal;
               STATUS = GPR[ins_rt]; 
               break;
            case 13: // MTC0 -> CAUSE
               CAUSE = GPR[ins_rt] | interrupt_signal;
               break;
            case 14: // MTC0 -> EPC
               CAUSE = (CAUSE & 0x0000003C) | interrupt_signal;
               EPC   = GPR[ins_rt]; 
               break;
            default:
               Unimplemented(PC, "mtc0");
	    } // end switch   
       } // end if OP_MTC0

  } // end else no interrupt & no exception
 
};  // end step()

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

void Unimplemented(int pc, char *name)
{
  std::cerr << "Instruction at pc 0x" << std::hex << pc << std::dec
	    << " not implemented in simulator: " << name << "\n";
   sc_stop();
}; // end Unimplemented

// void setInstruction(bool error,int val)
void setInstruction(bool error, int ir)
{
	IR=ir;
}

// void setRdata(bool error,int val)
void setRdata(bool error,int val)
{
        RDATA=val;
}

bool getInstructionRequest(int &type,int &address)
{
	type=1;
	address=PC;
	return true;
}

bool getDataRequest(int &type,int &address,int &wdata)
{
	type=CPU_ACCESS;
	address=CPU_ADDRESS;
	wdata=WDATA;
	if (CPU_OPMEM!=ACCESS_NONE)
		return true;
	else
		return false;
}

void setDataBerr()
{
}

// void setIrq(int irq)
void setIrq(int interrupt)
{
	INTERRUPT_SIGNAL=interrupt;
}

}; // end class MIPSR3000
