
/////////////////////////////////////////////////////////////////////
// File: soclib_vci_xcache.h
// Author: Alain Greiner
// Date : 27/O6/2005
// This program is released under the GNU Public License
// Copyright UPMC/LIP6
//
// HISTORY
// - 27/05/2005 A.Greiner
//   Introduction of the uncached burst mode in the DCACHE.
//   Introduction of the activity counters
// - 14/09/2005 A.Greiner
//   Bug fixing regarding the exit condition from the UNC_WAIT & MISS_WAIT
//   states in the DCACHE, and MISS_WAIT in the ICACHE.
//   Solution is using PKTID field.
//
/////////////////////////////////////////////////////////////////////
//
// This component is a generic cache fully compliant with the
// VCI ADVANCED interface.
//
// The VCI DATA and ADDRESS fields must 32 bits, the VCI ERROR field
// must be 1 bit .
// The TRDID, PKTID, PLEN, CLEN fields are unused, because the VCI
// controller waits the last word of the VCI response packet before sending
// another VCI request packet in order to maintain memory consistency,
// as the VCI interconnect does NOT garanty the "in order delivery" property.
//
//     => The number of lines must be a power of 2 and no larger than 1024.
//     => The number of words must be a power of 2 and no larger than 16.
//
// It contains actually two independant
// instruction and data caches, sharing the same VCI port
// (see the vci_interfaces.h file).
//
// INSTRUCTION CACHE
// The ICACHE is direct mapping.
// In case of MISS, the processor is stalled, the missing address is
// stored in the ICACHE_MISS_ADDR register by the ICACHE controler,
// and a request is posted to the VCI controler in the ICACHE_REQ.
// The missing cache line is written in the ICACHE_MISS_BUF[ICACHE_WORDS]
// buffer by the VCI controler, and the cache is updated by the ICACHE
// controler.
//
// DATA CACHE
// The DCACHE is direct mapping.
// The dcache_write policy is WRITE-THROUGH: the data is always written
// in the memory, and the cache is updated only in case of HIT.
// The data cache contains a write buffer. The VCI controler builds
// a single VCI write packet when there is several successive write
// addresses in the write buffer for the same 4 Kbytes page.
// The DCACHE accepts a "line invalidate" command. The line defined by
// the Y field of the address is invalidated in case of HIT.
// The DCACHE accepts two types of non cachable read :
// single word read (DTYPE_RU), and burst read (DTYPE_RB).
// In case of uncached burst, a prefetch mechanism is activated :
// The VCI controler builds a burst request,
// and a complete cache line is loaded in an uncached buffer.
// This buffer has a capacity of DCACHE-WORDS 32 bits words.
// Data in this buffer can only be read once : There is one valid bit
// per word, that is reset after the first succesful read.
// A processor request is refused (i.e. DCACHE.FRZ = true)
// if there is a READ MISS, a READ UNCACHED, or a WRITE and the
// DATA_FIFO is full.
//
// Therefore, this component contains basically 4 FSMs :
// - DCACHE_FSM controls the DCACHE interface.
// - ICACHE_FSM controls the ICACHE interface.
// - VCI_CMD_FSM controls the VCI request interface.
// - VCI_RSP_FSM controls the VCI response interface.
//
// This component has 4 MEALY outputs: DCACHE.DATA, DCACHE.FRZ,
// ICACHE.INST, ICACHE.FRZ
//
// This component implements a serial  "configuration" bus.
// It allows the software to disable selectively the ICACHE,
// the DCACHE, or both. This is done by writing in the ICACHE_DISABLE
// and DCACHE_DISABLE configuration registers :
//                    0x0-->   : DCACHE active , ICACHE active
//                    0x1-->   : DCACHE active , ICACHE disable
//                    0x2-->   : DCACHE disable, ICACHE active
//                    0x3-->   : DCACHE disable, ICACHE disable
// - When the DCACHE is disabled, both the data cache and the préfetch
// buffer are disabled, and all data read requests are interpreted
// as uncached word requests.
// - When the ICACHE is disabled, the instruction cache is unused,
// and the ICACHE buffer is simply used as a prefetch buffer,
// that behaves as a "one line cache".
// - If this configuration bus is not used, the CFG_CMD and CFG_DATA_IN
// inputs must be connected to 0.
//////////////////////////////////////////////////////////////////////////
// This component has 2 "generator" parameters
// - char*	NAME
// - int	index
//////////////////////////////////////////////////////////////////////////
// This component has 15 "template" parameters
// - ICACHE_LINES : number of lines for the INSTRUCTIN cache
// - ICACHE_WORDS : number of words per line for the INSTRUCTION cache
// - DCACHE_LINES : number of lines for the DATA cache
// - DCACHE_WORDS : number of words per line for the DATA cache
// - ADDRSIZE	  : number of bits of the VCI ADDRESS field
// - CELLSIZE	  : number of bytes the VCI DATA field
// - ERRSIZE	  : number of bits the VCI ERROR field
// - PLENSIZE	  : number of bits the VCI PLEN field
// - CLENSIZE	  : number of bits the VCI CLEN field
// - SRCIDSIZE    : number of bits of the VCI SRCID field
// - TRDIDSIZE    : number of bits of the VCI TRDID field
// - PKTIDSIZE    : number of bits of the VCI PKTID field
/////////////////////////////////////////////////////////////////////////

#ifndef SOCLIB_VCI_XCACHE_H
#define SOCLIB_VCI_XCACHE_H

#include <systemc.h>
#include "cache/soclib_xcache_interface.h"
#include "cache/soclib_xcache_configbus.h"
#include "shared/soclib_vci_interfaces.h"
#include "shared/soclib_generic_fifo.h"

// DCACHE_FSM STATES
enum{
	DCACHE_INIT    		= 0x19,
	DCACHE_IDLE    		= 0x10,
	DCACHE_WRITE_UPDT   	= 0x11,
	DCACHE_WRITE_REQ	= 0x12,
	DCACHE_MISS_REQ		= 0x13,
	DCACHE_MISS_WAIT	= 0x14,
	DCACHE_MISS_UPDT	= 0x15,
	DCACHE_UNC_REQ		= 0x16,
	DCACHE_UNC_WAIT		= 0x17,
	DCACHE_INVAL 		= 0x18};

// ICACHE_FSM STATES
enum{
	ICACHE_INIT		= 0x2F,
	ICACHE_IDLE		= 0x20,
	ICACHE_WAIT		= 0x21,
	ICACHE_UPDT		= 0x24};

// CMD_FSM STATES
enum{
	CMD_IDLE		= 0x30,
	CMD_DATA_LINE		= 0x31,
	CMD_DATA_WORD		= 0x32,
	CMD_DATA_WRITE		= 0x33,
	CMD_INS_MISS		= 0x34};

// RSP_FSM STATES
enum{
	RSP_IDLE		= 0x40,
	RSP_INS_MISS		= 0x41,
	RSP_INS_ERWAIT		= 0x42,
	RSP_INS_ERROR		= 0x43,
	RSP_DATA_MISS		= 0x44,
	RSP_DATA_UNC		= 0x45,
	RSP_DATA_BURST		= 0x46,
	RSP_DATA_WRITE		= 0x47,
	RSP_DATA_ERWAIT 	= 0x48,
	RSP_DATA_ERROR		= 0x49};

// Transaction markers
enum {
	WRITE_PKTID		= 1,
	READ_PKTID		= 0
};

///////////////////////////////
//    structure definition
///////////////////////////////

template<int	ICACHE_LINES,
	 int	ICACHE_WORDS,
	 int	DCACHE_LINES,
	 int	DCACHE_WORDS,
	 int	ADDRSIZE,
	 int	CELLSIZE,
	 int	ERRSIZE,
	 int	PLENSIZE,
	 int	CLENSIZE,
	 int	SRCIDSIZE,
	 int	TRDIDSIZE,
	 int	PKTIDSIZE>

struct SOCLIB_VCI_XCACHE : sc_module {

// PORTS
sc_in<bool> 		CLK;
sc_in<bool> 		RESETN;
ICACHE_CACHE_PORTS 	ICACHE;
DCACHE_CACHE_PORTS 	DCACHE;

ADVANCED_VCI_INITIATOR<	ADDRSIZE,
			CELLSIZE,
			ERRSIZE,
			PLENSIZE,
			CLENSIZE,
			SRCIDSIZE,
			TRDIDSIZE,
			PKTIDSIZE> 	VCI;

sc_in<bool>		CFG_DATA_IN;
sc_out<bool>		CFG_DATA_OUT;
sc_in<sc_uint<2> >	CFG_CMD;

// STRUCTURAL PARAMETERS
const	char 	*NAME;			//  instance name
int  		IDENT;			//  VCI SRCID value
int 		ICACHE_XSHIFT;
int 		ICACHE_XMASK;
int 		ICACHE_YSHIFT;
int 		ICACHE_YMASK;
int 		ICACHE_ZSHIFT;
int 		ICACHE_ZMASK;
int 		DCACHE_XSHIFT;
int 		DCACHE_XMASK;
int 		DCACHE_YSHIFT;
int 		DCACHE_YMASK;
int 		DCACHE_ZSHIFT;
int 		DCACHE_ZMASK;
int		SEGMENT_TYPE_MASK;
int		SEGMENT_TYPE_UNCACHED_WORD;
int		SEGMENT_TYPE_UNCACHED_BURST;

// REGISTERS
sc_signal<short int> DCACHE_FSM;		// DCACHE controler state
sc_signal<int> 		DCACHE_DATA[DCACHE_LINES][DCACHE_WORDS];
sc_signal<int> 		DCACHE_TAG[DCACHE_LINES];
sc_signal<int> 		DCACHE_SAVE_ADDR;
sc_signal<int> 		DCACHE_SAVE_DATA;
sc_signal<int> 		DCACHE_SAVE_TYPE;
sc_signal<int> 		DCACHE_SAVE_PREV;

soclib_generic_fifo<8,32>	DATA_FIFO;	// DCACHE data FIFO
soclib_generic_fifo<8,32>	ADDR_FIFO;	// DCACHE address FIFO
soclib_generic_fifo<8,3>	TYPE_FIFO;	// DCACHE type FIFO

sc_signal<short int> ICACHE_FSM;		// ICACHE controller state
sc_signal<int> 		ICACHE_DATA[ICACHE_LINES][ICACHE_WORDS];
sc_signal<int> 		ICACHE_TAG[ICACHE_LINES];
sc_signal<int> 		ICACHE_MISS_ADDR;
sc_signal<bool> 	ICACHE_REQ;

sc_signal<short int> VCI_CMD_FSM;		// VCI_CMD controler state
sc_signal<int> 		DCACHE_CMD_ADDR;
sc_signal<int> 		DCACHE_CMD_DATA;
sc_signal<int> 		DCACHE_CMD_TYPE;
sc_signal<int> 		DCACHE_MISS_ADDR;
sc_signal<int> 		CMD_CPT;		// counter for VCI request packet
  	
sc_signal<short int> VCI_RSP_FSM;		// VCI_RSP controler state
sc_signal<int> 		ICACHE_MISS_BUF[ICACHE_WORDS];	
sc_signal<bool> 	ICACHE_VAL_BUF[ICACHE_WORDS];	
sc_signal<int> 		DCACHE_MISS_BUF[DCACHE_WORDS];	
sc_signal<bool> 	DCACHE_VAL_BUF[DCACHE_WORDS];	
sc_signal<int> 		RSP_CPT;		// counter for VCI response packet

sc_signal<int> 		DCACHE_CPT_INIT;	// Counter for DCACHE initialisation
sc_signal<int> 		ICACHE_CPT_INIT;	// Counter for ICACHE initialisation

sc_signal<int>		CFG_SHIFT;		// shift register for the config bus
sc_signal<bool>		CFG_DCACHE_DISABLE;	// DCACHE disabled when true
sc_signal<bool>		CFG_ICACHE_DISABLE;	// ICACHE disabled when true

sc_signal<int>		CPT_DCACHE_DATA_READ;	// Activity counter DCACHE DATA READ
sc_signal<int>		CPT_DCACHE_DATA_WRITE;	// Activity counter DCACHE DATA WRITE	
sc_signal<int>		CPT_DCACHE_DIR_READ;	// Activity counter DCACHE DIR READ
sc_signal<int>		CPT_DCACHE_DIR_WRITE;	// Activity counter DCACHE DIR WRITE
sc_signal<int>		CPT_ICACHE_DATA_READ;	// Activity counter ICACHE DATA READ
sc_signal<int>		CPT_ICACHE_DATA_WRITE;	// Activity counter ICACHE DATA WRITE
sc_signal<int>		CPT_ICACHE_DIR_READ;	// Activity counter ICACHE DIR READ
sc_signal<int>		CPT_ICACHE_DIR_WRITE;	// Activity counter ICACHE DIR WRITE
sc_signal<int>		CPT_FIFO_READ;		// Activity counter FIFO READ
sc_signal<int>		CPT_FIFO_WRITE;		// Activity counter FIFO WRITE

/////////////////////////////////
//  constructor
/////////////////////////////////

SC_HAS_PROCESS(SOCLIB_VCI_XCACHE);

SOCLIB_VCI_XCACHE (sc_module_name        insname, 
                   SOCLIB_MAPPING_TABLE &mappingTable, 
                   int                   index)
	: DCACHE_FSM("DCACHE_FSM")
	 ,DCACHE_SAVE_ADDR("DCACHE_SAVE_ADDR")
	 ,DCACHE_SAVE_DATA("DCACHE_SAVE_DATA")
	 ,DCACHE_SAVE_TYPE("DCACHE_SAVE_TYPE")
	 ,DCACHE_SAVE_PREV("DCACHE_SAVE_PREV")
	 ,ICACHE_FSM("ICACHE_FSM")
	 ,ICACHE_MISS_ADDR("ICACHE_MISS_ADDR")
	 ,ICACHE_REQ("ICACHE_REQ")
	 ,VCI_CMD_FSM("VCI_CMD_FSM")
	 ,DCACHE_CMD_ADDR("DCACHE_CMD_ADDR")
	 ,DCACHE_CMD_DATA("DCACHE_CMD_DATA")
	 ,DCACHE_CMD_TYPE("DCACHE_CMD_TYPE")
	 ,DCACHE_MISS_ADDR("DCACHE_MISS_ADDR")
	 ,CMD_CPT("CMD_CPT")
	 ,VCI_RSP_FSM("VCI_RSP_FSM")
	 ,RSP_CPT("RSP_CPT")
	 ,DCACHE_CPT_INIT("DCACHE_CPT_INIT")
	 ,ICACHE_CPT_INIT("ICACHE_CPT_INIT")
	 ,CFG_SHIFT("CFG_SHIFT")
	 ,CFG_DCACHE_DISABLE("CFG_DCACHE_DISABLE")
	 ,CFG_ICACHE_DISABLE("CFG_ICACHE_DISABLE")
	 ,CPT_DCACHE_DATA_READ("CPT_DCACHE_DATA_READ")
	 ,CPT_DCACHE_DATA_WRITE("CPT_DCACHE_DATA_WRITE")
	 ,CPT_DCACHE_DIR_READ("CPT_DCACHE_DIR_READ")
	 ,CPT_DCACHE_DIR_WRITE("CPT_DCACHE_DIR_WRITE")
	 ,CPT_ICACHE_DATA_READ("CPT_ICACHE_DATA_READ")
	 ,CPT_ICACHE_DATA_WRITE("CPT_ICACHE_DATA_WRITE")
	 ,CPT_ICACHE_DIR_READ("CPT_ICACHE_DIR_READ")
	 ,CPT_ICACHE_DIR_WRITE("CPT_ICACHE_DIR_WRITE")
	 ,CPT_FIFO_READ("CPT_FIFO_READ")
	 ,CPT_FIFO_WRITE("CPT_FIFO_WRITE")
//   ,DATA_FIFO("DATA_FIFO")	// DCACHE data FIFO
//   ,ADDR_FIFO("ADDR_FIFO")	// DCACHE address FIFO
//   ,TYPE_FIFO("TYPE_FIFO")	// DCACHE type FIFO
{
#ifdef NONAME_RENAME
        char str[100];
        for (int i=0; i<DCACHE_LINES; i++ ) {
            for (int j=0; j<DCACHE_WORDS; j++ ) {
                sprintf(str, "DCACHE_DATA_%d_%d", i,j);
                DCACHE_DATA[i][j].rename(str);
            }
        }
        for (int i=0; i<DCACHE_LINES; i++ ) {
                sprintf(str, "DCACHE_TAG_%d", i);
                DCACHE_TAG[i].rename(str);
        }
        for (int i=0; i<ICACHE_LINES; i++ ) {
            for (int j=0; j<ICACHE_WORDS; j++ ) {
                sprintf(str, "ICACHE_DATA_%d_%d", i,j);
                ICACHE_DATA[i][j].rename(str);
            }
        }
        for (int i=0; i<ICACHE_LINES; i++ ) {
                sprintf(str, "ICACHE_TAG_%d", i);
                ICACHE_TAG[i].rename(str);
        }
        for (int i=0; i<ICACHE_WORDS; i++ ) {
                sprintf(str, "ICACHE_MISS_BUF_%d", i);
                ICACHE_MISS_BUF[i].rename(str);
        }
        for (int i=0; i<DCACHE_WORDS; i++ ) {
                sprintf(str,"DCACHE_MISS_BUF_%d", i);
                DCACHE_MISS_BUF[i].rename(str);
        }
        for (int i=0; i<ICACHE_WORDS; i++ ) {
                sprintf(str, "ICACHE_VAL_BUF_%d", i);
                ICACHE_VAL_BUF[i].rename(str);
        }
        for (int i=0; i<DCACHE_WORDS; i++ ) {
                sprintf(str,"DCACHE_VAL_BUF_%d", i);
                DCACHE_VAL_BUF[i].rename(str);
        }

DATA_FIFO.rename("DATA_FIFO");	// DCACHE data FIFO
ADDR_FIFO.rename("ADDR_FIFO");	// DCACHE address FIFO
TYPE_FIFO.rename("TYPE_FIFO");	// DCACHE type FIFO
#endif
#ifdef MODELSIM
	//Name the vector signals in MODELSIM
	int i, j, w;
        char str[20];
	for (i=0; i<DCACHE_LINES; i++ ) {
	    for (j=0; j<DCACHE_WORDS; j++ ) {
		w = sprintf(str, "DCACHE_DATA[%d][%d]", i,j);
		DCACHE_DATA[i][j].sc_bind_mti_obj_name(str);
	    }
	}
	for (i=0; i<DCACHE_LINES; i++ ) {
		w = sprintf(str, "DCACHE_TAG[%d]", i);
		DCACHE_TAG[i].sc_bind_mti_obj_name(str);
	}
	for (i=0; i<ICACHE_LINES; i++ ) {
	    for (j=0; j<ICACHE_WORDS; j++ ) {
		w = sprintf(str, "ICACHE_DATA[%d][%d]", i,j);
		ICACHE_DATA[i][j].sc_bind_mti_obj_name(str);
	    }
	}
	for (i=0; i<ICACHE_LINES; i++ ) {
		w = sprintf(str, "ICACHE_TAG[%d]", i);
		ICACHE_TAG[i].sc_bind_mti_obj_name(str);
	}
	for (i=0; i<ICACHE_WORDS; i++ ) {
		w = sprintf(str, "ICACHE_MISS_BUF[%d]", i);
		ICACHE_MISS_BUF[i].sc_bind_mti_obj_name(str);
	}
	for (i=0; i<DCACHE_WORDS; i++ ) {
		w = sprintf(str, "DCACHE_MISS_BUF[%d]", i);
		DCACHE_MISS_BUF[i].sc_bind_mti_obj_name(str);
	}
#endif


SC_METHOD (Transition);
dont_initialize();
  sensitive_pos << CLK;

SC_METHOD (genMoore);
dont_initialize();
  sensitive_neg << CLK;

SC_METHOD (genMealy);
dont_initialize();
  sensitive_neg << CLK;
  sensitive     << DCACHE.TYPE;
  sensitive     << DCACHE.ADR;
  sensitive     << DCACHE.REQ;
  sensitive     << DCACHE.UNC;
  sensitive     << ICACHE.REQ;
  sensitive     << ICACHE.ADR;

#if defined(SYSTEMCASS_SPECIFIC)
	ICACHE.FRZ  (ICACHE.REQ);
	ICACHE.INS  (ICACHE.REQ);
	ICACHE.FRZ  (ICACHE.ADR);
	ICACHE.INS  (ICACHE.ADR);
	DCACHE.FRZ  (DCACHE.TYPE);
	DCACHE.FRZ  (DCACHE.ADR);
	DCACHE.FRZ  (DCACHE.REQ);
	DCACHE.FRZ  (DCACHE.UNC);
	DCACHE.RDATA(DCACHE.ADR);
#endif

#if defined(SYSTEMCASS_SPECIFIC)
  SAVE_HANDLER(save_state);
#endif

// ICACHE parameters initialisation & verification

  ICACHE_XSHIFT   = 2;

  switch (ICACHE_WORDS) {
      case 1 :
        ICACHE_YSHIFT = ICACHE_XSHIFT;
        ICACHE_XMASK  = 0x0;
      break;
      case 2 :
        ICACHE_YSHIFT = ICACHE_XSHIFT + 1;
        ICACHE_XMASK  = 0x1 << ICACHE_XSHIFT;
      break;
      case 4 :
        ICACHE_YSHIFT = ICACHE_XSHIFT + 2;
        ICACHE_XMASK  = 0x3 << ICACHE_XSHIFT;
      break;
      case 8 :
        ICACHE_YSHIFT = ICACHE_XSHIFT + 3;
        ICACHE_XMASK  = 0x7 << ICACHE_XSHIFT;
      break;
      case 16 :
        ICACHE_YSHIFT = ICACHE_XSHIFT + 4;
        ICACHE_XMASK  = 0xF << ICACHE_XSHIFT;
      break;
      default :
      	printf("error in the soclib_vci_xcache :\n");
      	printf("cache line in the instruction cache must be 1, 2, 4, 8, or 16 words\n");
      sc_stop();
      break;
  }
  switch (ICACHE_LINES) {
      case 1 :
        ICACHE_ZSHIFT = ICACHE_YSHIFT;
        ICACHE_YMASK  = 0x0;
      break;
      case 2 :
        ICACHE_ZSHIFT = ICACHE_YSHIFT + 1;
        ICACHE_YMASK  = 0x1 << ICACHE_YSHIFT;
      break;
      case 4 :
        ICACHE_ZSHIFT = ICACHE_YSHIFT + 2;
        ICACHE_YMASK  = 0x3 << ICACHE_YSHIFT;
      break;
      case 8 :
        ICACHE_ZSHIFT = ICACHE_YSHIFT + 3;
        ICACHE_YMASK  = 0x7 << ICACHE_YSHIFT;
      break;
      case 16 :
        ICACHE_ZSHIFT = ICACHE_YSHIFT + 4;
        ICACHE_YMASK  = 0xF << ICACHE_YSHIFT;
      break;
      case 32 :
        ICACHE_ZSHIFT = ICACHE_YSHIFT + 5;
        ICACHE_YMASK  = 0x1F << ICACHE_YSHIFT;
      break;
      case 64 :
        ICACHE_ZSHIFT = ICACHE_YSHIFT + 6;
        ICACHE_YMASK  = 0x3F << ICACHE_YSHIFT;
      break;
      case 128 :
        ICACHE_ZSHIFT = ICACHE_YSHIFT + 7;
        ICACHE_YMASK  = 0x7F << ICACHE_YSHIFT;
      break;
      case 256 :
        ICACHE_ZSHIFT = ICACHE_YSHIFT + 8;
        ICACHE_YMASK  = 0xFF << ICACHE_YSHIFT;
      break;
      case 512 :
        ICACHE_ZSHIFT = ICACHE_YSHIFT + 9;
        ICACHE_YMASK  = 0x1FF << ICACHE_YSHIFT;
      break;
      case 1024 :
        ICACHE_ZSHIFT = ICACHE_YSHIFT + 10;
        ICACHE_YMASK  = 0x3FF << ICACHE_YSHIFT;
      break;
      default :
      	printf("error in the soclib_vci_xcache :\n");
      	printf("number of lines in the instruction cache must be a power of 2 not larger than 1024\n");
      sc_stop();
      break;
  }

  ICACHE_ZMASK    = ((~0x0) << ICACHE_ZSHIFT);

// DCACHE parameters initialisation & verification

// The segment type is determined by the 2 most significant bytes of the LSB field of the VCI address,
// these values are calculated in the constructor to avoid recalculating them at every request to DCACHE
//  ________ ____ ____________________
// |  MSB   |LSB |                    |
// |________ XX__ ____________________|
//
// 00 & 01 => SEGMENT_CACHED
// 10      => SEGMENT_UNCACHED_WORD
// 11      => SEGMENT_UNCACHED_BURST

  SEGMENT_TYPE_MASK	= mappingTable.getSegTypeMask();
  int segmask	= SEGMENT_TYPE_MASK;
  int cpt=0;

  while ((segmask&1)==0)
  {
      segmask>>=1;
      cpt++;
  }

  SEGMENT_TYPE_UNCACHED_WORD	= 2 << cpt;
  SEGMENT_TYPE_UNCACHED_BURST	= 3 << cpt;

//printf ("SEGMENT_TYPE_MASK=%x  SEGMENT_TYPE_UNCACHED_WORD=%x  SEGMENT_TYPE_UNCACHED_BURST=%x\n",SEGMENT_TYPE_MASK, SEGMENT_TYPE_UNCACHED_WORD, SEGMENT_TYPE_UNCACHED_BURST);

  DCACHE_XSHIFT   = 2;

  switch (DCACHE_WORDS) {
      case 1 :
        DCACHE_YSHIFT = DCACHE_XSHIFT;
        DCACHE_XMASK  = 0x0;
      break;
      case 2 :
        DCACHE_YSHIFT = DCACHE_XSHIFT + 1;
        DCACHE_XMASK  = 0x1 << DCACHE_XSHIFT;
      break;
      case 4 :
        DCACHE_YSHIFT = DCACHE_XSHIFT + 2;
        DCACHE_XMASK  = 0x3 << DCACHE_XSHIFT;
      break;
      case 8 :
        DCACHE_YSHIFT = DCACHE_XSHIFT + 3;
        DCACHE_XMASK  = 0x7 << DCACHE_XSHIFT;
      break;
      case 16 :
        DCACHE_YSHIFT = DCACHE_XSHIFT + 4;
        DCACHE_XMASK  = 0xF << DCACHE_XSHIFT;
      break;
      default :
      	printf("error in the soclib_vci_xcache :\n");
      	printf("cache line in the data cache must be 1, 2, 4, 8, or 16 words\n");
      sc_stop();
      break;
  }
  switch (DCACHE_LINES) {
      case 1 :
        DCACHE_ZSHIFT = DCACHE_YSHIFT;
        DCACHE_YMASK  = 0x0;
      break;
      case 2 :
        DCACHE_ZSHIFT = DCACHE_YSHIFT + 1;
        DCACHE_YMASK  = 0x1 << DCACHE_YSHIFT;
      break;
      case 4 :
        DCACHE_ZSHIFT = DCACHE_YSHIFT + 2;
        DCACHE_YMASK  = 0x3 << DCACHE_YSHIFT;
      break;
      case 8 :
        DCACHE_ZSHIFT = DCACHE_YSHIFT + 3;
        DCACHE_YMASK  = 0x7 << DCACHE_YSHIFT;
      break;
      case 16 :
        DCACHE_ZSHIFT = DCACHE_YSHIFT + 4;
        DCACHE_YMASK  = 0xF << DCACHE_YSHIFT;
      break;
      case 32 :
        DCACHE_ZSHIFT = DCACHE_YSHIFT + 5;
        DCACHE_YMASK  = 0x1F << DCACHE_YSHIFT;
      break;
      case 64 :
        DCACHE_ZSHIFT = DCACHE_YSHIFT + 6;
        DCACHE_YMASK  = 0x3F << DCACHE_YSHIFT;
      break;
      case 128 :
        DCACHE_ZSHIFT = DCACHE_YSHIFT + 7;
        DCACHE_YMASK  = 0x7F << DCACHE_YSHIFT;
      break;
      case 256 :
        DCACHE_ZSHIFT = DCACHE_YSHIFT + 8;
        DCACHE_YMASK  = 0xFF << DCACHE_YSHIFT;
      break;
      case 512 :
        DCACHE_ZSHIFT = DCACHE_YSHIFT + 9;
        DCACHE_YMASK  = 0x1FF << DCACHE_YSHIFT;
      break;
      case 1024 :
        DCACHE_ZSHIFT = DCACHE_YSHIFT + 10;
        DCACHE_YMASK  = 0x3FF << DCACHE_YSHIFT;
      break;
      default :
      	printf("error in the soclib_vci_xcache :\n");
      	printf("number of lines in the data cache must be a power of 2 not larger than 1024\n");
        sc_stop();
      break;
  }

// Checking VCI parameters

if(ADDRSIZE != 32) {
	printf("error in the soclib_vci_xcache : %s\n", NAME);
	printf("the ADDRSIZE parameter must be 32\n");
	sc_stop();
}

if(CELLSIZE != 4) {
	printf("error in the soclib_vci_xcache : %s\n", NAME);
	printf("the CELLSIZE parameter must be 4\n");
	sc_stop();
}

if(ERRSIZE != 1) {
	printf("error in the soclib_vci_xcache : %s\n", NAME);
	printf("the ERRSIZE parameter must be 1\n");
	sc_stop();
}

if(TRDIDSIZE > 32) {
	printf("error in the soclib_vci_xcache : %s\n", NAME);
	printf("the TRDIDSIZE parameter cannot be larger than 32\n");
	sc_stop();
}

if(PKTIDSIZE < 1) {
	printf("error in the soclib_vci_xcache : %s\n", NAME);
	printf("the PKTIDSIZE parameter must be equal or larger than 1\n");
	sc_stop();
}

DCACHE_ZMASK    = ((~0x0) << DCACHE_ZSHIFT);

IDENT    	= index;
NAME     	= (const char*) insname;

printf ("Successful Instanciation of SOCLIB_VCI_XCACHE : %s\n",NAME);

}; // end  constructor

///////////////////////////////////////
//  Transition method
///////////////////////////////////////

void Transition()
{

int 		icache_address;
int 		icache_y;
int 		icache_z;
bool 		icache_hit;		// hit on the instruction cache

int 		dcache_address;
int 		dcache_x;
int 		dcache_y;
int 		dcache_z;
bool 		dcache_hit;		// hit on the data cache
bool 		dcache_buf_hit; 	// hit on the uncached buffer
bool 		dcache_read;		// cached read request
bool 		dcache_unc;		// uncached request (word or burst)
bool 		dcache_inval;		// line invalidate request
bool 		dcache_write;		// write request
int 		dcache_wmsk;
int 		dcache_byte;
bool 		dcache_validreq;

bool		fifo_put;
bool		fifo_get;
int		data_fifo;
int		addr_fifo;
int		type_fifo;

int		type_segment;

//////////////////////////////
//        RESET
//////////////////////////////

if (RESETN == false) {
	  DCACHE_FSM = DCACHE_INIT;
	  DCACHE_CPT_INIT = DCACHE_LINES - 1;
	  ICACHE_FSM = ICACHE_INIT;
	  ICACHE_CPT_INIT = ICACHE_LINES - 1;
	  DATA_FIFO.init();
	  TYPE_FIFO.init();
	  ADDR_FIFO.init();
	  ICACHE_REQ = false;
	  VCI_CMD_FSM = CMD_IDLE;
	  VCI_RSP_FSM = RSP_IDLE;
	  CFG_ICACHE_DISABLE = false;
	  CFG_DCACHE_DISABLE = false;
	  for(int i = 0 ; i  < DCACHE_WORDS ; i++) { DCACHE_VAL_BUF[i] = false; }
	  ICACHE_MISS_ADDR = 0xFFFFFFFF;
	  CPT_DCACHE_DATA_READ  = 0;
	  CPT_DCACHE_DATA_WRITE = 0;
	  CPT_DCACHE_DIR_READ  = 0;
	  CPT_DCACHE_DIR_WRITE = 0;
	  CPT_ICACHE_DATA_READ  = 0;
	  CPT_ICACHE_DATA_WRITE = 0;
	  CPT_ICACHE_DIR_READ  = 0;
	  CPT_ICACHE_DIR_WRITE = 0;
	  CPT_FIFO_READ  = 0;
	  CPT_FIFO_WRITE = 0;
	  return;
} // end RESET
// icache_hit & icache_address
icache_address  = (int)ICACHE.ADR.read() & (ICACHE_ZMASK | ICACHE_YMASK);
if (CFG_ICACHE_DISABLE == false) { // cache enabled
	icache_y = (icache_address & ICACHE_YMASK) >> ICACHE_YSHIFT;
	icache_z = (icache_address >> ICACHE_ZSHIFT) | 0x80000000;
	if (icache_z == (ICACHE_TAG[icache_y]))   { icache_hit = true; }
	else      	                          { icache_hit = false; }
} else {			// cache disabled
	if(icache_address == ICACHE_MISS_ADDR)	{icache_hit = true; }
	else                                  	{icache_hit = false;}
}

// dcache_read, dcache_write, dcache_inval, dcache_unc

switch((int)DCACHE.TYPE.read()) {
    case DTYPE_RW :
    type_segment = (int)(DCACHE.ADR.read() & SEGMENT_TYPE_MASK);
    if (type_segment == SEGMENT_TYPE_UNCACHED_WORD) {
    dcache_read  = false;
    dcache_write = false;
    dcache_inval = false;
    dcache_unc   = true;
    } else if (type_segment == SEGMENT_TYPE_UNCACHED_BURST) {
    dcache_read  = false;
    dcache_write = false;
    dcache_inval = false;
    dcache_unc   = true;
    } else {
    dcache_read  = true;
    dcache_write = false;
    dcache_inval = false;
//    dcache_unc   = false;
    dcache_unc   = DCACHE.UNC.read();
    }
    break;
/*
    case DTYPE_RU :
    dcache_read  = false;
    dcache_write = false;
    dcache_inval = false;
    dcache_unc   = true;
    break;
    case DTYPE_RB :
    dcache_read  = false;
    dcache_write = false;
    dcache_inval = false;
    dcache_unc   = true;
    break;
*/
    case DTYPE_RZ :
    dcache_read  = false;
    dcache_write = false;
    dcache_inval = true;
//    dcache_unc   = false;
    dcache_unc   = DCACHE.UNC.read();
    break;
    case DTYPE_WW :
    dcache_read  = false;
    dcache_write = true;
    dcache_inval = false;
//    dcache_unc   = false;
    dcache_unc   = DCACHE.UNC.read();
    break;
    case DTYPE_WH :
    dcache_read  = false;
    dcache_write = true;
    dcache_inval = false;
//    dcache_unc   = false;
    dcache_unc   = DCACHE.UNC.read();
    break;
    case DTYPE_WB :
    dcache_read  = false;
    dcache_write = true;
    dcache_inval = false;
//    dcache_unc   = false;
    dcache_unc   = DCACHE.UNC.read();
    break;
    default :
    dcache_read  = false;
    dcache_write = false;
    dcache_inval = false;
//    dcache_unc   = false;
    dcache_unc   = DCACHE.UNC.read();
    break;

} // end switch TYPE

// dcache_address, dcache_x, dcache_y, dcache_z
dcache_address  = (int)DCACHE.ADR.read();
dcache_x = (dcache_address & DCACHE_XMASK) >> DCACHE_XSHIFT;
dcache_y = (dcache_address & DCACHE_YMASK) >> DCACHE_YSHIFT;
dcache_z = (dcache_address >> DCACHE_ZSHIFT) | 0x80000000;

//  dcache_buf_hit
if ((dcache_address & ~(DCACHE_XMASK)) == (DCACHE_MISS_ADDR & ~(DCACHE_XMASK)))
	{ dcache_buf_hit = DCACHE_VAL_BUF[dcache_x]; }
else	{ dcache_buf_hit = false; }

// dcache_hit (a conditionnal expression is faster than an if statement)
dcache_hit = (dcache_z == (DCACHE_TAG[dcache_y])); 
// 

// dcache_validreq (a conditionnal expression is faster than an if statement)
dcache_validreq = ((DCACHE.REQ == true) && (icache_hit == true));

/////////////////////////////////////////////////////////////////////
// The ICACHE_FSM controls the following ressources :
// - ICACHE_FSM
// - ICACHE_DATA[ICACHE_WORDS,ICACHE_LINES]
// - ICACHE_TAG[ICACHE_LINES]
// - ICACHE_MISS_ADDR
// - ICACHE_REQ set
// - ICACHE_CPT_INIT
//
// The VALID bit for a cache line is the MSB bit in the TAG.
//
// In case of MISS, the controller writes a request in the
// ICACHE_MISS_ADDR register and sets the ICACHE_REQ flip-flop.
// The ICACHE_REQ flip-flop is reset by the VCI_RSP controller,
// when the cache line is ready in the ICACHE buffer.
// When the CFG_ICACHE_DISABLE register is false, the ICACHE controler
// updates the cache and returns in IDLE state. When this flag is false,
// the ICACHE controler returns directly in the IDLE state.
/////////////////////////////////////////////////////////////////////

switch(ICACHE_FSM.read()) {

case ICACHE_INIT :
ICACHE_TAG[ICACHE_CPT_INIT] = 0;
ICACHE_CPT_INIT = ICACHE_CPT_INIT - 1;
if (ICACHE_CPT_INIT == 0) { ICACHE_FSM = ICACHE_IDLE; }
CPT_ICACHE_DIR_WRITE = CPT_ICACHE_DIR_WRITE + 1;
break;
	
case ICACHE_IDLE :
if (ICACHE.REQ == true) {
	if (icache_hit == false) {
	ICACHE_FSM = ICACHE_WAIT;
	ICACHE_MISS_ADDR = icache_address;
	ICACHE_REQ = true;
	}
	CPT_ICACHE_DIR_READ = CPT_ICACHE_DIR_READ + 1;
	CPT_ICACHE_DATA_READ = CPT_ICACHE_DATA_READ + 1;
}
break;

case ICACHE_WAIT :
if ((VCI.RSPVAL == true) && (VCI.REOP == true) && (VCI.RPKTID.read() == READ_PKTID)) 
	if ((CFG_ICACHE_DISABLE == true) || 
	    (VCI_RSP_FSM == RSP_INS_ERROR) || 
	    (VCI.RERROR.read() != 0)) 		ICACHE_FSM = ICACHE_IDLE;
	else					ICACHE_FSM = ICACHE_UPDT;
break;

case ICACHE_UPDT :
icache_y = (ICACHE_MISS_ADDR & ICACHE_YMASK) >> ICACHE_YSHIFT;
icache_z = (ICACHE_MISS_ADDR >> ICACHE_ZSHIFT) | 0x80000000;
ICACHE_TAG[icache_y] = icache_z;
for (int i=0 ; i<ICACHE_WORDS ; i++) { ICACHE_DATA[icache_y][i] = ICACHE_MISS_BUF[i]; }
ICACHE_FSM = ICACHE_IDLE;
CPT_ICACHE_DIR_WRITE = CPT_ICACHE_DIR_WRITE + 1;
CPT_ICACHE_DATA_WRITE = CPT_ICACHE_DATA_WRITE + 1;
break;
} // end switch ICACHE_FSM

//////////////////////////////////////////////////////////////////////://///////////
// The DCACHE_FSM controls the following ressources :
// - DCACHE_FSM
// - DCACHE_DATA[DCACHE_WORDS,DCACHE_LINES]
// - DCACHE_TAG[DCACHE_LINES]
// - DCACHE_SAVE_ADDR
// - DCACHE_SAVE_TYPE
// - DCACHE_SAVE_DATA
// - DCACHE_SAVE_PREV
// - DCACHE_CPT_INIT
// - DCACHE_VAL_BUF[DCACHE_WORDS] reset
// - fifo_put, data_fifo, addr_fifo, type_fifo
//
// The VALID bit for a cache line is the MSB bit in the TAG.
// The request type written in the FIFO is a copy of the processor request.
// In the IDLE state, the processor request is saved in the DCACHE_SAVE_ADDR,
// DCACHE_SAVE_DATA, DCACHE_SAVE_TYPE registers.
// The data word read in the cache is saved in DCACHE_SAVE_PREV.
//
// There is five mutually exclusive conditions to exit the IDLE state :
// - CACHED READ MISS => to the MISS_REQ state (to post the request in the FIFO),
// then to the MISS_WAIT state (waiting the cache line), then to the MISS_UPDT
// (to update the cache), and finally to the IDLE state.
// - UNCACHED READ (simple word or burst) => to the UNC_REQ state
// (to post the request in the FIFO), then to the UNC_WAIT state,
// and finally to the IDLE state.
// - CACHE INVALIDATE HIT => to the INVAL state for one cycle, then IDLE.
// - WRITE MISS => directly to the WRITE_REQ state (to post the request in the FIFO)
// Then it depends on the processor request: In order to support VCI write burst,
// the processor requests are taken into account in the WRITE_REQ state
// as well as in the IDLE state.
// - WRITE HIT => to the WRITE_UPDT state (to update the cache), then to
// the WRITE_REQ state.
/////////////////////////////////////////////////////://////////////////////////////

switch (DCACHE_FSM) {

case DCACHE_INIT :
fifo_put = false;
DCACHE_TAG[DCACHE_CPT_INIT] = 0;
DCACHE_CPT_INIT = DCACHE_CPT_INIT - 1;
if (DCACHE_CPT_INIT == 0) { DCACHE_FSM = DCACHE_IDLE; }
CPT_DCACHE_DIR_WRITE = CPT_DCACHE_DIR_WRITE + 1;
break;

case DCACHE_IDLE :
fifo_put = false;
if (dcache_validreq == true) {
        DCACHE_SAVE_ADDR = (int)DCACHE.ADR.read();
       	DCACHE_SAVE_DATA = (int)DCACHE.WDATA.read();
	DCACHE_SAVE_TYPE = (int)DCACHE.TYPE.read();
       	DCACHE_SAVE_PREV = DCACHE_DATA[dcache_y][dcache_x];
	if ((dcache_read == true) && (dcache_hit == false)) { // cached read miss 	
		for(int i = 0 ; i < DCACHE_WORDS ; i++) { DCACHE_VAL_BUF[i] = false; }
		DCACHE_FSM = DCACHE_MISS_REQ;
	} else if ((dcache_unc == true) && (dcache_buf_hit == false)) {  // uncached read miss
		for(int i = 0 ; i < DCACHE_WORDS ; i++) { DCACHE_VAL_BUF[i] = false; }
		DCACHE_FSM = DCACHE_UNC_REQ;
	} else if ((dcache_unc == true) && (dcache_buf_hit == true))  {  // uncached read hit
		DCACHE_VAL_BUF[dcache_x] = false;
	} else if ((dcache_write == true) && (dcache_hit == false)) { // write miss
		if (dcache_buf_hit == true) { DCACHE_VAL_BUF[dcache_x] = false; }
		DCACHE_FSM = DCACHE_WRITE_REQ;
	} else if ((dcache_write == true) && (dcache_hit == true)) { // write hit	
		if (dcache_buf_hit == true) { DCACHE_VAL_BUF[dcache_x] = false; }
		DCACHE_FSM = DCACHE_WRITE_UPDT;
	} else if ((dcache_inval == true) && (dcache_hit == true)) { // line invalidate	
		DCACHE_FSM = DCACHE_INVAL;
	}
CPT_DCACHE_DATA_READ = CPT_DCACHE_DATA_READ + 1;
CPT_DCACHE_DIR_READ = CPT_DCACHE_DIR_READ + 1;
}
break;

case DCACHE_WRITE_UPDT :
fifo_put = false;
dcache_x = (DCACHE_SAVE_ADDR & DCACHE_XMASK) >> DCACHE_XSHIFT;
dcache_y = (DCACHE_SAVE_ADDR & DCACHE_YMASK) >> DCACHE_YSHIFT;
dcache_wmsk = DCACHE_SAVE_TYPE & 0x00000003;
dcache_byte = DCACHE_SAVE_ADDR  & 0x00000003;
if (dcache_wmsk == 0) {         // write word
	DCACHE_DATA[dcache_y][dcache_x] = DCACHE_SAVE_DATA;
} else if (dcache_wmsk == 1) {  // write half
	if (dcache_byte == 0) {
		DCACHE_DATA[dcache_y][dcache_x] = (DCACHE_SAVE_PREV         & 0xFFFF0000) |
                                               	  (DCACHE_SAVE_DATA         & 0x0000FFFF) ;
	} else {
		DCACHE_DATA[dcache_y][dcache_x] = (DCACHE_SAVE_PREV         & 0x0000FFFF) |
                                                  ((DCACHE_SAVE_DATA << 16) & 0xFFFF0000) ;
	}
} else {                        // write byte
	if (dcache_byte == 0) {
		DCACHE_DATA[dcache_y][dcache_x] = (DCACHE_SAVE_PREV         & 0xFFFFFF00) |
                                               	  (DCACHE_SAVE_DATA         & 0x000000FF) ;
	} else if (dcache_byte == 1) {
		DCACHE_DATA[dcache_y][dcache_x] = (DCACHE_SAVE_PREV         & 0xFFFF00FF) |
                                               	  ((DCACHE_SAVE_DATA << 8 ) & 0x0000FF00) ;
       	} else if (dcache_byte == 2) {
		DCACHE_DATA[dcache_y][dcache_x] = (DCACHE_SAVE_PREV         & 0xFF00FFFF) |
                                               	  ((DCACHE_SAVE_DATA << 16) & 0x00FF0000) ;
       	} else if (dcache_byte == 3) {
		DCACHE_DATA[dcache_y][dcache_x] = (DCACHE_SAVE_PREV         & 0x00FFFFFF) |
                                               	  ((DCACHE_SAVE_DATA << 24) & 0xFF000000) ;
	}
}
DCACHE_FSM = DCACHE_WRITE_REQ;
CPT_DCACHE_DATA_WRITE = CPT_DCACHE_DATA_WRITE + 1;
break;

case DCACHE_WRITE_REQ :
fifo_put = true;
data_fifo = DCACHE_SAVE_DATA;
addr_fifo = DCACHE_SAVE_ADDR;
type_fifo = DCACHE_SAVE_TYPE;

if (DATA_FIFO.wok() == true) {
	DCACHE_SAVE_ADDR = (int)DCACHE.ADR.read();
	DCACHE_SAVE_DATA = (int)DCACHE.WDATA.read();
	DCACHE_SAVE_PREV = DCACHE_DATA[dcache_y][dcache_x];
	if (dcache_unc == true) { DCACHE_SAVE_TYPE = (int)DCACHE.TYPE.read() + 8; }
	else 			{ DCACHE_SAVE_TYPE = (int)DCACHE.TYPE.read(); }
	if ((dcache_validreq == true) && (dcache_read == true) && (dcache_hit == false)) { // cached read miss 	
		for(int i = 0 ; i < DCACHE_WORDS ; i++) { DCACHE_VAL_BUF[i] = false; }
		DCACHE_FSM = DCACHE_MISS_REQ;
	} else if ((dcache_validreq == true) && (dcache_unc == true) && (dcache_buf_hit == false)) {  // uncached read miss
		for(int i = 0 ; i < DCACHE_WORDS ; i++) { DCACHE_VAL_BUF[i] = false; }
		DCACHE_FSM = DCACHE_UNC_REQ;
	} else if ((dcache_validreq == true) && (dcache_unc == true) && (dcache_buf_hit == true))  {  // uncached read hit
		DCACHE_VAL_BUF[dcache_x] = false;
	} else if ((dcache_validreq == true) && (dcache_write == true) && (dcache_hit == false)) { // write miss
		if (dcache_buf_hit == true) { DCACHE_VAL_BUF[dcache_x] = false; }
		DCACHE_FSM = DCACHE_WRITE_REQ;
	} else if ((dcache_validreq == true) && (dcache_write == true) && (dcache_hit == true)) { // write hit	
		if (dcache_buf_hit == true) { DCACHE_VAL_BUF[dcache_x] = false; }
		DCACHE_FSM = DCACHE_WRITE_UPDT;
	} else if ((dcache_validreq == true) && (dcache_inval == true) && (dcache_hit == true)) { // line invalidate	
		DCACHE_FSM = DCACHE_INVAL;
	}
	 else  {DCACHE_FSM = DCACHE_IDLE;}
CPT_DCACHE_DATA_READ = CPT_DCACHE_DATA_READ + 1;
CPT_DCACHE_DIR_READ = CPT_DCACHE_DIR_READ + 1;
CPT_FIFO_WRITE = CPT_FIFO_WRITE + 1;
}
break;

case DCACHE_MISS_REQ :
fifo_put = true;
data_fifo = DCACHE_SAVE_DATA;
addr_fifo = DCACHE_SAVE_ADDR;
type_fifo = DCACHE_SAVE_TYPE;
if (DATA_FIFO.wok() == true) {
	DCACHE_FSM = DCACHE_MISS_WAIT;
	CPT_FIFO_WRITE = CPT_FIFO_WRITE + 1;
}
break;

case DCACHE_MISS_WAIT :
fifo_put = false;
if ((VCI.RSPVAL == true) && (VCI.REOP == true) && (VCI.RPKTID.read() == READ_PKTID))
        if ((CFG_DCACHE_DISABLE == true) ||
            (VCI_RSP_FSM == RSP_DATA_ERROR) ||
            (VCI.RERROR.read() != 0))           DCACHE_FSM = DCACHE_IDLE;
        else                                    DCACHE_FSM = DCACHE_MISS_UPDT;
break;

case DCACHE_MISS_UPDT :
fifo_put = false;
dcache_y = (DCACHE_MISS_ADDR & DCACHE_YMASK) >> DCACHE_YSHIFT;
dcache_z = (DCACHE_MISS_ADDR >> DCACHE_ZSHIFT) | 0x80000000;
DCACHE_TAG[dcache_y] = dcache_z;
for (int i=0 ; i<DCACHE_WORDS ; i++) { DCACHE_DATA[dcache_y][i] = DCACHE_MISS_BUF[i]; }
DCACHE_FSM = DCACHE_IDLE;
CPT_DCACHE_DATA_WRITE = CPT_DCACHE_DATA_WRITE + 1;
CPT_DCACHE_DIR_WRITE = CPT_DCACHE_DIR_WRITE + 1;
break;

case DCACHE_UNC_REQ :
fifo_put = true;
data_fifo = DCACHE_SAVE_DATA;
addr_fifo = DCACHE_SAVE_ADDR;
type_fifo = DCACHE_SAVE_TYPE;
if (DATA_FIFO.wok() == true) {
	DCACHE_FSM = DCACHE_UNC_WAIT;
	CPT_FIFO_WRITE = CPT_FIFO_WRITE + 1;
}
break;

case DCACHE_UNC_WAIT :
fifo_put = false;
if ((VCI.RSPVAL == true) && (VCI.REOP == true) && (VCI.RPKTID.read() == READ_PKTID))
    DCACHE_FSM = DCACHE_IDLE;
break;

case DCACHE_INVAL :
fifo_put = false;
dcache_y = (DCACHE_SAVE_ADDR & DCACHE_YMASK) >> DCACHE_YSHIFT;
DCACHE_TAG[dcache_y] = 0;
DCACHE_FSM = DCACHE_IDLE;
CPT_DCACHE_DIR_WRITE = CPT_DCACHE_DIR_WRITE + 1;
break;
} // end switch DCACHE_FSM

////////////////////////////////////////////////////////////////////////////
// The VCI_CMD_FSM controls the following ressources :
// - VCI_CMD_FSM
// - DCACHE_CMD_DATA
// - DCACHE_CMD_ADDR
// - DCACHE_CMD_TYPE
// - DCACHE_MISS_ADDR
// - CMD_CPT
// - fifo_get
//
// This FSM handles requests from both the DCACHE controler
// (DATA_FIFO non empty) and the ICACHE controler (ICACHE_REQ).
// As the DCACHE miss and the DCACHE uncached burst generate the same VCI request,
// there is actually 4 request types :
// - INS_MISS
// - DATA_LINE
// - DATA_WORD
// - DATA_WRITE
// The DCACHE requests have the highest priority.
// There is at most one (REQ/RSP) VCI transaction, as both CMD_FSM and RSP_FSM
// exit simultaneously the IDLE state.
// A request is consumed from the DATA_FIFO each time the CMD and RSP FSMs
// are in IDLE state, or the CMD FSM is in CMD_DATA_WRITE state, and there is
// another write request for the same 4K bytes page.
//////////////////////////////////////////////////////////////////////////////

switch ((VCI_CMD_FSM)) {
	
case CMD_IDLE :
CMD_CPT = 0;
fifo_get = false;
if (VCI_RSP_FSM == RSP_IDLE) {
	if (DATA_FIFO.rok() == true) {
		CPT_FIFO_READ = CPT_FIFO_READ + 1;
		fifo_get = true;
		DCACHE_CMD_DATA   = (int)DATA_FIFO.read();
		DCACHE_CMD_ADDR   = (int)ADDR_FIFO.read();
		DCACHE_CMD_TYPE   = (int)TYPE_FIFO.read();

		type_segment = (int)DCACHE.ADR.read() & SEGMENT_TYPE_MASK;
		if (((int)TYPE_FIFO.read()) == DTYPE_RW) 	
			{
				if 	(type_segment == SEGMENT_TYPE_UNCACHED_WORD) 	{ VCI_CMD_FSM = CMD_DATA_WORD; }
				else if (type_segment == SEGMENT_TYPE_UNCACHED_BURST) 	{ VCI_CMD_FSM = CMD_DATA_LINE; } //inutile
				else 						{ VCI_CMD_FSM = CMD_DATA_LINE; }
			}
		else                                          			{ VCI_CMD_FSM = CMD_DATA_WRITE; }
	} else if (ICACHE_REQ == true) {
		VCI_CMD_FSM = CMD_INS_MISS;
	}
}
break;

case CMD_INS_MISS :
fifo_get = false;
if ((VCI.CMDACK) == true) {
	CMD_CPT = CMD_CPT + 1;
	if (CMD_CPT == ICACHE_WORDS - 1) { VCI_CMD_FSM = CMD_IDLE; }
}
break;

case CMD_DATA_WORD :
fifo_get = false;
DCACHE_MISS_ADDR = DCACHE_CMD_ADDR;
if ((VCI.CMDACK) == true) { VCI_CMD_FSM = CMD_IDLE; }
break;

case CMD_DATA_LINE :
fifo_get = false;
DCACHE_MISS_ADDR = DCACHE_CMD_ADDR;
if ((VCI.CMDACK) == true) {
	CMD_CPT = CMD_CPT + 1;
	if (CMD_CPT == DCACHE_WORDS - 1) { VCI_CMD_FSM = CMD_IDLE; }
}
break;

case CMD_DATA_WRITE :
if (VCI.CMDACK == true) {
	if ((DATA_FIFO.rok() == false) ||
	   ((int)TYPE_FIFO.read() != DCACHE_CMD_TYPE) ||
           (((int)ADDR_FIFO.read() & 0xFFFFF000) != (DCACHE_CMD_ADDR & 0xFFFFF000))) {
		fifo_get = false;
		VCI_CMD_FSM = CMD_IDLE;
	} else {
		fifo_get = true;
		DCACHE_CMD_DATA   = (int)DATA_FIFO.read();
		DCACHE_CMD_ADDR   = (int)ADDR_FIFO.read();
		DCACHE_CMD_TYPE   = (int)TYPE_FIFO.read();
	}
} else {
	fifo_get = false;
}
break;
} // end  switch VCI_CMD_FSM

//////////////////////////////////////////////////////////////////////////
//  DATA_FIFO, ADR_FIFO and TYPE_FIFO
//  These FIFOs are used as a write buffer and contain the requests from
//  the DCACHE controler to the VCI controler.
//  They are controlled by the fifo_put signal (defined by DCACHE_FSM)
//  and the fifo_get signal (defined by VCI_CMD_FSM)
//////////////////////////////////////////////////////////////////////////

if ((fifo_put == true) && (fifo_get == true)) {
	DATA_FIFO.put_and_get((sc_uint<32>)data_fifo);
	ADDR_FIFO.put_and_get((sc_uint<32>)addr_fifo);
	TYPE_FIFO.put_and_get((sc_uint<3>)type_fifo); }
	
if ((fifo_put == true) && (fifo_get != true)) {
	DATA_FIFO.simple_put((sc_uint<32>)data_fifo);
	ADDR_FIFO.simple_put((sc_uint<32>)addr_fifo);
	TYPE_FIFO.simple_put((sc_uint<3>)type_fifo); }

if ((fifo_put != true) && (fifo_get == true)) {
	DATA_FIFO.simple_get();
	ADDR_FIFO.simple_get();
	TYPE_FIFO.simple_get(); }

//////////////////////////////////////////////////////////////////////////
// The VCI_RSP FSM controls the following ressources :
// - VCI_RSP_FSM :
// - ICACHE_MISS_BUF[ICACHE_WORDS]
// - DCACHE_MISS_BUF[DCACHE_WORDS]
// - DCACHE_VAL_BUF[DCACHE_WORDS] set
// - ICACHE_REQ reset
// - CPT_RSP
//
// This FSM is synchronized with the VCI_CMD_FSM, as both FSMs exit the
// IDLE state simultaneously.
// This FSM analyzes the VCI error code and activates the ICACHE.BERR and
// DCACHE.BERR signals during one cycle.
//////////////////////////////////////////////////////////////////////////

switch (VCI_RSP_FSM) {

case RSP_IDLE :
RSP_CPT = 0;
if (VCI_CMD_FSM == CMD_IDLE) {
	if (DATA_FIFO.rok() == true) {
		type_segment = (int)DCACHE.ADR.read() & SEGMENT_TYPE_MASK;
		if (((int)TYPE_FIFO.read()) == DTYPE_RW) 	
			{
				if 	(type_segment == SEGMENT_TYPE_UNCACHED_WORD) 	{ VCI_RSP_FSM = RSP_DATA_UNC;   }
				else if (type_segment == SEGMENT_TYPE_UNCACHED_BURST) 	{ VCI_RSP_FSM = RSP_DATA_BURST; }
				else 						{ VCI_RSP_FSM = RSP_DATA_MISS;  }
			}
		else                                          			{ VCI_RSP_FSM = RSP_DATA_WRITE; }
	} else if (ICACHE_REQ == true) {
		VCI_RSP_FSM = RSP_INS_MISS;
	}
}
break;

case RSP_INS_MISS :
if (VCI.RSPVAL == true) {
	if (RSP_CPT == ICACHE_WORDS) {
		printf("error in soclib_vci_xcache : \n");
		printf("illegal VCI response packet for instruction miss\n");
		sc_stop();
	}
	RSP_CPT = RSP_CPT + 1;
	ICACHE_MISS_BUF[RSP_CPT] = (int)VCI.RDATA.read();
	if ((VCI.REOP == true) && (VCI.RERROR.read() == 0)) {
		VCI_RSP_FSM = RSP_IDLE;
		ICACHE_REQ = false;
		if (RSP_CPT != ICACHE_WORDS - 1) {
			printf("error in soclib_vci_xcache : \n");
			printf("illegal VCI response packet for instruction miss\n");
			sc_stop();
		}
	} else if ((VCI.REOP == true) && (VCI.RERROR.read() != 0)) {
		VCI_RSP_FSM = RSP_INS_ERROR;
	} else if ((VCI.REOP == false) && (VCI.RERROR.read() != 0)) {
		VCI_RSP_FSM = RSP_INS_ERWAIT;
	}
}
break;

case RSP_INS_ERWAIT :
if ((VCI.RSPVAL == true) && (VCI.REOP == true)) { VCI_RSP_FSM = RSP_INS_ERROR; }
break;

case RSP_INS_ERROR :
VCI_RSP_FSM = RSP_IDLE;
ICACHE_REQ = false;
break;

case RSP_DATA_MISS :
if (VCI.RSPVAL == true) {
	if (RSP_CPT == DCACHE_WORDS) {
		printf("error in soclib_vci_xcache : \n");
		printf("illegal VCI response packet for data read miss");
		sc_stop();
	}
	RSP_CPT = RSP_CPT + 1;
	DCACHE_MISS_BUF[RSP_CPT] = (int)VCI.RDATA.read();
	if ((VCI.REOP == true) && (VCI.RERROR.read() == 0)) {
		VCI_RSP_FSM = RSP_IDLE;
		if (RSP_CPT != DCACHE_WORDS - 1) {
			printf("error in soclib_vci_xcache : \n");
			printf("illegal VCI response packet for data read miss");
			sc_stop();
		}
	} else if ((VCI.REOP == true) && (VCI.RERROR.read() != 0)) {
		VCI_RSP_FSM = RSP_DATA_ERROR;
	} else if ((VCI.REOP == false) && (VCI.RERROR.read() != 0)) {
		VCI_RSP_FSM = RSP_DATA_ERWAIT;
	}
}
break;

case RSP_DATA_BURST :
if (VCI.RSPVAL == true) {
	if (RSP_CPT == DCACHE_WORDS) {
		printf("error in soclib_vci_xcache : \n");
		printf("illegal VCI response packet for data read burst");
		sc_stop();
	}
	RSP_CPT = RSP_CPT + 1;
	DCACHE_MISS_BUF[RSP_CPT] = (int)VCI.RDATA.read();
	DCACHE_VAL_BUF[RSP_CPT] = true;
	if ((VCI.REOP == true) && (VCI.RERROR.read() == 0)) {
		VCI_RSP_FSM = RSP_IDLE;
		if (RSP_CPT != DCACHE_WORDS - 1) {
			printf("error in soclib_vci_xcache : \n");
			printf("illegal VCI response packet for data read burst");
			sc_stop();
		}
	} else if ((VCI.REOP == true) && (VCI.RERROR.read() != 0)) {
		VCI_RSP_FSM = RSP_DATA_ERROR;
	} else if ((VCI.REOP == false) && (VCI.RERROR.read() != 0)) {
		VCI_RSP_FSM = RSP_DATA_ERWAIT;
	}
}
break;

case RSP_DATA_WRITE :
if (VCI.RSPVAL == true) {
	if ((VCI.REOP == true) && (VCI.RERROR.read() == 0)) {
		VCI_RSP_FSM = RSP_IDLE;
	} else if ((VCI.REOP == true) && (VCI.RERROR.read() != 0)) {
		VCI_RSP_FSM = RSP_DATA_ERROR;
	} else if ((VCI.REOP != true) && (VCI.RERROR.read() != 0)) {
		VCI_RSP_FSM = RSP_DATA_ERWAIT;
	}
}
break;

case RSP_DATA_UNC :
if (VCI.RSPVAL == true) {
	int index = (DCACHE_MISS_ADDR & DCACHE_XMASK) >> DCACHE_XSHIFT;
	DCACHE_MISS_BUF[index] = (int)VCI.RDATA.read();
	DCACHE_VAL_BUF[index] = true;
	if ((VCI.REOP == true) && (VCI.RERROR.read() == 0)) {
		VCI_RSP_FSM = RSP_IDLE;
	} else if ((VCI.REOP == true) && (VCI.RERROR.read() != 0)) {
		VCI_RSP_FSM = RSP_DATA_ERROR;
	} else if (VCI.REOP == false) {
		printf("error in soclib_vci_xcache : \n");
		printf("illegal VCI response packet for data read uncached");
		sc_stop();
	}
}
break;

case RSP_DATA_ERWAIT :
if ((VCI.RSPVAL == true) && (VCI.REOP == true)) { VCI_RSP_FSM = RSP_DATA_ERROR; }
break;

case RSP_DATA_ERROR :
VCI_RSP_FSM = RSP_IDLE;
break;
} // end switch VCI_RSP_FSM

////////////////////////////////////////////////////////////////////////
//	configuration bus
////////////////////////////////////////////////////////////////////////

switch(CFG_CMD.read()) {
	case BUS_IDLE:
	break;

	case BUS_READ:
	CFG_SHIFT = (CFG_DCACHE_DISABLE ? 0x2: 0x0) + (CFG_ICACHE_DISABLE ? 0x1: 0x0);
	break;
	
	case BUS_SHIFT:
	CFG_SHIFT = ((CFG_SHIFT & 0xFF)>>1) + ((CFG_DATA_IN) ? (1<<7) : 0);
	break;
	
	case BUS_WRITE:
	CFG_DCACHE_DISABLE.write((CFG_SHIFT.read() & 0x2) == 0x2);
  	CFG_ICACHE_DISABLE.write((CFG_SHIFT.read() & 0x1) == 0x1);
	break;
	
} // end switch CFG
}; // end Transition()

///////////////////////////////////////:
//  genMoore method
///////////////////////////////////////

void genMoore()
{

// VCI.RSPACK, ICACHE.BERR & DCACHE.BERR

VCI.RSPACK = true;
ICACHE.BERR = (VCI_RSP_FSM == RSP_INS_ERROR); // (conditionnal expression...)
DCACHE.BERR = (VCI_RSP_FSM == RSP_DATA_ERROR); //(conditionnal expression...)

// VCI CMD

switch (VCI_CMD_FSM.read()) {

case CMD_IDLE :
VCI.CMDVAL  = false;
#if 0 // DISABLED USELESS SIGNALS
VCI.ADDRESS = 0;
VCI.WDATA   = 0;
VCI.BE      = 0;
VCI.PLEN    = 0;
#endif
VCI.CMD     = 0;
#if 0 // DISABLED USELESS SIGNALS
VCI.TRDID   = 0;
VCI.PKTID   = 0;
VCI.SRCID   = 0;
VCI.CONS    = false;
VCI.WRAP    = false;
VCI.CONTIG  = false;
VCI.CLEN    = 0;
VCI.CFIXED  = false;
#endif
VCI.EOP     = false;
break;

case CMD_DATA_WORD :
VCI.CMDVAL = true;
VCI.ADDRESS = (sc_uint<32>)(DCACHE_CMD_ADDR & (~0x3));
#if 0 // DISABLED USELESS SIGNALS
VCI.WDATA = 0;
#endif
VCI.BE = 0xF;
VCI.PLEN = 4;
VCI.CMD = VCI_CMD_READ;
VCI.TRDID  = 0;
VCI.PKTID  = READ_PKTID;
VCI.SRCID  = (sc_uint<SRCIDSIZE>)IDENT;
VCI.CONS   = false;
VCI.WRAP   = false;
VCI.CONTIG = false;
VCI.CLEN   = 0;
VCI.CFIXED = false;
VCI.EOP    = true;
break;

case CMD_DATA_WRITE :
VCI.CMDVAL = true;
if ((DCACHE_CMD_TYPE & 0x3) == 0) {  /* write WORD */
	VCI.ADDRESS = (sc_uint<32>)DCACHE_CMD_ADDR;
	VCI.WDATA   = (sc_uint<32>)DCACHE_CMD_DATA;
	VCI.BE      = 0xF;
}
if ((DCACHE_CMD_TYPE & 0x3) == 1) {  /* write HALF */
      	VCI.ADDRESS = (sc_uint<32>)DCACHE_CMD_ADDR;
        VCI.WDATA   = (sc_uint<32>)DCACHE_CMD_DATA;
        VCI.BE      = 0x3;
}
if ((DCACHE_CMD_TYPE & 0x2) == 2) {  /* write BYTE*/
      	VCI.ADDRESS = (sc_uint<32>)DCACHE_CMD_ADDR;
        VCI.WDATA   = (sc_uint<32>)DCACHE_CMD_DATA;
        VCI.BE      = 0x1;
}
VCI.PLEN   = 0;
VCI.CMD    = VCI_CMD_WRITE;
VCI.TRDID  = 0;
VCI.PKTID  = WRITE_PKTID;
VCI.SRCID  = (sc_uint<SRCIDSIZE>)IDENT;
VCI.CONS   = false;
VCI.WRAP   = false;
VCI.CONTIG = false;
VCI.CLEN   = 0;
VCI.CFIXED = false;
if ((DATA_FIFO.rok() == false) ||
  ((int)TYPE_FIFO.read() != DCACHE_CMD_TYPE) ||
  (((int)ADDR_FIFO.read() & 0xFFFFF000) != (DCACHE_CMD_ADDR & 0xFFFFF000))) { VCI.EOP = true; }
else { VCI.EOP = false; }
break;

case CMD_DATA_LINE :
VCI.CMDVAL = true;
VCI.ADDRESS = (sc_uint<32>)((DCACHE_CMD_ADDR & (~(DCACHE_XMASK))) + (CMD_CPT << 2));
#if 0 // DISABLED USELESS SIGNALS
VCI.WDATA  = 0;
#endif
VCI.BE     = 0xF;
VCI.PLEN   = 0;
VCI.CMD    = VCI_CMD_READ;
VCI.TRDID  = 0;
VCI.PKTID  = READ_PKTID;
VCI.SRCID  = (sc_uint<SRCIDSIZE>)IDENT;
VCI.CONS   = false;
VCI.WRAP   = false;
VCI.CONTIG = true;
VCI.CLEN   = 0;
VCI.CFIXED = false;
VCI.EOP = (CMD_CPT == DCACHE_WORDS - 1);
break;

case CMD_INS_MISS :
VCI.CMDVAL = true;
VCI.ADDRESS = (sc_uint<32>)((ICACHE_MISS_ADDR & (~(ICACHE_XMASK))) + (CMD_CPT << 2));
#if 0 // DISABLED USELESS SIGNALS
VCI.WDATA  = 0;
#endif
VCI.BE     = 0xF;
VCI.PLEN   = 0;
VCI.CMD    = VCI_CMD_READ;
VCI.TRDID  = 0;
VCI.PKTID  = READ_PKTID;
VCI.SRCID  = (sc_uint<SRCIDSIZE>)IDENT;
VCI.CONS   = false;
VCI.WRAP   = false;
VCI.CONTIG = true;
VCI.CLEN   = 0;
VCI.CFIXED = false;
VCI.EOP = (CMD_CPT == ICACHE_WORDS - 1);
break;

} // end switch VCI_CMD_FSM

// configuration bus

if ((CFG_SHIFT & 0x01) == 0x01)		CFG_DATA_OUT = true;
else					CFG_DATA_OUT = false;

}; // end genMoore()

//////////////////////////////////////////////////////////////////////////////////
//   genMealy method
//
// The Mealy signals are ICACHE.INS, ICACHE.FRZ, DCACHE.FRZ, DCACHE.RDATA
//
// DCACHE
// The processor requests are taken into account only in the DCACHE_IDLE
// and DCACHE_WRITE_REQ states.
// The DCACHE.FRZ signal is activated only if there is a processor
// request, and depends on the DCACHE FSM states:
// - In the IDLE state, DCACHE.FRZ is true when there is a cached read miss,
//   or an uncached read miss.
// - In the WRITE_REQ state, DCACHE.FRZ is true when there is a cached read miss,
//   or an uncached read miss, or when the DATA_FIFO is full.
// - DCACHE.FRZ is true in all other states.
// The DCACHE.RDATA signal is read in the DCACHE_MISS_BUF buffer in case
// of an uncached read (simple word or burst), and is read in the cache
// in all other cases.
//
// ICACHE
// The ICACHE.FRZ signal is activated only if there is a processor
// request, and depends on the ICACHE FSM states:
// In the IDLE state, the ICACHE.FRZ signal depends on the directory comparison,
// and the ICACHE.INS value is the cache or buffer content.
// In all others states, the ICACHE.FRZ signal is always true.
// and the ICACHE.INS value is 0.
///////////////////////////////////////////////////////////////////////////////////

void genMealy()
{
int  icache_address;
int  dcache_address;
bool dcache_miss;
bool dcache_buf_miss;
int  x;
int  y;
int  z;
int  type_segment;
//  ICACHE.FRZ & ICACHE.INS

if(ICACHE.REQ == true) {
	if (ICACHE_FSM == ICACHE_IDLE) {
		icache_address = (int)ICACHE.ADR.read();
		x = (icache_address & ICACHE_XMASK) >> ICACHE_XSHIFT;
		y = (icache_address & ICACHE_YMASK) >> ICACHE_YSHIFT;
		z = (icache_address & ICACHE_ZMASK) >> ICACHE_ZSHIFT;
		if (CFG_ICACHE_DISABLE == false) {  // cached
			if ((int)(z | 0x80000000) == (int)ICACHE_TAG[y]) {
			ICACHE.FRZ = false;
			ICACHE.INS = (sc_uint<32>)ICACHE_DATA[y][x];
		 	} else {
			ICACHE.FRZ = true;
			ICACHE.INS = (sc_uint<32>)ICACHE_DATA[y][x];
			}
		}else {				// uncached
			if ((z | y) == ICACHE_MISS_ADDR) {
			ICACHE.FRZ = false;
			ICACHE.INS = (sc_uint<32>)ICACHE_MISS_BUF[x];
		 	} else {
			ICACHE.FRZ = true;
			ICACHE.INS = (sc_uint<32>)ICACHE_MISS_BUF[x];
			}
		} // end if DISABLE
	} else {
		ICACHE.FRZ = true;
		ICACHE.INS = 0;
	} // end if IDLE
} else {
	ICACHE.FRZ = false;
	ICACHE.INS = 0;
} // end if REQ

// DCACHE.FRZ & DCACHE.RDATA

//  dcache_miss & dcache_buf_miss
dcache_address = (int)DCACHE.ADR.read();
x = (dcache_address & DCACHE_XMASK) >> DCACHE_XSHIFT;
y = (dcache_address & DCACHE_YMASK) >> DCACHE_YSHIFT;
z = (dcache_address & DCACHE_ZMASK) >> DCACHE_ZSHIFT;
if ((int)(z | 0x80000000) == (int)DCACHE_TAG[y]) { dcache_miss = false; }
else 					         { dcache_miss = true; }

if ((dcache_address & ~(DCACHE_XMASK)) == (DCACHE_MISS_ADDR & ~(DCACHE_XMASK)))
	{ dcache_buf_miss = ((DCACHE_VAL_BUF[x] == true) ? false : true); }
else	{ dcache_buf_miss = true; }

if (DCACHE.REQ == true) {
		type_segment = (int)DCACHE.ADR.read() & SEGMENT_TYPE_MASK;
	switch (DCACHE_FSM) {
	case DCACHE_IDLE :
		if        (DCACHE.TYPE.read() == DTYPE_RW)
		{
			if ((type_segment == SEGMENT_TYPE_UNCACHED_WORD)      //RU
			   || (type_segment == SEGMENT_TYPE_UNCACHED_BURST)) {//RB
			DCACHE.FRZ = dcache_buf_miss;
			DCACHE.RDATA = (sc_uint<32>)DCACHE_MISS_BUF[x];
			} else {//RW
 			DCACHE.FRZ = dcache_miss;
			DCACHE.RDATA = (sc_uint<32>)DCACHE_DATA[y][x];
			}
		} else {
			DCACHE.FRZ = false;
			DCACHE.RDATA = (sc_uint<32>)DCACHE_DATA[y][x];
		}
	break;
		
	case DCACHE_WRITE_REQ :
		if       (DATA_FIFO.wok() == false) {
			DCACHE.FRZ = true;
			DCACHE.RDATA = (sc_uint<32>)DCACHE_DATA[y][x];
		} else if (DCACHE.TYPE.read() == DTYPE_RW)
			{
				if ((type_segment == SEGMENT_TYPE_UNCACHED_WORD)      //RU
				   || (type_segment == SEGMENT_TYPE_UNCACHED_BURST)) {//RB
					DCACHE.FRZ = dcache_buf_miss;
					DCACHE.RDATA = (sc_uint<32>)DCACHE_MISS_BUF[x];
				} else {//RW
	 				DCACHE.FRZ = dcache_miss;
					DCACHE.RDATA = (sc_uint<32>)DCACHE_DATA[y][x];
				}
		} else {
				DCACHE.FRZ = false;
				DCACHE.RDATA = (sc_uint<32>)DCACHE_DATA[y][x];
			}
	break;
      	default :
		DCACHE.FRZ = true;
		DCACHE.RDATA = (sc_uint<32>)DCACHE_DATA[y][x];
      	break;
	} // end switch
} else {
        DCACHE.FRZ = false;
	DCACHE.RDATA = (sc_uint<32>)DCACHE_DATA[y][x];
} // end if REQ

};  // end genMealy()
#if defined(STATESAVER_SYSTEM)
    /*virtual implicit*/ void save_state(FILE *fichier)
    {
	    printf("save xcache state\n");
    }
#endif
#if defined(SOCVIEW)
    /*virtual implicit*/ void restore_state(FILE *fichier)
    {
	    printf("restore xcache state\n");
    }

#endif


}; // end structure SOCLIB_VCI_XCACHE

#endif


