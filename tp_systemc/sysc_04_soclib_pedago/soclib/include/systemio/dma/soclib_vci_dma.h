/**************************************************************************
 * File : soclib_vci_dma.h
 * Date : 14/08/2003
 * authors : Alain Greiner 
 * modified : 29/11/04 Francois Pecheux 
 * This program is released under the GNU public license
 * Copyright UPMC - LIP6
 *
 * This component is a DMA controler with three VCI ports :
 * - It is a VCI target that can be configured by the main processor.
 *   This port is controlled by a single TARGET_FSM.
 * - It is a VCI initiator, with two separate ports for READ and WRITE
 *   Each port is controlled by two separate request and response FSMs.
 * Both DATA and ADDRESS fields are 32 bits.
 * The source and destination buffers must be aligned on 32 bits 
 * word boundaries.
 * As a target the DMA controler contains  5 memory mapped registers.
 * Only the 4 less significant bits of the VCI address are decoded :
 * - SOURCE	Write	00** (source buffer address : word aligned)
 * - DEST	Write	01** (destination buffer address : word aligned)
 * - NWORDS	Write	10** (buffer size: number of words)
 * - RESET	Write	11** (aknowledge interrupt)
 * - STATUS	Read	**** (status : 0 if successfully completed)
 *   
 * Writing in registers RESET, SOURCE or DEST stops gracefully
 * the ongoing transfer and forces the 4 DMA engines in IDLE state 
 * after few cycle to complete the VCI transactions.
 * Writing in the NWORDS register starts the actual data transfer,
 * with the addresses written in SOURCE and DEST registers, but only
 * when all DMA engines have reach the IDLE state.
 * The DMA controller sets an IRQ when the transfer is completed.
 * Writing in the RESET register is the normal way to acknowledge IRQ.
 *
 * The burst size used by the DMA engine for both READ and WRITE is
 * a structural parameter (defined by the constructor) : BURST is the
 * number of words and cannot be larger than 128.
 *
 * The READ master FSM writes into an internal FIFO, that is read by the 
 * WRITE master FSM. The FIFO depth is 128 words.
***************************************************************************/

#ifndef SOCLIB_VCI_DMA_H
#define SOCLIB_VCI_DMA_H

#include <systemc.h>
#include "shared/soclib_vci_interfaces.h"
#include "shared/soclib_generic_fifo.h"
#include "shared/soclib_mapping_table.h"

template < int BURST,
		int ADDRSIZE,
		int CELLSIZE,
		int ERRSIZE,
		int PLENSIZE,
		int CLENSIZE,
		int SRCIDSIZE,
		int TRDIDSIZE,
		int PKTIDSIZE> struct SOCLIB_VCI_DMA:sc_module
{

// STRUCTURAL PARAMETERS
  const char *NAME;
  int IDENT_INIT_READ;
  int IDENT_INIT_WRITE;

// EXTERNAL PORTS
    sc_in < bool > CLK;
    sc_in < bool > RESETN;
    sc_out < bool > IRQ;
    ADVANCED_VCI_INITIATOR <	ADDRSIZE,
			CELLSIZE,
			ERRSIZE,
			PLENSIZE,
			CLENSIZE,
			SRCIDSIZE,
			TRDIDSIZE,
			PKTIDSIZE> VCI_READ;
    ADVANCED_VCI_INITIATOR <	ADDRSIZE,
			CELLSIZE,
			ERRSIZE,
			PLENSIZE,
			CLENSIZE,
			SRCIDSIZE,
			TRDIDSIZE,
			PKTIDSIZE > VCI_WRITE;
    ADVANCED_VCI_TARGET <	ADDRSIZE,
			CELLSIZE,
			ERRSIZE,
			PLENSIZE,
			CLENSIZE,
			SRCIDSIZE,
			TRDIDSIZE,
			PKTIDSIZE > VCI_TARGET;

// REGISTERS
    soclib_generic_fifo < BURST, 8 * CELLSIZE > FIFO;

    sc_signal < int >SOURCE;	// memory mapped register
    sc_signal < int >DEST;	// memory mapped register
    sc_signal < int >NWORDS;	// memory mapped register
    sc_signal < int >STATUS;	// memory mapped register

    sc_signal <sc_uint<8> > REG_SRCID;

    sc_signal < int >TARGET_FSM;
    sc_signal < int >REQ_READ_FSM;
    sc_signal < int >RSP_READ_FSM;
    sc_signal < int >REQ_WRITE_FSM;
    sc_signal < int >RSP_WRITE_FSM;

    sc_signal < int >SRC_ID;	// source ID
    sc_signal < int >READ_PTR;	// read address
    sc_signal < int >READ_WORD_COUNT;	// counting total number of read words
    sc_signal < int >READ_BURST_COUNT;	// counting the words in a read burst
    sc_signal < int >WRITE_PTR;	// write address
    sc_signal < int >WRITE_WORD_COUNT;	// counting total number of writen words
    sc_signal < int >WRITE_BURST_COUNT;	// counting the words in a write burst
    sc_signal < bool > STOP_READ;	// ongoing read to be stopped
    sc_signal < bool > STOP_WRITE;	// ongoing write to be stopped
    sc_signal < bool > ERROR_READ;	// read error flag
    sc_signal < bool > ERROR_WRITE;	// write error flag

//  MASTER FSM STATES
  enum
  {
    MASTER_IDLE = 1,
    MASTER_OK = 2,
    MASTER_EOB = 3,
    MASTER_IRQ = 0
  };

// TARGET FSM STATES
  enum
  {
    TARGET_IDLE = 0,
    TARGET_RSP = 1,
    TARGET_EOP = 2
  };

// REGISTER MAPPING
  enum
  {
    SOURCE_ADR = 0x0,
    DEST_ADR = 0x4,
    NWORDS_ADR = 0x8,
    RESET_ADR = 0xC
  };

////////////////////////////
// SEQUENTIAL
////////////////////////////
  void transition ()
  {
    int nwords;
    int fifo_data;
    bool fifo_read = false;
    bool fifo_write = false;

    if (RESETN == false)
      {
	FIFO.init ();
	REQ_READ_FSM = MASTER_IDLE;
	RSP_READ_FSM = MASTER_IDLE;
	REQ_WRITE_FSM = MASTER_IDLE;
	RSP_WRITE_FSM = MASTER_IDLE;
	TARGET_FSM = TARGET_IDLE;
	ERROR_READ = false;
	ERROR_WRITE = false;
      }
    else
      {
	// TARGET_FSM , SOURCE , DEST , NWORDS, set STOP_READ and STOP_WRITE
	switch (TARGET_FSM.read ())
	  {
	  case TARGET_IDLE:
	    if (VCI_TARGET.CMDVAL == true)
	      {
		if ((VCI_TARGET.CMD.read () == VCI_CMD_WRITE) &&  ((VCI_TARGET.ADDRESS.read () & 0xC) == SOURCE_ADR))
		  {
		    SOURCE = (int) (VCI_TARGET.WDATA.read () & 0xFFFFFFFC);
#if 1
		    printf ("on ecrit %x dans SOURCE\n",
			    (int) (VCI_TARGET.WDATA.read () & 0xFFFFFFFC));
#endif
		    REG_SRCID = (sc_uint<8>)VCI_TARGET.SRCID.read ();
		    if (VCI_TARGET.EOP == true)
		      {
			TARGET_FSM = TARGET_EOP;
		      }
		    else
		      {
			TARGET_FSM = TARGET_RSP;
		      }
		  }
		else if ((VCI_TARGET.CMD.read () == VCI_CMD_WRITE) &&
		    ((VCI_TARGET.ADDRESS.read () & 0xC) == DEST_ADR))
		  {
		    DEST = (int) (VCI_TARGET.WDATA.read () & 0xFFFFFFFC);
#if 1
		    printf ("on ecrit %x dans DEST\n",
			    (int) (VCI_TARGET.WDATA.read () & 0xFFFFFFFC));
#endif
		    REG_SRCID = (sc_uint<8>)VCI_TARGET.SRCID.read ();
		    if (VCI_TARGET.EOP == true)
		      {
			TARGET_FSM = TARGET_EOP;
		      }
		    else
		      {
			TARGET_FSM = TARGET_RSP;
		      }
		  }
		else if ((VCI_TARGET.CMD.read () == VCI_CMD_WRITE) &&
		    ((VCI_TARGET.ADDRESS.read () & 0xC) == RESET_ADR))
		  {
#if 1
		    printf ("on ecrit dans RESET\n");
#endif
		    // reset un peu simpliste
		    REQ_READ_FSM = MASTER_IDLE ;
		    RSP_READ_FSM = MASTER_IDLE ;
	            REQ_WRITE_FSM = MASTER_IDLE ;
	            RSP_WRITE_FSM = MASTER_IDLE ;

		    REG_SRCID = (sc_uint<8>)VCI_TARGET.SRCID.read ();
		    if (VCI_TARGET.EOP == true)
		      {
			TARGET_FSM = TARGET_EOP;
		      }
		    else
		      {
			TARGET_FSM = TARGET_RSP;
		      }
		  }
		else if ((VCI_TARGET.CMD.read () == VCI_CMD_WRITE) &&
		    ((VCI_TARGET.ADDRESS.read () & 0xC) == NWORDS_ADR))
		  {
		    NWORDS = (int) VCI_TARGET.WDATA.read ();
		    REG_SRCID = (sc_uint<8>)VCI_TARGET.SRCID.read ();
#if 1
		    printf ("on ecrit %x dans NWORDS\n",
			    (int) (VCI_TARGET.WDATA.read ()));
#endif
		    if ((REQ_READ_FSM == MASTER_IDLE) &&
			(RSP_READ_FSM == MASTER_IDLE) &&
			(REQ_WRITE_FSM == MASTER_IDLE) &&
			(RSP_WRITE_FSM == MASTER_IDLE))
		      {
			if (VCI_TARGET.EOP == true)
			  {
			    TARGET_FSM = TARGET_EOP;
			  }
			else
			  {
			    TARGET_FSM = TARGET_RSP;
			  }
		      }
		  }
		else printf("on a reçu une commande dans le DMA\n");

	      }
	    break;
	  case TARGET_RSP:
	    if (VCI_TARGET.RSPACK == true)
	      {
		TARGET_FSM = TARGET_IDLE;
	      }
	    break;
	  case TARGET_EOP:
	    if (VCI_TARGET.RSPACK == true)
	      {
		TARGET_FSM = TARGET_IDLE;
	      }
	    break;
	  }			// end switch TARGET FSM

	/////////////////////////////////////////////////////////////////////////////
	// The four master FSM exit simultaneously the MASTER_IDLE state, 
	// to start a new DMA transfer, when there is a WRITE in the NWORDS register,
	// but only if they are all in the MASTER_IDLE state.
	// After each burst, the four master FSM test independantly the STOP_READ 
	// and STOP_WRITE flags (in the MASTER_EOB state) to stop the ingoing transfer 
	// if requested by a WRITE in one memory mapped register.
	/////////////////////////////////////////////////////////////////////////////

	// REQ_READ FSM, READ_PTR, READ_BURST_COUNT, READ_WORD_COUNT
	switch (REQ_READ_FSM)
	  {
	  case MASTER_IDLE:
	    if ((VCI_TARGET.CMDVAL == true) &&
		(VCI_TARGET.CMD.read () == VCI_CMD_WRITE) &&
		((VCI_TARGET.ADDRESS.read () & 0xC) == NWORDS_ADR) &&
		(RSP_READ_FSM == MASTER_IDLE) &&
		(REQ_WRITE_FSM == MASTER_IDLE) &&
		(RSP_WRITE_FSM == MASTER_IDLE))
	      {
		REQ_READ_FSM = MASTER_OK;
		READ_PTR = SOURCE;
		SRC_ID   = VCI_TARGET.SRCID.read ();
		nwords = (int) VCI_TARGET.WDATA.read ();
		READ_WORD_COUNT = nwords;
		if (nwords < BURST)
		  {
		    READ_BURST_COUNT = nwords;
		  }
		else
		  {
		    READ_BURST_COUNT = BURST;
		  }
	      }
	    break;
	  case MASTER_OK:
	    if (VCI_READ.CMDACK == true)
	      {
		READ_BURST_COUNT = READ_BURST_COUNT - 1;
		READ_WORD_COUNT = READ_WORD_COUNT - 1;
		READ_PTR = READ_PTR + 4;
		if (READ_BURST_COUNT == 1)
		  {
		    REQ_READ_FSM = MASTER_EOB;
		  }
	      }
	    break;
	  case MASTER_EOB:
	    if (RSP_READ_FSM == MASTER_EOB)
	      {
		if (READ_WORD_COUNT == 0)
		  {
		    REQ_READ_FSM = MASTER_IRQ;
		  }
		else
		  {
		    REQ_READ_FSM = MASTER_OK;
		    if (READ_WORD_COUNT < BURST)
		      {
			READ_BURST_COUNT = READ_WORD_COUNT;
		      }
		    else
		      {
			READ_BURST_COUNT = BURST;
		      }
		  }
	      }
	    break;
	  case MASTER_IRQ:
	    break;
	  }			// end switch REQ_READ_FSM

	// RSP_READ_FSM, reset STOP_READ, ERROR_READ, fifo_write and fifo_data
	switch (RSP_READ_FSM)
	  {
	  case MASTER_IDLE:
	    if ((VCI_TARGET.CMDVAL == true) &&
		(VCI_TARGET.CMD.read () == VCI_CMD_WRITE) &&
		((VCI_TARGET.ADDRESS.read () & 0xC) == NWORDS_ADR) &&
		(REQ_READ_FSM == MASTER_IDLE) &&
		(REQ_WRITE_FSM == MASTER_IDLE) &&
		(RSP_WRITE_FSM == MASTER_IDLE))
	      {
		RSP_READ_FSM = MASTER_OK;
		ERROR_READ = false;
	      }
	  case MASTER_OK:
	    if ((VCI_READ.RSPVAL == true) && (FIFO.wok () == true))
	      {
		fifo_write = true;
		fifo_data = (int) VCI_READ.RDATA.read ();
		if (VCI_READ.REOP == true)
		  {
		    RSP_READ_FSM = MASTER_EOB;
		  }
		if (VCI_READ.RERROR.read () != 0)
		  {
		    ERROR_READ = true;
		  }
	      }
	    break;
	  case MASTER_EOB:
	    if (REQ_READ_FSM == MASTER_EOB)
	      {
		if (READ_WORD_COUNT == 0)
		  {
		    RSP_READ_FSM = MASTER_IRQ;
		  }
		else
		  {
		    RSP_READ_FSM = MASTER_OK;
		  }
	      }
	  case MASTER_IRQ:
	    break;
	  }			// end switch RSP_READ_FSM

	// REQ_WRITE FSM, WRITE_PTR, WRITE_BURST_COUNT, WRITE_WORD_COUNT, fifo_read
	switch (REQ_WRITE_FSM)
	  {
	  case MASTER_IDLE:
	    if ((VCI_TARGET.CMDVAL == true) &&
		(VCI_TARGET.CMD.read () == VCI_CMD_WRITE) &&
		((VCI_TARGET.ADDRESS.read () & 0xC) == NWORDS_ADR) &&
		(RSP_READ_FSM == MASTER_IDLE) &&
		(REQ_READ_FSM == MASTER_IDLE) &&
		(RSP_WRITE_FSM == MASTER_IDLE))
	      {
		REQ_WRITE_FSM = MASTER_OK;
		WRITE_PTR = DEST;
		nwords = (int) VCI_TARGET.WDATA.read ();
		WRITE_WORD_COUNT = nwords;
		if (nwords < BURST)
		  {
		    WRITE_BURST_COUNT = nwords;
		  }
		else
		  {
		    WRITE_BURST_COUNT = BURST;
		  }
	      }
	    break;
	  case MASTER_OK:
	    if ((VCI_WRITE.CMDACK == true) && (FIFO.rok () == true))
	      {
		WRITE_BURST_COUNT = WRITE_BURST_COUNT - 1;
		WRITE_WORD_COUNT = WRITE_WORD_COUNT - 1;
		WRITE_PTR = WRITE_PTR + 4;
		fifo_read = true;
		if (WRITE_BURST_COUNT == 1)
		  {
		    REQ_WRITE_FSM = MASTER_EOB;
		  }
	      }
	    break;
	  case MASTER_EOB:
	    if (RSP_WRITE_FSM == MASTER_EOB)
	      {
		if (WRITE_WORD_COUNT == 0)
		  {
		    REQ_WRITE_FSM = MASTER_IRQ;
		  }
		else
		  {
		    REQ_WRITE_FSM = MASTER_OK;
		    if (WRITE_WORD_COUNT < BURST)
		      {
			WRITE_BURST_COUNT = WRITE_WORD_COUNT;
		      }
		    else
		      {
			WRITE_BURST_COUNT = BURST;
		      }
		  }
	      }
	    break;
	  case MASTER_IRQ:
	    break;
	  }			// end switch REQ_WRITE_FSM

	// RSP_WRITE_FSM, ERROR_WRITE, and reset STOP_WRITE
	switch (RSP_WRITE_FSM)
	  {
	  case MASTER_IDLE:
	    if ((VCI_TARGET.CMDVAL == true) &&
		(VCI_TARGET.CMD.read () == VCI_CMD_WRITE) &&
		((VCI_TARGET.ADDRESS.read () & 0xC) == NWORDS_ADR) &&
		(REQ_READ_FSM == MASTER_IDLE) &&
		(RSP_READ_FSM == MASTER_IDLE) &&
		(REQ_WRITE_FSM == MASTER_IDLE))
	      {
		RSP_WRITE_FSM = MASTER_OK;
		ERROR_WRITE = false;
	      }
	  case MASTER_OK:
	    if (VCI_WRITE.RSPVAL == true)
	      {
		if (VCI_WRITE.RERROR.read () != 0)
		  {
		    ERROR_WRITE = true;
		  }
		if (VCI_WRITE.REOP == true)
		  {
		    RSP_WRITE_FSM = MASTER_EOB;
		  }
	      }
	    break;
	  case MASTER_EOB:
	    if (REQ_WRITE_FSM == MASTER_EOB)
	      {
		if (WRITE_WORD_COUNT == 0)
		  {
		    RSP_WRITE_FSM = MASTER_IRQ;
		  }
		else
		  {
		    RSP_WRITE_FSM = MASTER_OK;
		  }
	      }
	  case MASTER_IRQ:
	    break;
	  }			// end switch RSP_WRITE_FSM

	// FIFO
	if ((fifo_read == true) && (fifo_write == true))
	  {
	    FIFO.put_and_get (fifo_data);
	  }
	if ((fifo_read == false) && (fifo_write == true))
	  {
	    FIFO.simple_put (fifo_data);
	  }
	if ((fifo_read == true) && (fifo_write == false))
	  {
	    FIFO.simple_get ();
	  }

      }				// end else RESETN
  }

  void genMoore ()
  {
// VCI READ CMD interface 
    if (REQ_READ_FSM == MASTER_OK)
      {
#if 0
printf("READ_PTR=%x\n",READ_PTR.read());
#endif
	VCI_READ.CMDVAL = true;
	VCI_READ.ADDRESS = READ_PTR.read ();
	VCI_WRITE.WDATA = 0;
	VCI_READ.CMD = VCI_CMD_READ;
	if (READ_BURST_COUNT == 1)
	  {
	    VCI_READ.EOP = true;
	  }
	else
	  {
	    VCI_READ.EOP = false;
	  }
	VCI_READ.BE = 0xF;
	VCI_READ.PLEN = 0;
	VCI_READ.SRCID = (sc_uint<8>)IDENT_INIT_READ;
      }
    else
      {
	VCI_READ.CMDVAL = false;
      }

// VCI READ RSP interface
    if ((RSP_READ_FSM == MASTER_OK) && (FIFO.wok () == true))
      {
	VCI_READ.RSPACK = true;
      }
    else
      {
	VCI_READ.RSPACK = false;
      }

// VCI WRITE CMD interface 
    if ((REQ_WRITE_FSM == MASTER_OK) && (FIFO.rok () == true))
      {
#if 0
printf("WRITE_PTR=%x\n",WRITE_PTR.read());
#endif
	VCI_WRITE.CMDVAL = true;
	VCI_WRITE.ADDRESS = WRITE_PTR.read ();
int data = FIFO.read();
#if 0
fprintf (stderr, "data = ");
for (int i = 0; i < 4; i++)
  fprintf (stderr, "%c", ((char*)&data)[i]);
fprintf (stderr, "\n");
#endif
	VCI_WRITE.WDATA = data; //FIFO.read ();
	VCI_WRITE.CMD = VCI_CMD_WRITE;
	if (WRITE_BURST_COUNT == 1)
	  {
	    VCI_WRITE.EOP = true;
	  }
	else
	  {
	    VCI_WRITE.EOP = false;
	  }
	VCI_WRITE.BE = 0xF;
	VCI_WRITE.PLEN = 0;
	VCI_WRITE.SRCID = (sc_uint<8>)IDENT_INIT_WRITE;
      }
    else
      {
	VCI_WRITE.CMDVAL = false;
      }

// VCI WRITE RSP interface
    if (RSP_WRITE_FSM == MASTER_OK)
      {
	VCI_WRITE.RSPACK = true;
      }
    else
      {
	VCI_WRITE.RSPACK = false;
      }

// IRQ signal
    if (RSP_WRITE_FSM == MASTER_IRQ)
      {
	IRQ = true;
      }
    else
      {
	IRQ = false;
      }

// VCI TARGET interface
    switch (TARGET_FSM)
      {
      case TARGET_IDLE:
	if ((VCI_TARGET.CMDVAL == true) &&
	    (VCI_TARGET.CMD.read () == VCI_CMD_WRITE) &&
	    (VCI_TARGET.ADDRESS.read () & 0xC == NWORDS_ADR))
	  {
	    if ((REQ_WRITE_FSM == MASTER_IDLE) &&
		(RSP_WRITE_FSM == MASTER_IDLE) &&
		(REQ_WRITE_FSM == MASTER_IDLE) &&
		(RSP_WRITE_FSM == MASTER_IDLE))
	      {
		VCI_TARGET.CMDACK = true;
	      }
	    else
	      {
		VCI_TARGET.CMDACK = false;
	      }
	  }
	else
	  {
	    VCI_TARGET.CMDACK = true;
	  }
	VCI_TARGET.RDATA = 0;
	VCI_TARGET.RSPVAL = false;
	VCI_TARGET.RSRCID = 0;
	VCI_TARGET.RERROR = 0;
	VCI_TARGET.REOP = false;
	break;
      case TARGET_RSP:
	VCI_TARGET.CMDACK = false;
	VCI_TARGET.RSPVAL = true;
	if ((ERROR_READ == false) && (ERROR_WRITE == false))
	  {
	    VCI_TARGET.RDATA = 0;
	  }
	if ((ERROR_READ == false) && (ERROR_WRITE == true))
	  {
	    VCI_TARGET.RDATA = 1;
	  }
	if ((ERROR_READ == true) && (ERROR_WRITE == false))
	  {
	    VCI_TARGET.RDATA = 2;
	  }
	if ((ERROR_READ == true) && (ERROR_WRITE == true))
	  {
	    VCI_TARGET.RDATA = 3;
	  }
	VCI_TARGET.RERROR = 0;
	VCI_TARGET.REOP = false;
	VCI_TARGET.RSRCID = (sc_uint<8>)REG_SRCID;
	break;
      case TARGET_EOP:
	VCI_TARGET.CMDACK = false;
	VCI_TARGET.RSPVAL = true;
	if ((ERROR_READ == false) && (ERROR_WRITE == false))
	  {
	    VCI_TARGET.RDATA = 0;
	  }
	if ((ERROR_READ == false) && (ERROR_WRITE == true))
	  {
	    VCI_TARGET.RDATA = 1;
	  }
	if ((ERROR_READ == true) && (ERROR_WRITE == false))
	  {
	    VCI_TARGET.RDATA = 2;
	  }
	if ((ERROR_READ == true) && (ERROR_WRITE == true))
	  {
	    VCI_TARGET.RDATA = 3;
	  }
	VCI_TARGET.RERROR = 0;
	VCI_TARGET.REOP = true;
	VCI_TARGET.RSRCID = (sc_uint<8>)REG_SRCID;
	break;
      }				// end switch TARGET_FSM

  }				// end sequential

////////////////////////////////////////
//       Constructor   
////////////////////////////////////////

  SC_HAS_PROCESS (SOCLIB_VCI_DMA);

  SOCLIB_VCI_DMA (sc_module_name name, int globalWrite, int localWrite, int globalRead, int localRead, int globalTarget,int localTarget,
		  SOCLIB_MAPPING_TABLE mappingTable)
  {
#ifdef NONAME_RENAME
SOURCE.rename("SOURCE");
DEST.rename("DEST");
NWORDS.rename("NWORDS");
STATUS.rename("STATUS");
REG_SRCID.rename("REG_SRCID");
TARGET_FSM.rename("TARGET_FSM");
REQ_READ_FSM.rename("REQ_READ_FSM");
RSP_READ_FSM.rename("RSP_READ_FSM");
REQ_WRITE_FSM.rename("REQ_WRITE_FSM");
RSP_WRITE_FSM.rename("RSP_WRITE_FSM");
SRC_ID.rename("SRC_ID")	;
READ_PTR.rename("READ_PTR");
READ_WORD_COUNT.rename("READ_WORD_COUNT");
READ_BURST_COUNT.rename("READ_BURST_COUNT");
WRITE_PTR.rename("WRITE_PTR");
WRITE_WORD_COUNT.rename("WRITE_WORD_COUNT");
WRITE_BURST_COUNT.rename("WRITE_BURST_COUNT");
STOP_READ.rename("STOP_READ");
STOP_WRITE.rename("STOP_WRITE")	;
ERROR_READ.rename("ERROR_READ");
ERROR_WRITE.rename("ERROR_WRITE");

#endif

    SC_METHOD (transition);
    dont_initialize ();
    sensitive_pos << CLK;
    SC_METHOD (genMoore);
    dont_initialize ();
    sensitive_neg << CLK;
    NAME = (const char *) name;
    IDENT_INIT_READ = (globalRead << mappingTable.getSrcidLSBSize()) | (localRead);
    IDENT_INIT_WRITE = (globalWrite << mappingTable.getSrcidLSBSize()) | (localWrite);
    if ((BURST > 128) || (BURST < 1))
      {
	std::cerr << "Configuration Error in component VCI_DMA_PLUS" << std::endl;
	std::cerr << "BURST must be larger than 0 and no larger than 128" << std::endl;
      }

  };				// end constructor

};				// end struct vci_dma_plus

#endif
