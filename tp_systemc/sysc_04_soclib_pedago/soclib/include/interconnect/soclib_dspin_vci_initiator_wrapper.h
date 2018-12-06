/**************************************************************************
 * File : SOCLIB_DSPIN_VCI_INITIATOR_WRAPPER.h
 * Date : 29/02/2004
 * author : Alain Greiner 
 * Copyright : UPMC - LIP6
 *
 * This model describes a VCI master wrapper for the DSPIN  distributed
 * network on chip, respecting the VCI ADVANCED specification.
 *
 * This model can be used by a two level hierarchical interconnected system , only
 * if the DSPIN interconnect is the upper level
 *
 * The DSPIN packet format makes some hypothesis regrading the VCI fields :
 * - the VCI DATA and ADDRESS fields are 32 bits. 
 * - the VCI ERROR field is 1 bit.
 * - the VCI SRCID field is 10 bits.
 * - the VCI TRDID field is 4 bits.
 * - the VCI PKTID field is 2 bits.
 * - Some VCI fields used for optimisation are NOT supported
 * and NOT transmitted in the DSPIN packet :  The PLEN, WRAP, CLEN & CFIXED
 * VCI fiels are forced to 0 by the initiator wrapper. 
 *
 * Because of the hypothesis on the SRCIDSIZE(10 bits with 8 used for the fields X and Y) ,
 * each subsystem (X,Y) cannot contain more than 4 initiators
 *
 * A DSPIN packet is a sequence of 36 bits words.
 * Each DSPIN word contains a 32 bits DATA field and a 4 bits TAGS field.
 * The TAGS field contains 4 flags : EOP, BOP, ERR, PAR.
 * The DSPIN packet format is a varible length sequence of DSPIN words.
 * - REQ_WRITE : Header + N . (Address + WData)
 * - REQ_READ  : Header + N . Address
 * - RSP_WRITE : Header + N . RData
 * - RSP_WRITE : Header + N . RData
 *
 * The DSPIN Address word contains the VCI BE field in the 4 MSB bits.
 * The 4 MSB of the VCI ADDRESS are transmittted in the DSPIN Header. 
 * 
 * The DSPIN request header DATA field has the following format :
 *          |AAAA|GNCC|TTTT|PPSS|SSSS|SSSS|XXXX|YYYY|
 *          
 * The DSPIN response header DATA field has the following format :
 *          |....|....|TTTT|PPSS|SSSS|SSSS|XXXX|YYYY|
 * 
 * - XXXX and YYYY are the destination coordinates in the DSPIN network.
 * - AAAA are the 4 MSB bits of the VCI address.
 * - PP   are the 2 bits of the VCI PKTID field.   
 * - TTTT are the 4 bits of the VCI TRDID field.   
 * - CC are the 2 bits of the VCI CMD field.
 * - SSSSSSSSSS are the 10 bits of the VCI SRCID field. 
 * - G & N are the VCI fields CONTIG & CONS respectively. 
 *
 * The request and response packets are handled by two independant FSMs.
 * The initiator wrapper contains two fifos : the  REQ_FIFO is used to build 
 * the DSPIN request packet, and RSP_FIFO is used to store the incoming
 * DSPIN response packet.
 *
 * The initiator wrapper contains a routing table that decodes the MSB
 * address bits to obtain the destination coordinates (XXXX,YYYY), that are
 * used by the DSPIN network to route the request paquet. 
 * This routing table is implemented as a ROM dynamically allocated by 
 * the constructor.
 * 
 * This component has 8 "template" parameters: 
 * - int  REQ_FIFO_SIZE defines the depth of the request fifo.
 *   It cannot be larger than 256.
 * - int  RSP_FIFO_SIZE defines the depth of the response fifo.
 *   It cannot be larger than 256.
 * and the VCI parameters : 
 *  - ADDRSIZE	  : number of bits of the VCI ADDRESS field    
 *  - CELLSIZE	  : number of bytes the VCI DATA field    
 *  - ERRSIZE	  : number of bits the VCI ERROR field    
 *  - PLENSIZE	  : number of bits the VCI PLEN field    
 *  - CLENSIZE	  : number of bits the VCI CLEN field    
 *  - SRCIDSIZE    : number of bits of the VCI SRCID field
 *  - TRDIDSIZE    : number of bits of the VCI TRDID field
 *  - PKTIDSIZE    : number of bits of the VCI PKTID field
 *
 * This component has 2 "constructor" parameters:
 * - char* insname is the instance name
 * - segmap is a reference to the segment table 
 *
***************************************************************************/

#ifndef SOCLIB_DSPIN_VCI_INITIATOR_WRAPPER_H
#define SOCLIB_DSPIN_VCI_INITIATOR_WRAPPER_H

#include <systemc.h>
#include "interconnect/soclib_dspin_interfaces.h"
#include "shared/soclib_vci_interfaces.h"
#include "shared/soclib_generic_fifo.h"
#include "shared/soclib_segment_table.h"

template <int REQ_FIFO_SIZE, 
	  int RSP_FIFO_SIZE,
	  int ADDRSIZE,
	  int CELLSIZE,
	  int ERRSIZE,
	  int PLENSIZE,
	  int CLENSIZE,
	  int SRCIDSIZE,
	  int TRDIDSIZE,
	  int PKTIDSIZE>

	  
struct SOCLIB_DSPIN_VCI_INITIATOR_WRAPPER : sc_module {

// EXTERNAL PORTS
sc_in<bool>     CLK;
sc_in<bool>     RESETN;
DSPIN_OUT<36>	DSPINOUT;
DSPIN_IN<36>	DSPININ;

ADVANCED_VCI_TARGET<ADDRSIZE,
		CELLSIZE,
		ERRSIZE,
		PLENSIZE,
		CLENSIZE,
		SRCIDSIZE,
		TRDIDSIZE,
		PKTIDSIZE>	VCI;

// STRUCTURAL PARAMETERS 
unsigned  int*	ROUTING_TABLE;	// routing table (2**MSB_NUMBER entries)
const char* 	NAME;
int		MSB_NUMBER;
int 	   LSB_NUMBER;
int		SRCID_MASK;

// REGISTERS
soclib_generic_fifo<REQ_FIFO_SIZE, 36>	REQ_FIFO;
soclib_generic_fifo<RSP_FIFO_SIZE, 36>	RSP_FIFO;

sc_signal<int>			REQ_FSM;
sc_signal<int>			RSP_FSM;
sc_signal<int>			BUF_RSRCID;
sc_signal<int>			BUF_RPKTID;
sc_signal<int>			BUF_RTRDID;

// REQ_FSM STATES
enum{
REQ_HEADER      	= 0,
REQ_ADDRESS_READ	= 1,
REQ_ADDRESS_WRITE	= 2,
REQ_DATA_WRITE		= 3
};

// RSP_FSM STATES
enum{
RSP_HEADER		= 0,
RSP_DATA		= 1,
};

// DSPIN Flags
enum{
	DSPIN_CONS	= 0x04000000,
	DSPIN_CONTIG	= 0x08000000
};

//parity fonction
  bool parity(int val){
    int tmp = 0;
    for (int i = 0; i < 32; i++){
      tmp = tmp + (val >> i);
    }
    return (tmp & 1 == 1);
  }

/////////////////////////////////////////////
// 	Transition()       
/////////////////////////////////////////////
 
void Transition()       
{

sc_uint<36>	req_fifo_data;
bool		req_fifo_write;
bool		req_fifo_read;
sc_uint<36>	rsp_fifo_data;
bool		rsp_fifo_write;
bool		rsp_fifo_read;

if(RESETN == false) { 
	RSP_FIFO.init();
	REQ_FIFO.init();
	REQ_FSM = REQ_HEADER;
	RSP_FSM = RSP_HEADER;
	return;
} 
	
// VCI request to DSPIN request
// The VCI packet is analysed, translated,
// and the DSPIN packet is stored in the REQ_FIFO

	// req_fifo_read
	req_fifo_read = DSPINOUT.READ.read();

	// REQ_FSM, req_fifo_write and req_fifo_data
	switch(REQ_FSM) {
	case REQ_HEADER :
	if(VCI.CMDVAL == true) {
		req_fifo_write = true;
		int dest = (int)(VCI.ADDRESS.read() >> (ADDRSIZE - MSB_NUMBER - LSB_NUMBER));
		req_fifo_data = (sc_uint<36>) (ROUTING_TABLE[dest])              |
		       	        (sc_uint<36>) (VCI.ADDRESS.read() & 0xF0000000)  |
		      		(sc_uint<36>) ((VCI.PKTID.read() & 0x03)  << 18) |
		      		(sc_uint<36>) ((VCI.TRDID.read() & 0x0F)  << 20) |
		                (sc_uint<36>) ((VCI.CMD.read())           << 24) |
		                (sc_uint<36>) ((VCI.SRCID.read()& 0x3FF)  << 8)  |
		                (((sc_uint<36>) DSPIN_BOP)                << 32) ;
		if(VCI.CONTIG.read() == true) 
			{ req_fifo_data = req_fifo_data | (sc_uint<36>) DSPIN_CONTIG; } 
		if(VCI.CONS.read() == true) 
			{ req_fifo_data = req_fifo_data | (sc_uint<36>) DSPIN_CONS; } 
		if(parity(req_fifo_data) == true) 
			{ req_fifo_data = req_fifo_data | ((sc_uint<36>) DSPIN_PAR) << 32; } 
		if(REQ_FIFO.wok() == true) {
			if(VCI.CMD.read() == VCI_CMD_WRITE) {REQ_FSM = REQ_ADDRESS_WRITE;} 
			else                                {REQ_FSM = REQ_ADDRESS_READ;} 
		}
	} else {
		req_fifo_write = false;
	}
	break;
	case REQ_ADDRESS_READ :
	req_fifo_write = VCI.CMDVAL;
	if((VCI.CMDVAL == true) && (REQ_FIFO.wok() == true)) {
		req_fifo_data = (sc_uint<36>) (VCI.ADDRESS.read() & 0x0FFFFFFF) |
				(((sc_uint<36>) VCI.BE.read()) << 28)           ;
		if(parity(req_fifo_data) == true) 
			{ req_fifo_data = req_fifo_data | ((sc_uint<36>) DSPIN_PAR) << 32; } 
		if(VCI.EOP == true) {
			req_fifo_data = req_fifo_data | (((sc_uint<36>) DSPIN_EOP) << 32);
			REQ_FSM = REQ_HEADER;
		}
	}
	break;
	case REQ_ADDRESS_WRITE :
	req_fifo_write = VCI.CMDVAL;
	if((VCI.CMDVAL == true) && (REQ_FIFO.wok() == true)) {
		req_fifo_data = (sc_uint<36>) (VCI.ADDRESS.read() & 0x0FFFFFFF) |
				(((sc_uint<36>) VCI.BE.read()) << 28)           ;
		if(parity(req_fifo_data) == true) 
			{ req_fifo_data = req_fifo_data | ((sc_uint<36>) DSPIN_PAR) << 32; } 
		REQ_FSM = REQ_DATA_WRITE;
	}
	break;
	case REQ_DATA_WRITE :
	req_fifo_write = VCI.CMDVAL;
	if((VCI.CMDVAL == true) && (REQ_FIFO.wok() == true)) {
		req_fifo_data = (sc_uint<36>) VCI.WDATA.read();
		if(parity(req_fifo_data) == true) 
			{ req_fifo_data = req_fifo_data | ((sc_uint<36>) DSPIN_PAR) << 32; } 
		if(VCI.EOP == true) {
			req_fifo_data = req_fifo_data | (((sc_uint<36>) DSPIN_EOP) << 32);
			REQ_FSM = REQ_HEADER;
		}else{ 
			REQ_FSM=REQ_ADDRESS_WRITE;
		}
	}
	break;
	} // end switch REQ_FSM

	// REQ_FIFO
	if((req_fifo_write == true) && (req_fifo_read == false)) { REQ_FIFO.simple_put(req_fifo_data); } 
	if((req_fifo_write == true) && (req_fifo_read == true))  { REQ_FIFO.put_and_get(req_fifo_data); } 
	if((req_fifo_write == false) && (req_fifo_read == true)) { REQ_FIFO.simple_get(); }

// DSPIN response to VCI response
// The DSPIN packet is stored in the RSP_FIFO
// The FIFO output is analysed and translated to a VCI packet
	
	// rsp_fifo_write, rsp_fifo_data
	rsp_fifo_write = DSPININ.WRITE.read();
	rsp_fifo_data  = DSPININ.DATA.read();

	// RSP_FSM, BUF_RPKTID, rsp_fifo_read
	switch(RSP_FSM) {
	case RSP_HEADER :
		rsp_fifo_read = true;
		if(RSP_FIFO.rok() == true) {
			BUF_RSRCID = (int)((RSP_FIFO.read() >>8     ) & SRCID_MASK);
			BUF_RPKTID = (int)((RSP_FIFO.read() >> 18) & 0x03);
			BUF_RTRDID = (int)((RSP_FIFO.read() >> 20) & 0x0F);
			RSP_FSM = RSP_DATA;
		}
	break;
	case RSP_DATA :
		rsp_fifo_read = VCI.RSPACK;
		if((RSP_FIFO.rok() == true) && (VCI.RSPACK == true)) {
			if(((RSP_FIFO.read() >> 32) & DSPIN_EOP) == DSPIN_EOP) { RSP_FSM = RSP_HEADER; }
		}
	break;
	} // end switch RSP_FSM
			
	// RSP_FIFO
	if((rsp_fifo_write == true) && (rsp_fifo_read == false)) { RSP_FIFO.simple_put(rsp_fifo_data); } 
	if((rsp_fifo_write == true) && (rsp_fifo_read == true))  { RSP_FIFO.put_and_get(rsp_fifo_data); } 
	if((rsp_fifo_write == false) && (rsp_fifo_read == true)) { RSP_FIFO.simple_get(); }

};  // end Transition()

/////////////////////////////////////////////
// 	GenMoore()       
/////////////////////////////////////////////
 
void GenMoore()       
{
	
// VCI REQ interface

switch(REQ_FSM) {
	case REQ_HEADER :
	VCI.CMDACK = false;
	break;
	case REQ_ADDRESS_READ :
	VCI.CMDACK = REQ_FIFO.wok();
	break;
	case REQ_ADDRESS_WRITE :
	VCI.CMDACK = false;
	break;
	case REQ_DATA_WRITE :
	VCI.CMDACK = REQ_FIFO.wok();
	break;
} // end switch VCI_REQ_FSM

// VCI RSP interface

switch(RSP_FSM) {
        case RSP_HEADER :
	VCI.RSPVAL = false;
	VCI.RDATA = (sc_uint<ADDRSIZE>) 0;
	VCI.RPKTID = (sc_uint<PKTIDSIZE>) 0;
	VCI.RTRDID = (sc_uint<TRDIDSIZE>) 0;
	VCI.RSRCID = (sc_uint<SRCIDSIZE>) 0;
	VCI.RERROR = (sc_uint<ERRSIZE>) 0;
	VCI.REOP   = false;
	break;
	case RSP_DATA :
	VCI.RSPVAL = RSP_FIFO.rok();
	VCI.RDATA = (sc_uint<ADDRSIZE>) (RSP_FIFO.read() & 0xFFFFFFFF);
	VCI.RPKTID = (sc_uint<PKTIDSIZE>)BUF_RPKTID;
	VCI.RTRDID = (sc_uint<TRDIDSIZE>)BUF_RTRDID;
	VCI.RSRCID = (sc_uint<SRCIDSIZE>)BUF_RSRCID;
	if(((RSP_FIFO.read() >> 32) & DSPIN_ERR) == DSPIN_ERR){ VCI.RERROR = 1; }
	else                                                  { VCI.RERROR = 0; }
	if(((RSP_FIFO.read() >> 32) & DSPIN_EOP) == DSPIN_EOP) {VCI.REOP = true;}
	else                                                   {VCI.REOP = false;}
	break;
} // end switch VCI_RSP_FSM

// DSPIN_OUT interface

DSPINOUT.WRITE = REQ_FIFO.rok();
DSPINOUT.DATA = REQ_FIFO.read();

// DSPIN_IN interface

DSPININ.READ = RSP_FIFO.wok();

}; // end GenMoore

////////////////////////////////////////
//       Constructor   
////////////////////////////////////////

SC_HAS_PROCESS(SOCLIB_DSPIN_VCI_INITIATOR_WRAPPER);

SOCLIB_DSPIN_VCI_INITIATOR_WRAPPER(sc_module_name insname, 
				   SOCLIB_SEGMENT_TABLE seg)
{

SC_METHOD(Transition);
sensitive_pos << CLK;

SC_METHOD(GenMoore);
sensitive_neg << CLK;


//Checking VCI parameters


  if(ADDRSIZE != 32) {
      	printf("error in the soclib_dspin_vci_initiator_wrapper : %s\n", NAME);
      	printf("the ADDRSIZE parameter must be 32\n");
        exit(1);        
  }

  if(CELLSIZE != 4) {
      	printf("error in the soclib_dspin_vci_initiator_wrapper: %s\n", NAME);
      	printf("the CELLSIZE parameter must be 4\n");
        exit(1);        
  }

  if(ERRSIZE != 1) {
      	printf("error in the soclib_dspin_vci_initiator_wrapper : %s\n", NAME);
      	printf("the ERRSIZE parameter must be 1\n");
        exit(1);        
  }

  if(SRCIDSIZE > 32) {
      	printf("error in the soclib_dspin_vci_initiator_wrapper  : %s\n", NAME);
      	printf("the SRCIDSIZE parameter cannot be larger than 32\n");
        exit(1);        
  }

  if(TRDIDSIZE > 32) {
      	printf("error in the soclib_dspin_vci_initiator_wrapper  : %s\n", NAME);
      	printf("the TRDIDSIZE parameter cannot be larger than 32\n");
        exit(1);        
  }

  if(PKTIDSIZE > 32) {
      	printf("error in the soclib_dspin_vci_initiator_wrapper  : %s\n", NAME);
      	printf("the PKTIDSIZE parameter cannot be larger than 32\n");
        exit(1);        
  }

// building the ROUTING_TABLE
 
MSB_NUMBER = seg.getMSBNumber();
LSB_NUMBER = seg.getLSBNumber();
SRCID_MASK = 0x7FFFFFFF >> ( 31 - SRCIDSIZE);

ROUTING_TABLE = new unsigned int[1 << (MSB_NUMBER+LSB_NUMBER)];
seg.initRoutingTable(ROUTING_TABLE);

// instance name definition

NAME = (const char*) insname;

printf("Successful Instanciation of SOCLIB_DSPIN_VCI_INITIATOR_WRAPPER : %s\n", NAME);

}; // end constructor

}; // end struct SOCLIB_DSPIN_VCI_INITIATOR_WRAPPER

#endif 
