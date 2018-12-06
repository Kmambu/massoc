////////////////////////////////////////////////////////////////////////////
// File : soclib_vci_dma.h
// Date : 05/03/2006
// author : Alain Greiner 
// This program is released under the GNU public license
// Copyright UPMC - LIP6
//
// This component is a DMA controler with two VCI ports :
// - It is a VCI target that can be configured by the main processor.
//   This port is controlled by a single TARGET_FSM.
// - It is a VCI initiator.
//   This port is controlled by two separate request and response FSMs.
// Both VCI DATA and ADDESS fields are 32 bits.
// The VCI EEROR field is one bit
//
// The source and destination buffers must be aligned on 32 bits 
// word boundaries.
// As a target the DMA controler contains  5 memory mapped registers.
// Only the 4 less significant bits of the VCI address are decoded :
// - SOURCE	Write	00** (source buffer address : word aligned)
// - DEST	Write	01** (destination buffer address : word aligned)
// - NWORDS	Write	10** (buffer size: number of 32 bits words)
// - RESET	Write	11** (aknowledge interrupt)
// - STATUS	Read	**** (status : 0 if successfully completed)
//   
// Writing in registers RESET, SOURCE or DEST stops gracefully
// the ongoing transfer and forces the 2 DMA engines in IDLE state 
// after few cycle to complete the VCI transaction.
// Writing in the NWORDS register starts the actual data transfer,
// with the addresses written in SOURCE and DEST registers.
// The DMA controller sets an IRQ when the transfer is completed.
// Writing in the RESET register is the normal way to acknowledge IRQ.
//
// The initiator FSM uses an internal buffer to store a burst.
////////////////////////////////////////////////////////////////////////////
// This component has 8 "template" parameter :
// - int BURST		: burst size (cannot be larger than 128)
// - int ADDRSIZE	: number of bits of the VCI ADDRESS field
// - int CELLSIZE	: number of bytes of the VCI DATA field
// - int ERRSIZE	: number of bits of the VCI ERROR field
// - int PLENSIZE	: number of bits of the VCI PLEN field
// - int CLENSIZE	: number of bits of the VCI CLEN field
// - int SRCIDSIZE	: number of bits of the VCI SRCID field
// - int TRDIDSIZE	: number of bits of the VCI TRDID field
// - int PKTIDSIZE	: number of bits of the VCI PKTID field
///////////////////////////////////////////////////////////////////////////
// This component has six "constructor" parameters :
// - char *name : instance name
// - int  init_index : VCI initiator index
// - int  target_index : VCI target index
// - SOCLIB_MAPPING_TABLE maptab : pointer on the mapping table
////////////////////////////////////////////////////////////////////////////

#ifndef SOCLIB_VCI_DMA_H
#define SOCLIB_VCI_DMA_H

#include <systemc.h>
#include "shared/soclib_vci_interfaces.h"
#include "shared/soclib_mapping_table.h"

template<int	BURST,
	 int	ADDRSIZE,
	 int	CELLSIZE,
	 int	ERRSIZE,
	 int	PLENSIZE,
	 int	CLENSIZE,
	 int	SRCIDSIZE,
	 int	TRDIDSIZE,
	 int	PKTIDSIZE>

struct SOCLIB_VCI_DMA : sc_module {

// STRUCTURAL PARAMETERS
const	char			*NAME;
int				INIT_IDENT;
int				TARGET_IDENT;
int				BASE;
int				SIZE;

// EXTERNAL PORTS
sc_in<bool>                  	CLK;
sc_in<bool>                  	RESETN;
sc_out<bool>		    	IRQ;
ADVANCED_VCI_INITIATOR<	ADDRSIZE,
	 		CELLSIZE,
	 		ERRSIZE,
	 		PLENSIZE,
	 		CLENSIZE,
	 		SRCIDSIZE,
	 		TRDIDSIZE,
	 		PKTIDSIZE> VCI_INIT;
ADVANCED_VCI_TARGET<	ADDRSIZE,
	 		CELLSIZE,
	 		ERRSIZE,
	 		PLENSIZE,
	 		CLENSIZE,
	 		SRCIDSIZE,
	 		TRDIDSIZE,
	 		PKTIDSIZE> VCI_TARGET;

// REGISTERS
sc_signal<int>			BUF[128];		// internal buffer

sc_signal<int>      		SOURCE;			// memory mapped register
sc_signal<int>      		DEST;			// memory mapped register
sc_signal<int>      		NWORDS;			// memory mapped register

sc_signal<int>      		TARGET_FSM;
sc_signal<int>      		REQ_FSM;
sc_signal<int>      		RSP_FSM;

sc_signal<int>      		READ_PTR;		// read address
sc_signal<int>      		WRITE_PTR;		// write address
sc_signal<int>      		WORD_COUNT;		// counting the total number of words
sc_signal<int>     		REQ_INDEX;		// counting the words in a REQ burst
sc_signal<int>     		RSP_INDEX;		// counting the words in a RSP burst
sc_signal<bool>     		STOP;			// ongoing transfer to be stopped
sc_signal<bool>     		ERROR_READ;		// read error flag
sc_signal<bool>     		ERROR_WRITE;		// write error flag
sc_signal<int>			BUF_SRCID;		// save SRCID
sc_signal<int>			BUF_TRDID;		// save TRDID
sc_signal<int>			BUF_PKTID;		// save PKTID

//  REQ_FSM STATES
enum {
REQ_IDLE 		= 0,
REQ_READ		= 1,
REQ_WAIT		= 2,
REQ_WRITE		= 3,
REQ_TEST 		= 4,
REQ_IRQ			= 5
};

//  RSP_FSM STATES
enum {
RSP_IDLE 		= 0,
RSP_READ		= 1,
RSP_WAIT		= 2,
RSP_WRITE		= 3,
RSP_TEST 		= 4,
RSP_IRQ			= 5
};

// TARGET FSM STATES
enum{
TARGET_IDLE		= 0,
TARGET_RSP      	= 1,
TARGET_EOP      	= 2,
TARGET_ERROR_RSP	= 3,
TARGET_ERROR_EOP      	= 4
};

// REGISTER MAPPING
enum{
SOURCE_ADR		= 0x0,
DEST_ADR		= 0x4,
NWORDS_ADR		= 0x8,
RESET_ADR		= 0xC
};

////////////////////////////
// 	transition()
////////////////////////////
void transition()
{

if(RESETN == false) {
	REQ_FSM = REQ_IDLE;
 	RSP_FSM = RSP_IDLE;
	TARGET_FSM = TARGET_IDLE;
	STOP = true;
	return;
} 

/////////////////////////////////////////////////////////////
// The Target FSM controls the following registers:
// TARGET_FSM , SOURCE , DEST , NWORDS, STOP
/////////////////////////////////////////////////////////////



switch(TARGET_FSM.read()) {
case TARGET_IDLE :
	if(VCI_TARGET.CMDVAL == true) { 
		int address = VCI_TARGET.ADDRESS.read();
		BUF_SRCID = (int)VCI_TARGET.SRCID.read();
		BUF_TRDID = (int)VCI_TARGET.TRDID.read();
		BUF_PKTID = (int)VCI_TARGET.PKTID.read();
		if((address < BASE) || (address > BASE + SIZE - 1)) {
			if(VCI_TARGET.EOP == true) TARGET_FSM = TARGET_ERROR_EOP;
			else                       TARGET_FSM = TARGET_ERROR_RSP;
		}
   		else if((VCI_TARGET.CMD.read() == VCI_CMD_WRITE) && ((address & 0xC) == SOURCE_ADR)) {
			SOURCE = (int)(VCI_TARGET.WDATA.read() & 0xFFFFFFFC); 
			STOP = true;
			if(VCI_TARGET.EOP == true) {TARGET_FSM = TARGET_EOP;}
			else                       {TARGET_FSM = TARGET_RSP;}
		} 
   		else if((VCI_TARGET.CMD.read() == VCI_CMD_WRITE) && ((address & 0xC) == DEST_ADR)) {
			DEST = (int)(VCI_TARGET.WDATA.read() & 0xFFFFFFFC); 
			STOP = true;
			if(VCI_TARGET.EOP == true) {TARGET_FSM = TARGET_EOP;}
			else                       {TARGET_FSM = TARGET_RSP;}
		} 
   		else if((VCI_TARGET.CMD.read() == VCI_CMD_WRITE) && ((address & 0xC) == RESET_ADR)) {
			STOP = true;
			if(VCI_TARGET.EOP == true) {TARGET_FSM = TARGET_EOP;}
			else                       {TARGET_FSM = TARGET_RSP;}
		}
   		else if((VCI_TARGET.CMD.read() == VCI_CMD_WRITE) && ((address & 0xC) == NWORDS_ADR)) {
			NWORDS = (int)VCI_TARGET.WDATA.read(); 
			STOP = false;
			if(VCI_TARGET.EOP == true) {TARGET_FSM = TARGET_EOP;}
			else                       {TARGET_FSM = TARGET_RSP;}
		} 
		else if(VCI_TARGET.CMD.read() == VCI_CMD_READ) {
			if(VCI_TARGET.EOP == true) {TARGET_FSM = TARGET_EOP;}
			else                       {TARGET_FSM = TARGET_RSP;}
		}
	} 
break;
case TARGET_RSP :
	if(VCI_TARGET.RSPACK == true) { TARGET_FSM = TARGET_IDLE; }
break;
case TARGET_EOP :
	if(VCI_TARGET.RSPACK == true) { TARGET_FSM = TARGET_IDLE; }
break;
case TARGET_ERROR_RSP :
	if(VCI_TARGET.RSPACK == true) { TARGET_FSM = TARGET_IDLE; }
break;
case TARGET_ERROR_EOP :
	if(VCI_TARGET.RSPACK == true) { TARGET_FSM = TARGET_IDLE; }
break;
} // end switch TARGET FSM
	
/////////////////////////////////////////////////////////////////////////////
// The two master FSM exit simultaneously the IDLE state, 
// to start a new DMA transfer, when the STOP is false.
// Soft Reset : After each burst, the two master FSM test the STOP 
// in the TEST state to stop the ingoing transfer if requested. 
// The request FSM controls the following registers :
// REQ FSM, READ_PTR, WRITE_PTR, REQ_INDEX, WORD_COUNT
/////////////////////////////////////////////////////////////////////////////

switch(REQ_FSM) {
case REQ_IDLE :
	if ((STOP == false) && (RSP_FSM == RSP_IDLE)) {
		REQ_FSM = REQ_READ;
		READ_PTR = SOURCE;
		WRITE_PTR = DEST;
		WORD_COUNT = NWORDS;
		if(NWORDS < BURST) REQ_INDEX = NWORDS;
		else               REQ_INDEX = BURST;
	}
break;
case REQ_READ :
	if(VCI_INIT.CMDACK == true) {
		if(REQ_INDEX == 1) REQ_FSM = REQ_WAIT;
		REQ_INDEX = REQ_INDEX -1;
		READ_PTR = READ_PTR + 4;
	}
break;
case REQ_WAIT :
	if(RSP_FSM == RSP_WAIT) {
		REQ_FSM = REQ_WRITE;
		if(WORD_COUNT < BURST) REQ_INDEX = WORD_COUNT;
		else                   REQ_INDEX = BURST;
	}
break;
case REQ_WRITE :
	if(VCI_INIT.CMDACK == true) {
		if(REQ_INDEX == 1) REQ_FSM = REQ_TEST;
		REQ_INDEX = REQ_INDEX - 1;
		WRITE_PTR = WRITE_PTR + 4;
		WORD_COUNT = WORD_COUNT - 1;
	} 
break;
case REQ_TEST :
	if(RSP_FSM == RSP_TEST) {
		if(STOP == true) {
			REQ_FSM = REQ_IDLE;
		} else {
			if(WORD_COUNT < BURST) REQ_INDEX = WORD_COUNT;
			else                   REQ_INDEX = BURST;
			if(WORD_COUNT == 0)    REQ_FSM = REQ_IRQ;
			else		       REQ_FSM = REQ_READ;
		}
	}
break;
case REQ_IRQ :
	if((RSP_FSM == RSP_IRQ) && (STOP == true)) {
		REQ_FSM = REQ_IDLE;
	}
break;
} // end switch REQ_FSM
	
/////////////////////////////////////////////////////////////////////////////
// The two master FSM exit simultaneously the IDLE state, 
// to start a new DMA transfer, when the STOP is false.
// Soft Reset : After each burst, the two master FSM test the STOP 
// in the TEST state to stop the ingoing transfer if requested. 
// The response FSM controls the following registers :
// RSP_FSM, reset STOP, BUF, RSP_INDEX, ERROR_READ, ERROR_WRITE
/////////////////////////////////////////////////////////////////////////////

switch(RSP_FSM) {
case RSP_IDLE :
	if ((STOP == false) && (REQ_FSM == REQ_IDLE)) {
		if(NWORDS < BURST) RSP_INDEX = NWORDS;
		else               RSP_INDEX = BURST;
		RSP_FSM = RSP_READ;
		ERROR_READ = false;
		ERROR_WRITE = false;
	}
break;
case RSP_READ :
	if(VCI_INIT.RSPVAL == true) { 
		BUF[RSP_INDEX] = (int)VCI_INIT.RDATA.read();
		RSP_INDEX = RSP_INDEX - 1;
		if(VCI_INIT.REOP == true) RSP_FSM = RSP_WAIT;
		if(VCI_INIT.RERROR.read() != 0) ERROR_READ = true;
	}
break;
case RSP_WAIT :
	if(REQ_FSM == REQ_WAIT) {
		RSP_FSM = REQ_WRITE;
		if(WORD_COUNT < BURST) RSP_INDEX = WORD_COUNT;
		else                   RSP_INDEX = BURST;
	}
break;
case RSP_WRITE :
	if(VCI_INIT.RSPVAL == true) { 
		RSP_INDEX = RSP_INDEX - 1;
		if(VCI_INIT.REOP == true) RSP_FSM = RSP_TEST;
		if(VCI_INIT.RERROR.read() != 0) ERROR_WRITE = true;
	}
break;
case RSP_TEST :
	if(REQ_FSM == REQ_TEST) {
		if(STOP == true) {
			RSP_FSM = RSP_IDLE;
		} else {
			if(WORD_COUNT < BURST) RSP_INDEX = WORD_COUNT;
			else                   RSP_INDEX = BURST;
			if(WORD_COUNT == 0)    RSP_FSM = RSP_IRQ;
			else		       RSP_FSM = RSP_READ;
		}
	}
break;
case RSP_IRQ :
	if((REQ_FSM == REQ_IRQ) && (STOP == true)) {
		RSP_FSM = RSP_IDLE;
	} 
break;
} // end switch RSP_FSM

}  // end transition

////////////////////////////
// 	genMoore()
////////////////////////////
void genMoore()
{

// VCI INIT interface 

if(REQ_FSM == REQ_READ) {
	VCI_INIT.CMDVAL  = true;
	VCI_INIT.ADDRESS = (sc_uint<32>)READ_PTR;
	VCI_INIT.WDATA  = 0;
	VCI_INIT.CMD     = (sc_uint<2>)VCI_CMD_READ;
	if(REQ_INDEX == 1) VCI_INIT.EOP = true;
	else               VCI_INIT.EOP = false;
	VCI_INIT.BE      = 0xF;
	VCI_INIT.PLEN    = 0;
	VCI_INIT.SRCID   = (sc_uint<8>)INIT_IDENT;
	VCI_INIT.TRDID   = 0;
	VCI_INIT.PKTID   = 0;
	VCI_INIT.CLEN    = 0;
	VCI_INIT.CFIXED  = false;
	VCI_INIT.CONS    = false;
	VCI_INIT.WRAP    = false;
	VCI_INIT.CONTIG  = false;
} else if(REQ_FSM == REQ_WRITE) {
	VCI_INIT.CMDVAL  = true;
	VCI_INIT.ADDRESS = (sc_uint<32>)WRITE_PTR;
	VCI_INIT.WDATA   = (sc_uint<32>)(BUF[REQ_INDEX]);
	VCI_INIT.CMD     = (sc_uint<2>)VCI_CMD_WRITE;
	if(REQ_INDEX == 1) VCI_INIT.EOP = true;
	else               VCI_INIT.EOP = false;
	VCI_INIT.BE      = 0xF;
	VCI_INIT.PLEN    = 0;
	VCI_INIT.SRCID   = (sc_uint<8>)INIT_IDENT;
	VCI_INIT.TRDID   = 0;
	VCI_INIT.PKTID   = 0;
	VCI_INIT.CLEN    = 0;
	VCI_INIT.CFIXED  = false;
	VCI_INIT.CONS    = false;
	VCI_INIT.WRAP    = false;
	VCI_INIT.CONTIG  = false;
} else {
	VCI_INIT.CMDVAL  = false;
}

if(RSP_FSM == RSP_READ) {
	VCI_INIT.RSPACK = true;
} else if(RSP_FSM == RSP_WRITE) {
	VCI_INIT.RSPACK = true;
} else {
	VCI_INIT.RSPACK = false;
}

// IRQ signal
    if (RSP_FSM == RSP_IRQ) 	{ IRQ = true; }
    else 			{ IRQ = false; }

// VCI TARGET interface

switch(TARGET_FSM) {
case TARGET_IDLE :
	VCI_TARGET.CMDACK = true;
	VCI_TARGET.RSPVAL = false; 
break;
case TARGET_RSP :
	VCI_TARGET.CMDACK = false;
	VCI_TARGET.RSPVAL = true; 
	if((ERROR_READ == false) && (ERROR_WRITE == false)) {VCI_TARGET.RDATA  = 0;}
	if((ERROR_READ == false) && (ERROR_WRITE == true )) {VCI_TARGET.RDATA  = 1;}
	if((ERROR_READ == true ) && (ERROR_WRITE == false)) {VCI_TARGET.RDATA  = 2;}
	if((ERROR_READ == true ) && (ERROR_WRITE == true )) {VCI_TARGET.RDATA  = 3;}
	VCI_TARGET.RERROR = 0;
	VCI_TARGET.REOP = false;
break;
case TARGET_EOP :
	VCI_TARGET.CMDACK = false;
	VCI_TARGET.RSPVAL = true;
	VCI_TARGET.RSRCID = (sc_uint<SRCIDSIZE>)BUF_SRCID;
	VCI_TARGET.RTRDID = (sc_uint<TRDIDSIZE>)BUF_TRDID;
	VCI_TARGET.RPKTID = (sc_uint<PKTIDSIZE>)BUF_PKTID;
	if((ERROR_READ == false) && (ERROR_WRITE == false)) {VCI_TARGET.RDATA  = 0;}
	if((ERROR_READ == false) && (ERROR_WRITE == true )) {VCI_TARGET.RDATA  = 1;}
	if((ERROR_READ == true ) && (ERROR_WRITE == false)) {VCI_TARGET.RDATA  = 2;}
	if((ERROR_READ == true ) && (ERROR_WRITE == true )) {VCI_TARGET.RDATA  = 3;}
	VCI_TARGET.RERROR = 0;
	VCI_TARGET.REOP = true;
	break;
case TARGET_ERROR_RSP :
	VCI_TARGET.CMDACK = false;
	VCI_TARGET.RSPVAL = true; 
	if((ERROR_READ == false) && (ERROR_WRITE == false)) {VCI_TARGET.RDATA  = 0;}
	if((ERROR_READ == false) && (ERROR_WRITE == true )) {VCI_TARGET.RDATA  = 1;}
	if((ERROR_READ == true ) && (ERROR_WRITE == false)) {VCI_TARGET.RDATA  = 2;}
	if((ERROR_READ == true ) && (ERROR_WRITE == true )) {VCI_TARGET.RDATA  = 3;}
	VCI_TARGET.RERROR = 1;
	VCI_TARGET.REOP = false;
break;
case TARGET_ERROR_EOP :
	VCI_TARGET.CMDACK = false;
	VCI_TARGET.RSPVAL = true;
	if((ERROR_READ == false) && (ERROR_WRITE == false)) {VCI_TARGET.RDATA  = 0;}
	if((ERROR_READ == false) && (ERROR_WRITE == true )) {VCI_TARGET.RDATA  = 1;}
	if((ERROR_READ == true ) && (ERROR_WRITE == false)) {VCI_TARGET.RDATA  = 2;}
	if((ERROR_READ == true ) && (ERROR_WRITE == true )) {VCI_TARGET.RDATA  = 3;}
	VCI_TARGET.RERROR = 1;
	VCI_TARGET.REOP = true;
	break;
} // end switch TARGET_FSM
	
} // end genMoore

////////////////////////////////////////
//       Constructor   
////////////////////////////////////////

SC_HAS_PROCESS(SOCLIB_VCI_DMA);

SOCLIB_VCI_DMA(sc_module_name 		name, 
		unsigned int 		init_index,
		unsigned int 		target_index,
		SOCLIB_MAPPING_TABLE 	maptab)
{
#ifdef NONAME_RENAME 
  char str[100];
  for (int i=0; i<128; i++) {
	sprintf(str, "BUF_%d", i);
	BUF[i].rename(str);}

  SOURCE.rename("SOURCE");
  DEST.rename("DEST");
  NWORDS.rename("NWORDS");
  TARGET_FSM.rename("TARGET_FSM");
  REQ_FSM.rename("REQ_FSM");
  RSP_FSM.rename("RSP_FSM");
  READ_PTR.rename("READ_PTR");
  WRITE_PTR.rename("WRITE_PTR");
  WORD_COUNT.rename("WORD_COUNT");
  REQ_INDEX.rename("REQ_INDEX");
  RSP_INDEX.rename("RSP_INDEX");
  STOP.rename("STOP");
  ERROR_READ.rename("ERROR_READ");
  ERROR_WRITE.rename("ERROR_WRITE");
  BUF_SRCID.rename("BUF_SRCID");
  BUF_TRDID.rename("BUF_TRDID");
  BUF_PKTID.rename("BUF_PKTID");
#endif


SC_METHOD(transition);
sensitive_pos << CLK;

SC_METHOD(genMoore);
sensitive_neg << CLK;

if((BURST > 128) || (BURST < 1)) {
	cerr << "Configuration Error in component VCI_DMA" << endl;
	cerr << "BURST must be larger than 0 and no larger than 128" << endl;
}

// checking VCI parameters

if(ADDRSIZE != 32) {
        printf("error in the soclib_vci_xcache : %s\n", NAME);
        printf("the ADDRSIZE parameter must be 32\n");
        exit(1);
}

if(CELLSIZE != 4) {
        printf("error in the soclib_vci_xcache : %s\n", NAME);
        printf("the CELLSIZE parameter must be 4\n");
        exit(1);
}

if(ERRSIZE != 1) {
        printf("error in the soclib_vci_xcache : %s\n", NAME);
        printf("the ERRSIZE parameter must be 1\n");
        exit(1);
}

if(SRCIDSIZE > 32) {
        printf("error in the soclib_vci_xcache : %s\n", NAME);
        printf("the SCRIDSIZE parameter cannot be larger than 32\n");
        exit(1);
}

if(TRDIDSIZE > 32) {
        printf("error in the soclib_vci_xcache : %s\n", NAME);
        printf("the TRDIDSIZE parameter cannot be larger than 32\n");
        exit(1);
}

if(PKTIDSIZE > 32) {
        printf("error in the soclib_vci_xcache : %s\n", NAME);
        printf("the PKTIDSIZE parameter cannot be larger than 32\n");
        exit(1);
}

NAME = (const char *) name;

// get BASE address and segment SIZE
std::list<SEGMENT_TABLE_ENTRY>	seglist = maptab.getSegmentList(target_index);
BASE = (*seglist.begin()).getBase();
SIZE = (*seglist.begin()).getSize();

// get VCI INITIATOR IDENT
INIT_IDENT = init_index;

printf("successful Instanciation of SOCLIB_VCI_DMA : %s\n", NAME);

}; // end constructor

}; // end struct SOCLIB_VCI_DMA

#endif
