
////////////////////////////////////////////////////////////////////////////////
// File : soclib_vci_ramlocks.h
// Author : Alain Greiner
// Date : 18/03/2004
// This program is released under the GNU Public License
// Copyright : UPMC-LIP6
//
// This component implements a VCI ADVANCED ram for binary locks : 
// - All read requests (both simple read and linked read) are considered 
// as "set" : The read value is returned, and the memory cell is set to 1.
// - All write requests are considered as "reset" : the memory cell is set to 0.
// 
// The VCI ADDRESS & DATA fiels are 32 bits.
// The VCI ERROR field is 1 bit.
// The VCI ID fields are 8 bits each.
//
// This component contains a single segment defined by
// a BASE address and a SIZE.
// Both the BASE and SIZE must be multiple of 4 bytes.
//
// This component cheks address for segmentation violation,
// and can be used as a default target.
//
/////////////////////////////////////////////////////////////////////////
// This component has 3 "generator" parameters
// - char 			*name   : instance name
// - int			index   : VCI target Index
// - SOCLIB_SEGMENT_TABLE	*segmap : pointer on the segment table
////////////////////////////////////////////////////////////////////////
// This component has 8 "template" parameters :
// [VCI_PARAM]
/////////////////////////////////////////////////////////////////////////

#ifndef SOCLIB_VCI_RAMLOCKS_H
#define SOCLIB_VCI_RAMLOCKS_H

#include <systemc.h>
#include "shared/soclib_vci_interfaces.h"
#include "shared/soclib_mapping_table.h"

////////////////////////////////////////        
//	structure definition
////////////////////////////////////////

template <VCI_PARAM_DECLAR>

struct SOCLIB_VCI_RAMLOCKS : sc_module {

// IO PORTS

sc_in<bool> 			CLK;
sc_in<bool> 			RESETN;
ADVANCED_VCI_TARGET<VCI_PARAM> 	VCI;

//  REGISTERS

sc_signal<int>			FIFO_STATE;
sc_signal<int>			FIFO_RDATA[2];
sc_signal<int>			FIFO_RERROR[2];
sc_signal<int>			FIFO_REOP[2];
sc_signal<int>			FIFO_RSRCID[2];
sc_signal<int>			FIFO_RTRDID[2];
sc_signal<int>			FIFO_RPKTID[2];

//  STRUCTURAL PARAMETERS

const char 			*NAME , *segNAME;		// instance name
int 				SIZE;		// segment size
int 				BASE;		// segment base
int 	 			*RAM;		// segment buffer

////////////////////////////////////////////////////
//	constructor
////////////////////////////////////////////////////

SC_HAS_PROCESS(SOCLIB_VCI_RAMLOCKS);

SOCLIB_VCI_RAMLOCKS (sc_module_name       insname ,  // instance name 
  		     unsigned int	  index,     // VCI target index
		     SOCLIB_MAPPING_TABLE segtab)    // segment table pointer
{
#ifdef NONAME_RENAME
                FIFO_STATE.rename("FIFO_STATE");
        char newname[100];
        for (int i=0; i<2; i++ ) {
                sprintf(newname, "FIFO_RDATA_%2.2d", i);
                FIFO_RDATA[i].rename(newname);
                sprintf(newname, "FIFO_RERROR_%2.2d", i);
                FIFO_RERROR[i].rename(newname);
                sprintf(newname, "FIFO_REOP_%2.2d", i);
                FIFO_REOP[i].rename(newname);
                sprintf(newname, "FIFO_RSRCID_%2.2d", i);
                FIFO_RSRCID[i].rename(newname);
                sprintf(newname, "FIFO_RTRDID_%2.2d", i);
                FIFO_RTRDID[i].rename(newname);
                sprintf(newname, "FIFO_RPKTID_%2.2d", i);
                FIFO_RPKTID[i].rename(newname);
        }

#endif
  SC_METHOD (transition);
  sensitive_pos << CLK;

  SC_METHOD (genMoore);
  sensitive_neg << CLK;

  NAME = (const char*) insname;
  
  // segment allocation
  
  std::list<SEGMENT_TABLE_ENTRY> seglist = segtab.getSegmentList(index);
  BASE = (*seglist.begin()).getBase();
  SIZE = (*seglist.begin()).getSize();
  segNAME = (*seglist.begin()).getName();

  if((BASE & 0x00000003) != 0x0) {
	  printf("Error in component SOCLIB_VCI_RAMLOCKS %s\n", NAME);
	  printf("The BASE address must be word aligned\n");
	  sc_stop();
  }
  if((SIZE & 0x00000003) != 0x0) {
	  printf("Error in component SOCLIB_VCI_RAMLOCKS %s\n", NAME);
	  printf("The SIZE parameter must be multiple of 4\n");
	  sc_stop();
  }

  RAM = new int[SIZE >> 2] ;
  
// checking the VCI parameters

if (ADDRSIZE != 32) {
	perror(" Error in TTY component : the ADDRSIZE parameter\n");
	perror(" must be equal to 32\n");
	sc_stop(); 
}

if (CELLSIZE != 4) {
	perror(" Error in TTY component : the CELLSIZE parameter\n");
	perror(" must be equal to 4\n");
	sc_stop(); 
}

if (ERRSIZE != 1) {
	perror(" Error in TTY component : the ERRSIZE parameter\n");
	perror(" must be equal to 1\n");
	sc_stop(); 
}

if (SRCIDSIZE > 32) {
	perror(" Error in TTY component : the SRCIDSIZE parameter\n");
	perror(" cannot be larger than 32\n");
	sc_stop(); 
}

if (TRDIDSIZE > 32) {
	perror(" Error in TTY component : the TRDIDSIZE parameter\n");
	perror(" cannot be larger than 32\n");
	sc_stop(); 
}

if (PKTIDSIZE > 32) {
	perror(" Error in TTY component : the PKTIDSIZE parameter\n");
	perror(" cannot be larger than 32\n");
	sc_stop(); 
}

  printf("Successful Instanciation of SOCLIB_VCI_RAMLOCKS : %s\n",NAME);
  printf ("segmentName : %s \n" ,segNAME);

}; // end constructor

//////////////////////////////////////////////
//	Transition method
//////////////////////////////////////////////

void transition()
{

int    			address;
int    			rdata;
int    			cmd;    
int			rerror;
  
if (RESETN == false) {
  FIFO_STATE = 0;
  for (int i=0;i< (SIZE >>2);i++){
    RAM[i]=0;		// All the memory cells set to 0
  }

return;
} // end RESETN

switch (FIFO_STATE) {
       	
case 0 :
	if (VCI.CMDVAL == true) { 
		address = ((int)VCI.ADDRESS.read()) & 0xFFFFFFFC; 
		cmd     = (int)VCI.CMD.read();
		rdata	= 0;
		rerror 	= 1;
		if ((address >= BASE) && (address <  BASE + SIZE)) { 
			rerror = 0;
			if ((cmd & 0x1) == 0x1) {	// read 
				rdata  = RAM[(address - BASE) >> 2];
				RAM[(address - BASE) >> 2] = 1;
			} else {			// write
				RAM[(address - BASE) >> 2] = 0;
			}
		}
		FIFO_RDATA[0] 	= rdata; 
		FIFO_RERROR[0]	= rerror;
		FIFO_RSRCID[0]	= (int)VCI.SRCID.read();
		FIFO_RTRDID[0]	= (int)VCI.TRDID.read();
		FIFO_RPKTID[0]  = (int)VCI.PKTID.read();
		FIFO_REOP[0]	= (int)VCI.EOP.read();
		FIFO_STATE	= 1;
	} else {
		FIFO_STATE = 0;
	}
break;

case 1 :
	if (VCI.CMDVAL == true) { 
		address = ((int)VCI.ADDRESS.read()) & 0xFFFFFFFC; 
		cmd     = (int)VCI.CMD.read();
		rdata	= 0;
		rerror 	= 1;
		if ((address >= BASE) && (address <  BASE + SIZE)) { 
			rerror = 0;
			if ((cmd & 0x1) == 0x1) {	// read 
				rdata  = RAM[(address - BASE) >> 2];
				RAM[(address - BASE) >> 2] = 1;
			} else {			// write
				RAM[(address - BASE) >> 2] = 0;
			}
		}
		if (VCI.RSPACK == true) { 
			FIFO_RDATA[0] 	= rdata; 
			FIFO_RERROR[0]	= rerror;
			FIFO_RSRCID[0]	= (int)VCI.SRCID.read();
			FIFO_RTRDID[0]	= (int)VCI.TRDID.read();
			FIFO_RPKTID[0]  = (int)VCI.PKTID.read();
			FIFO_REOP[0]	= (int)VCI.EOP.read();
			FIFO_STATE	= 1;
		} else {
			FIFO_RDATA[1] 	= rdata; 
			FIFO_RERROR[1]	= rerror;
			FIFO_RSRCID[1]	= (int)VCI.SRCID.read();
			FIFO_RTRDID[1]	= (int)VCI.TRDID.read();
			FIFO_RPKTID[1]  = (int)VCI.PKTID.read();
			FIFO_REOP[1]	= (int)VCI.EOP.read();
			FIFO_STATE	= 2;
		}
	} else {  //  CMDVAL == false
		if (VCI.RSPACK == true) { 
			FIFO_STATE	= 0;
		} else {
			FIFO_STATE	= 1;
		}
	}
break;

case 2 :
	if (VCI.RSPACK == true) {
		FIFO_RDATA[0] 	= FIFO_RDATA[1]; 
		FIFO_RERROR[0]	= FIFO_RERROR[1];
		FIFO_RSRCID[0]	= FIFO_RSRCID[1];
		FIFO_RTRDID[0]	= FIFO_RTRDID[1];
		FIFO_RPKTID[0]	= FIFO_RPKTID[1];
		FIFO_REOP[0]	= FIFO_REOP[1];
		FIFO_STATE	= 1;
	} else {
		FIFO_STATE	= 2;
	}
break;
	
} // end switch FIFO_STATE

}; // end transition()

/////////////////////////////////////////////////////
//	genMoore() method
/////////////////////////////////////////////////////

void genMoore()
{

switch(FIFO_STATE) {

	case 0 :
		VCI.CMDACK = true;
		VCI.RSPVAL = false;
		VCI.RDATA  = (sc_uint<32>) 0;
		VCI.RERROR = (sc_uint<ERRSIZE  >) 0;
		VCI.RSRCID = (sc_uint<SRCIDSIZE>) 0;
		VCI.RPKTID = (sc_uint<PKTIDSIZE>) 0;
		VCI.RTRDID = (sc_uint<TRDIDSIZE>) 0;
		VCI.REOP   = false;
	break;

	case 1 :
		VCI.CMDACK = true;
		VCI.RSPVAL = true;
		VCI.RDATA  = (sc_uint<32>)FIFO_RDATA[0];
		VCI.RERROR = (sc_uint<ERRSIZE  >) FIFO_RERROR[0];
		VCI.RSRCID = (sc_uint<SRCIDSIZE>) FIFO_RSRCID[0];
		VCI.RPKTID = (sc_uint<PKTIDSIZE>) FIFO_RPKTID[0];
		VCI.RTRDID = (sc_uint<TRDIDSIZE>) FIFO_RTRDID[0];
		VCI.REOP   = (bool) FIFO_REOP[0];
	break;

	case 2 :
		VCI.CMDACK = false;
		VCI.RSPVAL = true;
		VCI.RDATA  = (sc_uint<32>)FIFO_RDATA[0];
		VCI.RERROR = (sc_uint<ERRSIZE  >) FIFO_RERROR[0];
		VCI.RSRCID = (sc_uint<SRCIDSIZE>) FIFO_RSRCID[0];
		VCI.RPKTID = (sc_uint<PKTIDSIZE>) FIFO_RPKTID[0];
		VCI.RTRDID = (sc_uint<TRDIDSIZE>) FIFO_RTRDID[0];
		VCI.REOP   = (bool) FIFO_REOP[0];
	break;

} // end switch FIFO_STATE

}; // end genMoore()

};  // end structure SOCLIB_VCI_RAMLOCKS

#endif

