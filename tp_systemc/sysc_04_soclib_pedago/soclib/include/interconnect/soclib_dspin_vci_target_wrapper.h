/**************************************************************************
 * File : SOCLIB_DSPIN_VCI_TARGET_WRAPPER.h
 * Date : 29/02/2004
 * author : Alain Greiner 
 * Copyright : UPMC - LIP6
 *
 * This model describes a VCI target wrapper for the DSPIN  distributed
 * network on chip, respecting the VCI ADVANCED specification.
 *
 * This model can support a 2 level hierarchical interconnect , if the DSPIN interconnect is 
 * the upper level
 *
 * The DSPIN packet format makes some hypothesis regarding the VCI fiels :
 * - the VCI DATA and ADDRESS fields are 32 bits.
 * - the VCI ERROR field is 1 bit.
 * - the VCI SRCID field is 10 bits.
 * - the VCI PKTID field is 2 bits.
 * - the VCI TRDID field is 4 bits.
 * Some VCI fields used for optimisation are NOT supported
 * and NOT transmitted in the DSPIN packet :  The PLEN, WRAP, CLEN, & CFIXED 
 * VCI fields are forced to 0 by the initiator wrapper.  
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
 *
 * The DSPIN Address word contains the VCI BE field in the 4 MSB bits.
 * (the 4 MSB of the VCI ADDRESS are transmittted in the DSPIN Header. 
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
 * - SSSSSSSSSS are the 10 bits of the VCI SRCID field in case of requests or RSRCID in case of response.
 * - G & N are the VCI fields CONTIG & CONS respectively.    
 *   
 * The request and response packets are handled by two independant FSMs.
 * The request FSM translates the DSPIN requests to a VCI request,
 * using the REQ_FIFO to store the DSPIN packet.
 * The response FSM translates the VCI response to a DSPIN response
 * using the RSP_FIFO to store the DSPIN packet.
 * 
 * The target wrapper does NOT contain a routing table.
 * The destination coordinates (XXXX,YYYY) used by the DSPIN
 * network to route the response packet are directly obtained 
 * from the most significant bits of the RSRCID field.
 * (The LSB bits of the VCI RSRCID field can be used to adress
 * a specific initiator in the sub-system).
 * 
 * This component has 11 "template" parameters: 
 * - int REQ_FIFO_SIZE  defines the depth of the request fifo.
 *   It cannot be larger than 256.
 * - int RSP_FIFO_SIZE  defines the depth of the response fifo.
 *   It cannot be larger than 256.
 *  the VCI parameters :
 * - int	ADDRSIZE,
 * - int	CELLSIZE,
 * - int	ERRSIZE,
 * - int	PLENSIZE,
 * - int	CLENSIZE,
 * - int	SRCIDSIZE,
 * - int	TRDIDSIZE,
 * - int	PKTIDSIZE
 * and the VCI SRCID MSB size to build the X and Y fields of the response 
 * - int SRCIDMSBSIZE
 *   
 * This component has one "constructor" parameter :
 * - char* NAME defines the instance name.
 *
 ***************************************************************************/

#ifndef SOCLIB_DSPIN_VCI_TARGET_WRAPPER_H
#define SOCLIB_DSPIN_VCI_TARGET_WRAPPER_H

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
	  int PKTIDSIZE,
	  int SRCIDMSBSIZE>
	  
struct SOCLIB_DSPIN_VCI_TARGET_WRAPPER : sc_module {

// EXTERNAL PORTS
sc_in<bool>     CLK;
sc_in<bool>     RESETN;
DSPIN_OUT<36>	DSPINOUT;
DSPIN_IN<36>	DSPININ;
ADVANCED_VCI_INITIATOR <ADDRSIZE,
			CELLSIZE,
			ERRSIZE,
			PLENSIZE,
			CLENSIZE,
			SRCIDSIZE,
			TRDIDSIZE,
			PKTIDSIZE>		VCI;

// STRUCTURAL PARAMETERS
const char*				NAME;

// REGISTERS
soclib_generic_fifo<REQ_FIFO_SIZE, 36>  REQ_FIFO;
soclib_generic_fifo<RSP_FIFO_SIZE, 36>	RSP_FIFO;

sc_signal<int>				REQ_FSM;
sc_signal<int>				RSP_FSM;
sc_signal<sc_uint<2> >			BUF_CMD;
sc_signal<sc_uint<CELLSIZE> >			BUF_BE;
sc_signal<sc_uint<SRCIDSIZE> >			BUF_SRCID;
sc_signal<sc_uint<32> >			BUF_MSBAD;
sc_signal<sc_uint<32> >			BUF_LSBAD;
sc_signal<sc_uint<PKTIDSIZE> >			BUF_PKTID;
sc_signal<sc_uint<TRDIDSIZE> >			BUF_TRDID;
sc_signal<bool >			BUF_CONS;
sc_signal<bool >			BUF_CONTIG;

// REQ_FSM STATES
enum{
	REQ_IDLE		= 0,
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
	REQ_FSM = REQ_IDLE;
	RSP_FSM = RSP_HEADER;
	return;
} // end reset
	
// DSPIN request to VCI request 
// The DSPIN packet is written into the REQ_FIFO
// and the FIFO output is analysed and translated

	// req_fifo_write and req_fifo_data
	req_fifo_write = DSPININ.WRITE.read();
	req_fifo_data  = DSPININ.DATA.read();
	
	// REQ_FSM, req_fifo_read, BUF_CMD, BUF_SRCID, BUF_PKTID, BUF_TRDID, BUF_MSBAD, BUF_LSBAD
	switch(REQ_FSM) {
	case REQ_IDLE :
		req_fifo_read = true;
		if(REQ_FIFO.rok() == true) {
			BUF_CMD    = (sc_uint<2>)  ((REQ_FIFO.read() >> 24) & 0x000000003); 
			BUF_SRCID  = (sc_uint<SRCIDSIZE>)  ((REQ_FIFO.read() >> 8)  & 0x0000003FF);
			BUF_MSBAD  = (sc_uint<ADDRSIZE>) ((REQ_FIFO.read())       & 0x0F0000000);
			BUF_PKTID  = (sc_uint<PKTIDSIZE>)  ((REQ_FIFO.read() >> 18) & 0x000000003);
			BUF_TRDID  = (sc_uint<TRDIDSIZE>)  ((REQ_FIFO.read() >> 20) & 0x00000000F);
			if((REQ_FIFO.read() & (sc_uint<36>)DSPIN_CONS) == (sc_uint<36>)DSPIN_CONS) { BUF_CONS = true; } 
			else 									   { BUF_CONS = false; }
			if((REQ_FIFO.read() & (sc_uint<36>)DSPIN_CONTIG) == (sc_uint<36>)DSPIN_CONTIG) { BUF_CONTIG = true; } 
			else 									       { BUF_CONTIG = false; }
			if((sc_uint<2>)(REQ_FIFO.read() >> 24) == VCI_CMD_WRITE) {
				REQ_FSM = REQ_ADDRESS_WRITE; 
			} else {
				REQ_FSM = REQ_ADDRESS_READ; 
			}
		}
		break;
	case REQ_ADDRESS_READ :
		req_fifo_read = VCI.CMDACK.read();
		if((VCI.CMDACK.read() == true) && (REQ_FIFO.rok() == true) && 
			(((REQ_FIFO.read() >> 32) & DSPIN_EOP) == DSPIN_EOP)) 
				{ REQ_FSM = REQ_IDLE; }
		break;
	case REQ_ADDRESS_WRITE :
		req_fifo_read = true;
		if(REQ_FIFO.rok() == true) {
			BUF_BE     = (sc_uint<CELLSIZE>)  (REQ_FIFO.read() >> 28);
			BUF_LSBAD  = (sc_uint<ADDRSIZE>) (REQ_FIFO.read()) & 0x0FFFFFFF;
			REQ_FSM    = REQ_DATA_WRITE;
		}
		break;
	case REQ_DATA_WRITE :
		req_fifo_read = VCI.CMDACK.read();
		if((VCI.CMDACK.read() == true) && (REQ_FIFO.rok() == true)) {
			if(((REQ_FIFO.read() >> 32) & DSPIN_EOP) == DSPIN_EOP) {
				REQ_FSM = REQ_IDLE;
			} else {
				REQ_FSM = REQ_ADDRESS_WRITE;
			}
		}
		break;
	} // end switch REQ_FSM

	// REQ_FIFO
	if((req_fifo_write == true) && (req_fifo_read == false)) { REQ_FIFO.simple_put(req_fifo_data); } 
	if((req_fifo_write == true) && (req_fifo_read == true))  { REQ_FIFO.put_and_get(req_fifo_data); } 
	if((req_fifo_write == false) && (req_fifo_read == true)) { REQ_FIFO.simple_get(); }


// VCI response to DSPIN response 
// The VCI packet is analysed, translated, and
// the SPIN packet is written into the RSP_FIFO
// 
	// rsp_fifo_read 
	rsp_fifo_read  = DSPINOUT.READ.read();

	// RSP_FSM, rsp_fifo_write and rsp_fifo_data
	switch(RSP_FSM) {
	case RSP_HEADER :
		rsp_fifo_write = VCI.RSPVAL.read();
		if((VCI.RSPVAL.read() == true) && (RSP_FIFO.wok() == true)) { 
			rsp_fifo_data = ((sc_uint<36>)VCI.RSRCID.read() >>(SRCIDSIZE - SRCIDMSBSIZE)) |// take only the MSB bits
								(((sc_uint<36>)VCI.RSRCID.read())<<8) |
		                  	(((sc_uint<36>)VCI.RPKTID.read() & 0x03) << 18) |
		                  	(((sc_uint<36>)VCI.RTRDID.read() & 0x0F) << 20) |
  		                  	(((sc_uint<36>)DSPIN_BOP)                <<32)  ; 
			if(parity(rsp_fifo_data) == true) { 
				rsp_fifo_data = rsp_fifo_data | ((sc_uint<36>)DSPIN_PAR << 32); } 
		  RSP_FSM = RSP_DATA; 
		}
	break;
	case RSP_DATA :
		rsp_fifo_write = VCI.RSPVAL.read();
		if((VCI.RSPVAL.read() == true) && (RSP_FIFO.wok() == true)) { 
			rsp_fifo_data = (sc_uint<36>)VCI.RDATA.read(); 
			if(parity(rsp_fifo_data) == true) { 
				rsp_fifo_data = rsp_fifo_data | ((sc_uint<36>)DSPIN_PAR << 32); } 
			if(VCI.RERROR.read() != 0) { 
				rsp_fifo_data = rsp_fifo_data | ((sc_uint<36>)DSPIN_ERR << 32); }
			if(VCI.REOP.read() == true) { 
				rsp_fifo_data = rsp_fifo_data | ((sc_uint<36>)DSPIN_EOP << 32); 
		  		RSP_FSM = RSP_HEADER;
			}
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
	
//  VCI REQ interface

switch(REQ_FSM) {
	case REQ_IDLE :
		VCI.CMDVAL = false;
		break;
	case REQ_ADDRESS_READ :
		VCI.CMDVAL = REQ_FIFO.rok();
		VCI.ADDRESS = ((sc_uint<ADDRSIZE>)(REQ_FIFO.read()) & 0x0FFFFFFF) | (sc_uint<ADDRSIZE>)BUF_MSBAD;
		VCI.BE = (sc_uint<CELLSIZE>)(REQ_FIFO.read() >> 28);
		VCI.CMD = BUF_CMD;
		VCI.WDATA = 0;
		VCI.PKTID = BUF_PKTID;
		VCI.SRCID = BUF_SRCID;
		VCI.TRDID = BUF_TRDID;
		VCI.PLEN = 0;
		VCI.CLEN = 0;
		VCI.CFIXED = false;
		VCI.CONS = BUF_CONS;
		VCI.CONTIG = BUF_CONTIG;
		VCI.WRAP = false;
		if(((int)(REQ_FIFO.read() >> 32) & DSPIN_EOP) == DSPIN_EOP) { VCI.EOP = true; }
		else                                                        { VCI.EOP = false; }
		break;

	case REQ_ADDRESS_WRITE :
		VCI.CMDVAL = false;
		break;

	case REQ_DATA_WRITE :
		VCI.CMDVAL = REQ_FIFO.rok();
		VCI.ADDRESS = (sc_uint<ADDRSIZE>)BUF_LSBAD | (sc_uint<ADDRSIZE>)BUF_MSBAD;
		VCI.BE = BUF_BE;
		VCI.CMD = BUF_CMD;
		VCI.WDATA = (sc_uint<8*CELLSIZE>)(REQ_FIFO.read());
		VCI.PKTID = BUF_PKTID;
		VCI.SRCID = BUF_SRCID;
		VCI.TRDID = BUF_TRDID;
		VCI.PLEN = 0;
		VCI.CLEN = 0;
		VCI.CFIXED = false;
		VCI.CONS = BUF_CONS;
		VCI.CONTIG = BUF_CONTIG;
		VCI.WRAP = false;
		if(((int)(REQ_FIFO.read() >> 32) & DSPIN_EOP) == DSPIN_EOP) { VCI.EOP = true; }
		else                                                        { VCI.EOP = false; }
		break;

} // end switch REQ_FSM

// VCI RSP interface

if((RSP_FSM != RSP_HEADER) && (RSP_FIFO.wok() == true)) {VCI.RSPACK = RSP_FIFO.wok();}
else                                                    {VCI.RSPACK = false;}

// DSPININ interface

DSPININ.READ = REQ_FIFO.wok();

// DSPINOUT interface

DSPINOUT.WRITE = RSP_FIFO.rok();
DSPINOUT.DATA = RSP_FIFO.read();

}; // end GenMoore

////////////////////////////////////////
//       Constructor   
////////////////////////////////////////

SC_HAS_PROCESS(SOCLIB_DSPIN_VCI_TARGET_WRAPPER);

SOCLIB_DSPIN_VCI_TARGET_WRAPPER(sc_module_name insname)

{	
SC_METHOD(Transition);
sensitive_pos << CLK;

SC_METHOD(GenMoore);
sensitive_neg << CLK;

// checking parameters
 
if((REQ_FIFO_SIZE > 256) || (REQ_FIFO_SIZE < 1)) {
	perror("error in VCI_DSPIN_TARGET_WRAPPER\n");
	perror("The REQ_FIFO_SIZE parameter must be larger than 1 and no larger than 256\n");
	exit(1);
}

if((RSP_FIFO_SIZE > 256) || (RSP_FIFO_SIZE < 1)) {
	perror("error in VCI_DSPIN_TARGET_WRAPPER\n");
	perror("The RSP_FIFO_SIZE parameter must be larger than 1 and no larger than 256\n");
	exit(1);
}

//Checking VCI parameters


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
      	printf("the SRCIDSIZE parameter cannot be larger than 32\n");
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
  
  if (SRCIDSIZE< SRCIDMSBSIZE){
      	printf("error in the soclib_vci_xcache : %s\n", NAME);
      	printf("the SRCIDMSBSIZE cannot be larger than the SRCIDSIZE \n");
        exit(1);        
  }

NAME = (const char *) insname;

printf("Successful Instanciation of SOCLIB_DSPIN_VCI_TARGET_WRAPPER : %s\n", NAME);

}; // end constructor

}; // end struct SOCLIB_DSPIN_VCI_TARGET_WRAPPER

#endif 
