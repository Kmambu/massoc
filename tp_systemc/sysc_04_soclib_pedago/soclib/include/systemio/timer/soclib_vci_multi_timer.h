
/*******************************************************************************
 * File : soclib_vci_multi_timer.h
 * Date : 08/02/2004
 * Author : Alain Greiner
 * Copyright UPMC-LIP6
 * It is released under the GNU Public License.
 *
 * This component is a generic timer : It contains up to 256 independant,
 * software controled timers.
 * The timer index i is defined by ADDRESS[12:4].
 * Each timer takes 16 bytes in the address space, and is seen 
 * as 4 memory mapped registers :
 *
 * - TIMER_VALUE[i]	: address 0 
 * A read request returns the value contained in TIMER_VALUE[i].
 * A write request sets a new value in TIMER_VALUE[i].
 * 
 * - TIMER_MODE[i] 	: address 4
 * This mode register contains two bits : 
 * bit0 enables the timer if it is 1,
 * bit1 enables interrupts when it is 1.
 * 
 * - TIMER_PERIOD[i]	: address 8
 * A write request writes the WDATA value in the TIMER_PERIOD[i] register, 
 * the TIMER_VALUE[i] register is set to 0, and the WDATA value is also
 * written in TIMER_COUNTER[i]. The MODE[i] register is set to 0.
 * (timer not running / interrupt masked).  
 * A read request returns the value contained in the TIMER_PERIOD[i] register. 
 * 
 * - TIMER_RESETIRQ[i]	: address 12
 * any write request resets the TIMER_IRQ[i] register to false.
 * A read request returns the 0 value if TIMER_IRQ[i] is false.
 *
 * This component has 3 "generator" parameters :
 * - name	instance name
 * - index	VCI index
 * - segtab	pointer on the system segment table.
 *
 * This component has 9 template parameter :
 * - int	NIRQ		: number of independant IRQ driven by this component.
 * - [VCI_PARAM]
********************************************************************************/

#ifndef SOCLIB_VCI_MULTI_TIMER_H
#define SOCLIB_VCI_MULTI_TIMER_H

#include <systemc.h>
#include "shared/soclib_vci_interfaces.h"
#include "shared/soclib_mapping_table.h"



////////////////////////////////////////////////////////////
//	Structure definition
////////////////////////////////////////////////////////////

template<
    int NIRQ,
    VCI_PARAM_DECLAR>

struct SOCLIB_VCI_MULTI_TIMER : sc_module {

//	I/O Ports
	sc_in<bool> CLK;
	sc_in<bool> RESETN;
ADVANCED_VCI_TARGET<VCI_PARAM> 	VCI;
	sc_out<bool> IRQ[NIRQ];
	
//	Structural parameters
	const char 	*NAME;
	int		BASE;
	int		SIZE;
	const char	*segNAME;
 
//	Registers
	sc_signal<short int> TARGET_FSM;
	sc_signal<int>	TIMER_VALUE[NIRQ];
	sc_signal<int>	TIMER_PERIOD[NIRQ];
	sc_signal<int>	TIMER_COUNTER[NIRQ];
	sc_signal<int>	TIMER_MODE[NIRQ];
	sc_signal<bool>	TIMER_IRQ[NIRQ];

	sc_signal<int>	BUF_SRCID;		// save SRCID
	sc_signal<int>	BUF_TRDID;		// save TRDID
	sc_signal<int>	BUF_PKTID;		// save PKTID
	sc_signal<int>	BUF_NUM;		// save selected timer index
	sc_signal<int>	BUF_ADR;		// save selected register address
	sc_signal<int>	BUF_WDATA;		// save data

//	FSM states
	enum{
		TARGET_IDLE           = 0,
		TARGET_ERROR_RSP      = 1,
		TARGET_ERROR_EOP      = 2,
		TARGET_WRITE_RSP      = 3,
		TARGET_WRITE_EOP      = 4,
		TARGET_READ_RSP       = 5,
		TARGET_READ_EOP       = 6
	};

//	register mapping
	enum{
		VALUE_ADDRESS	 	= 0,
		MODE_ADDRESS		= 4,
		PERIOD_ADDRESS		= 8,
		RESETIRQ_ADDRESS  	= 12 
	};

        enum {
	    TIMER_RUNNING = 1,
	    TIMER_IRQ_ENABLED = 2
	};

////////////////////////////////////////////////////////////
//	Constructor
////////////////////////////////////////////////////////////

SC_HAS_PROCESS(SOCLIB_VCI_MULTI_TIMER);

SOCLIB_VCI_MULTI_TIMER(sc_module_name		name,	// instance name
			int			index,	// VCI target index
		     SOCLIB_MAPPING_TABLE &segtab)    // segment table pointer
{
#ifdef NONAME_RENAME
char newname[100];
TARGET_FSM.rename("TARGET_FSM");
for (int i=0;i<NIRQ;i++)
{
	sprintf(newname,"TIMER_VALUE_%2.2d",i);
	TIMER_VALUE[i].rename(newname);
	sprintf(newname,"TIMER_PERIOD_%2.2d",i);
	TIMER_PERIOD[i].rename(newname);
	sprintf(newname,"TIMER_COUNTER_%2.2d",i);
	TIMER_COUNTER[i].rename(newname);
	sprintf(newname,"TIMER_MODE_%2.2d",i);
	TIMER_MODE[i].rename(newname);
	sprintf(newname,"TIMER_IRQ_%2.2d",i);
	TIMER_IRQ[i].rename(newname);
}

BUF_SRCID.rename("BUF_SRCID");              // save SRCID
BUF_TRDID.rename("BUF_TRDID");              // save TRDID
BUF_PKTID.rename("BUF_PKTID");              // save PKTID
BUF_NUM.rename("BUF_NUM");                // save selected timer index
BUF_ADR.rename("BUF_ADR");                // save selected register address
BUF_WDATA.rename("BUF_WDATA");              // save data

#endif
  SC_METHOD (transition);
  sensitive_pos << CLK;
  
  SC_METHOD (genMoore);
  sensitive_neg << CLK;
  
  NAME = (const char*) name;
  
  //checking the vci parameters
  
if (ADDRSIZE != 32) {
	printf(" Error in SOCLIB_VCI_MULTI_TIMER component : %s\n", NAME);
	printf(" the ADDRSIZE parameter\n");
	printf(" must be equal to 32\n");
	sc_stop(); 
}

if (CELLSIZE != 4) {
	printf(" Error in SOCLIB_VCI_MULTI_TIMER component : %s\n", NAME);
	printf(" the CELLSIZE parameter\n");
	printf(" must be equal to 4\n");
	sc_stop(); 
}

if (ERRSIZE != 1) {
	printf(" Error in SOCLIB_VCI_MULTI_TIMER component : %s\n", NAME);
	printf(" the ERRSIZE parameter\n");
	printf(" must be equal to 1\n");
	sc_stop(); 
}

if (SRCIDSIZE > 16) {
	printf(" Error in SOCLIB_VCI_MULTI_TIMER component : %s\n", NAME);
	printf(" the SRCIDSIZE parameter\n");
	printf(" cannot be larger than 16\n");
	sc_stop(); 
}

if (TRDIDSIZE > 32) {
	printf(" Error in SOCLIB_VCI_MULTI_TIMER component : %s\n", NAME);
	printf(" the TRDIDSIZE parameter\n");
	printf(" cannot be larger than 32\n");
	sc_stop(); 
}

if (PKTIDSIZE > 32) {
	printf(" Error in SOCLIB_VCI_MULTI_TIMER component : %s\n", NAME);
	printf(" the PKTIDSIZE parameter\n");
	printf(" cannot be larger than 32\n");
	sc_stop(); 
} 
  // segment definition
  std::list<SEGMENT_TABLE_ENTRY> seglist = segtab.getSegmentList(index);
  BASE = seglist.begin()->getBase();
  SIZE = seglist.begin()->getSize();
  segNAME = seglist.begin()->getName();
  
  
  // checking parameters
  if ((NIRQ < 1) || (NIRQ > 256)) {
	  printf("Error in component SOCLIB_VCI_MULTI_TIMER %s\n", NAME);
	  printf("The NIRQ parameter (number of independant timers) \n");
	  printf("cannot be larger than 256\n");
	  sc_stop();
  }
  if (SIZE < (NIRQ*32)) {
	  printf("Error in component SOCLIB_VCI_MULTI_TIMER %s\n", NAME);
	  printf("The segment SIZE allocated to this component\n");
	  printf("must be equal or larger than 32*NIRQ bytes\n");
	  sc_stop();
  }
  if (BASE % 4) {
	  printf("Error in component SOCLIB_VCI_MULTI_TIMER %s\n", NAME);
	  printf("The BASE address must be multiple of 4\n");
	  sc_stop();
  }

  printf("Successful Instanciation of SOCLIB_VCI_MULTI_TIMER : %s\n", NAME);
  printf ("segmentName : %s \n" ,segNAME);

} // end constructor

////////////////////////////////////////////////////////////
//	transition 
////////////////////////////////////////////////////////////

void transition()
{

if(RESETN == false) 
{
        TARGET_FSM = TARGET_IDLE;
	for(int i = 0 ; i < NIRQ ; i++) {
		TIMER_VALUE[i] = 0;
		TIMER_PERIOD[i] = 0;
		TIMER_COUNTER[i] = 0;
		TIMER_MODE[i] = 0;
		TIMER_IRQ[i] = false;
	}
	return;
}

int	address;	// VCI adress

switch(TARGET_FSM) 
	{
	case TARGET_IDLE :
		if(VCI.CMDVAL) {
			address = (int)VCI.ADDRESS.read() & (~0x3);
			BUF_SRCID = (int)VCI.SRCID.read();
			BUF_TRDID = (int)VCI.TRDID.read();
			BUF_PKTID = (int)VCI.PKTID.read();
			BUF_WDATA = (int)VCI.WDATA.read();
			BUF_NUM   = ((address - BASE) & 0x00000FF0) >> 4;
			BUF_ADR   =  (address - BASE) & 0x0000000F;
			if ((address < BASE) || (address >= (BASE + SIZE))) {
			    TARGET_FSM = (VCI.EOP)?TARGET_ERROR_EOP:TARGET_ERROR_RSP;
			} else if (VCI.CMD.read() == VCI_CMD_WRITE) {
			    TARGET_FSM = (VCI.EOP)?TARGET_WRITE_EOP:TARGET_WRITE_RSP;
			} else {
			    TARGET_FSM = (VCI.EOP)?TARGET_READ_EOP:TARGET_READ_RSP;
			}
		}
	break;
	
	case TARGET_WRITE_RSP :
	case TARGET_WRITE_EOP :
		if (BUF_ADR == VALUE_ADDRESS) {
			TIMER_VALUE[BUF_NUM] = BUF_WDATA;
		} else if (BUF_ADR == RESETIRQ_ADDRESS) {
		    TIMER_IRQ[BUF_NUM] = false;
		} else if (BUF_ADR == MODE_ADDRESS) {
			TIMER_MODE[BUF_NUM] = BUF_WDATA & 0x3;
		} else if (BUF_ADR == PERIOD_ADDRESS) {
			TIMER_PERIOD[BUF_NUM] = BUF_WDATA;
		}
		if (VCI.RSPACK)
		    TARGET_FSM = TARGET_IDLE;
	break;
	
	case TARGET_READ_RSP :
	case TARGET_READ_EOP :
	case TARGET_ERROR_RSP :
	case TARGET_ERROR_EOP :
	    if (VCI.RSPACK)
		TARGET_FSM = TARGET_IDLE;
	break;
	} // end switch TARGET FSM

// Increment TIMER_VALUE[i], decrement TIMER_COUNTER[i] & Set TIMER_IRQ[i]
for(int i = 0 ; i < NIRQ ; i++) { 
	if (TIMER_MODE[i] & TIMER_RUNNING) {
		TIMER_VALUE[i] = TIMER_VALUE[i] + 1;
		if ( TIMER_COUNTER[i] > 0) {
			TIMER_COUNTER[i] = TIMER_COUNTER[i] - 1;
		} else {
			TIMER_COUNTER[i] = TIMER_PERIOD[i];
			TIMER_IRQ[i] = true;
		}
	} // end if timer running
} // end for

} // end transition()

////////////////////////////////////////////////////////////
//	genMoore
////////////////////////////////////////////////////////////

void genMoore()
{

// VCI signals
switch (TARGET_FSM) {
	case TARGET_IDLE:
    VCI.CMDACK = true;
    VCI.RSPVAL = false;
#if 0 // DISABLED USELESS SIGNALS
		VCI.RSRCID  = 0;
		VCI.RTRDID  = 0;
		VCI.RPKTID  = 0;
#endif
		VCI.RERROR  = 0;
#if 0 // DISABLED USELESS SIGNALS
		VCI.RDATA  = 0;
#endif
		VCI.REOP   = false;
	break;
	case TARGET_WRITE_RSP:
		VCI.CMDACK = false;
		VCI.RSPVAL = true;
		VCI.RSRCID  = (sc_uint<SRCIDSIZE>)BUF_SRCID;
		VCI.RTRDID  = (sc_uint<TRDIDSIZE>)BUF_TRDID;
		VCI.RPKTID  = (sc_uint<PKTIDSIZE>)BUF_PKTID;
		VCI.RERROR  = 0;
		VCI.RDATA  = 0;
		VCI.REOP   = false;
	break;
	case TARGET_WRITE_EOP:
		VCI.CMDACK = false;
		VCI.RSPVAL = true;
		VCI.RSRCID  = (sc_uint<SRCIDSIZE>)BUF_SRCID;
		VCI.RTRDID  = (sc_uint<TRDIDSIZE>)BUF_TRDID;
		VCI.RPKTID  = (sc_uint<PKTIDSIZE>)BUF_PKTID;
		VCI.RERROR  = 0;
		VCI.RDATA  = 0;
		VCI.REOP   = true;
	break;
	case TARGET_READ_RSP:
		VCI.CMDACK = false;
		VCI.RSPVAL = true;
		VCI.RSRCID  = (sc_uint<SRCIDSIZE>)BUF_SRCID;
		VCI.RTRDID  = (sc_uint<TRDIDSIZE>)BUF_TRDID;
		VCI.RPKTID  = (sc_uint<PKTIDSIZE>)BUF_PKTID;
		VCI.RERROR  = 0;
		
		if (BUF_ADR == VALUE_ADDRESS)    { VCI.RDATA  = (sc_uint<8*CELLSIZE>)TIMER_VALUE[BUF_NUM]; }
		if (BUF_ADR == PERIOD_ADDRESS)   { VCI.RDATA  = (sc_uint<8*CELLSIZE>)TIMER_PERIOD[BUF_NUM]; }
		if (BUF_ADR == MODE_ADDRESS)     { VCI.RDATA  = (sc_uint<8*CELLSIZE>)(TIMER_MODE[BUF_NUM] & 0x3); }
		if (BUF_ADR == RESETIRQ_ADDRESS) { VCI.RDATA  = (sc_uint<8*CELLSIZE>)(TIMER_IRQ[BUF_NUM]); }
		VCI.REOP   = false;
	break;
	case TARGET_READ_EOP:
		VCI.CMDACK = false;
		VCI.RSPVAL = true;
		VCI.RSRCID  = (sc_uint<SRCIDSIZE>)BUF_SRCID;
		VCI.RTRDID  = (sc_uint<TRDIDSIZE>)BUF_TRDID;
		VCI.RPKTID  = (sc_uint<PKTIDSIZE>)BUF_PKTID;
		VCI.RERROR  = 0;

		if (BUF_ADR == VALUE_ADDRESS)    { VCI.RDATA  = (sc_uint<8*CELLSIZE>)TIMER_VALUE[BUF_NUM]; }
		if (BUF_ADR == PERIOD_ADDRESS)   { VCI.RDATA  = (sc_uint<8*CELLSIZE>)TIMER_PERIOD[BUF_NUM]; }
		if (BUF_ADR == MODE_ADDRESS)     { VCI.RDATA  = (sc_uint<8*CELLSIZE>)(TIMER_MODE[BUF_NUM] & 0x3); }
		if (BUF_ADR == RESETIRQ_ADDRESS) { VCI.RDATA  = (sc_uint<8*CELLSIZE>)(TIMER_IRQ[BUF_NUM]); }
		VCI.REOP   = true;
	break;
	case TARGET_ERROR_RSP:
		VCI.CMDACK = false;
		VCI.RSPVAL = true;
		VCI.RSRCID  = (sc_uint<SRCIDSIZE>)BUF_SRCID;
		VCI.RTRDID  = (sc_uint<TRDIDSIZE>)BUF_TRDID;
		VCI.RPKTID  = (sc_uint<PKTIDSIZE>)BUF_PKTID;
		VCI.RERROR  = 1;
		VCI.RDATA  = 0;
		VCI.REOP   = false;
	break;
	case TARGET_ERROR_EOP:
		VCI.CMDACK = false;
		VCI.RSPVAL = true;
		VCI.RSRCID  = (sc_uint<SRCIDSIZE>)BUF_SRCID;
		VCI.RTRDID  = (sc_uint<TRDIDSIZE>)BUF_TRDID;
		VCI.RPKTID  = (sc_uint<PKTIDSIZE>)BUF_PKTID;
		VCI.RERROR  = 1;
		VCI.RDATA  = 0;
		VCI.REOP   = true;
	break;
} // end switch TARGET FSM

// IRQ[i]
for (int i = 0 ; i < NIRQ ; i++) {
  IRQ[i] = (TIMER_IRQ[i] && ((TIMER_MODE[i] & TIMER_IRQ_ENABLED)));
}

} // end genMoore()

}; // end structure SOCLIB_VCI_MULTI_TIMER


#endif // ifndef SOCLIB_VCI_MULTI_TIMER_H


