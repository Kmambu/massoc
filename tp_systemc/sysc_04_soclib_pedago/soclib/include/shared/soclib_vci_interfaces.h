// -*- c++ -*-
/**************************************************************************************
 * File : soclib_vci_interfaces.h
 * Date : 09/03/2004
 * Authors : A.Greiner and R.Buchmann
 * It is released under the GNU Public License.
 * 
 * This file defines the VCI ports and signals data types that must be used
 * for VCI ports ans signals by all VCI ADVANCED components in the SOCLIB library.
 * The size of all VCI fields that are bit vectors are parameterized by templates.
**************************************************************************************/


#ifndef SOCLIB_VCI_INTERFACES_H
#define SOCLIB_VCI_INTERFACES_H

#define VCI_PARAM_DECLAR int ADDRSIZE,int CELLSIZE,int ERRSIZE,int PLENSIZE,int CLENSIZE,int SRCIDSIZE,int TRDIDSIZE,int PKTIDSIZE
#define VCI_PARAM ADDRSIZE,CELLSIZE,ERRSIZE,PLENSIZE,CLENSIZE,SRCIDSIZE,TRDIDSIZE,PKTIDSIZE

#include <systemc.h>

/***  VCI command mnemonics  ***/

enum {
VCI_CMD_NOP      = 0,
VCI_CMD_READ     = 1,
VCI_CMD_WRITE    = 2,
VCI_CMD_READLOCK = 3
};

/***  VCI ADVANCED signals  ***/

template <int ADDRSIZE,
          int CELLSIZE,
          int ERRSIZE,
	  int PLENSIZE,
	  int CLENSIZE,
	  int SRCIDSIZE,
	  int TRDIDSIZE,
	  int PKTIDSIZE>

class ADVANCED_VCI_SIGNALS : sc_module {
	public:
sc_signal<bool>                    RSPACK;
sc_signal<bool>                    RSPVAL; 
sc_signal<sc_uint<8*CELLSIZE> >    RDATA; 
sc_signal<bool>                    REOP;  
sc_signal<sc_uint<ERRSIZE> >       RERROR; 
sc_signal<sc_uint<SRCIDSIZE>  >    RSRCID;  
sc_signal<sc_uint<TRDIDSIZE>  >    RTRDID;
sc_signal<sc_uint<PKTIDSIZE>  >    RPKTID;
   
sc_signal<bool>                    CMDACK; 
sc_signal<bool>                    CMDVAL;   
sc_signal<sc_uint<ADDRSIZE> >      ADDRESS; 
sc_signal<sc_uint<CELLSIZE> >      BE;
sc_signal<sc_uint<2> >             CMD;     
sc_signal<bool>                    CONTIG; 
sc_signal<sc_uint<8*CELLSIZE> >    WDATA;  
sc_signal<bool>                    EOP;
sc_signal<bool>                    CONS; 
sc_signal<sc_uint<PLENSIZE> >      PLEN;
sc_signal<bool>                    WRAP; 
sc_signal<bool>                    CFIXED;
sc_signal<sc_uint<CLENSIZE> >      CLEN;  
sc_signal<sc_uint<SRCIDSIZE>  >    SRCID; 
sc_signal<sc_uint<TRDIDSIZE>  >    TRDID;
sc_signal<sc_uint<PKTIDSIZE>  >    PKTID;   

/*
    DONT DELETE PLEASE
    ------------------

ADVANCED_VCI_SIGNALS(std::string name_ = ((std::string)sc_gen_unique_name("advanced_vci")))
       : RSPACK  (((std::string) (name_ + "_RSPACK")).c_str()),
	 RSPVAL  (((std::string) (name_ + "_RSPVAL")).c_str()),
	 RDATA   (((std::string) (name_ + "_RDATA" )).c_str()),
	 REOP  	 (((std::string) (name_ + "_REOP"  )).c_str()),
	 RERROR  (((std::string) (name_ + "_RERROR")).c_str()),
         RSRCID  (((std::string) (name_ + "_RSRCID")).c_str()),
         RTRDID	 (((std::string) (name_ + "_RTRDID")).c_str()),
         RPKTID  (((std::string) (name_ + "_RPKTID")).c_str()),	 
	 CMDACK  (((std::string) (name_ + "_CMDACK")).c_str()),
	 CMDVAL  (((std::string) (name_ + "_CMDVAL" )).c_str()),
	 ADDRESS (((std::string) (name_ + "_ADDRESS")).c_str()),
	 BE      (((std::string) (name_ + "_BE"     )).c_str()),
	 CMD     (((std::string) (name_ + "_CMD"    )).c_str()),
	 CONTIG  (((std::string) (name_ + "_CONTIG" )).c_str()),
	 WDATA   (((std::string) (name_ + "_WDATA"  )).c_str()),
	 EOP     (((std::string) (name_ + "_EOP"    )).c_str()),
	 CONS    (((std::string) (name_ + "_CONS"   )).c_str()),
	 PLEN    (((std::string) (name_ + "_PLEN"   )).c_str()),
	 WRAP  	 (((std::string) (name_ + "_WRAP"   )).c_str()),
	 CFIXED  (((std::string) (name_ + "_CFIXED" )).c_str()),
	 CLEN    (((std::string) (name_ + "_CLEN"   )).c_str()),
         SRCID   (((std::string) (name_ + "_SRCID"  )).c_str()),
         TRDID   (((std::string) (name_ + "_TRDID"  )).c_str()),
         PKTID   (((std::string) (name_ + "_PKTID"  )).c_str())
*/

ADVANCED_VCI_SIGNALS(sc_module_name insname = sc_module_name (sc_gen_unique_name("advanced_vci_signals")))
       : RSPACK  ("RSPACK"),
	 RSPVAL  ("RSPVAL"),
	 RDATA   ("RDATA" ),
	 REOP  	 ("REOP"  ),
	 RERROR  ("RERROR"),
         RSRCID  ("RSRCID"),
         RTRDID	 ("RTRDID"),
         RPKTID  ("RPKTID"),	 
	 CMDACK  ("CMDACK"),
	 CMDVAL  ("CMDVAL" ),
	 ADDRESS ("ADDRESS"),
	 BE      ("BE"     ),
	 CMD     ("CMD"    ),
	 CONTIG  ("CONTIG" ),
	 WDATA   ("WDATA"  ),
	 EOP     ("EOP"    ),
	 CONS    ("CONS"   ),
	 PLEN    ("PLEN"   ),
	 WRAP  	 ("WRAP"   ),
	 CFIXED  ("CFIXED" ),
	 CLEN    ("CLEN"   ),
         SRCID   ("SRCID"  ),
         TRDID   ("TRDID"  ),
         PKTID   ("PKTID"  ) {};

ADVANCED_VCI_SIGNALS();

  friend std::ostream& operator << (std::ostream& os, const ADVANCED_VCI_SIGNALS<ADDRSIZE,
				    CELLSIZE, ERRSIZE,PLENSIZE,CLENSIZE,SRCIDSIZE,
				    TRDIDSIZE,PKTIDSIZE>& sig) {
    os << std::boolalpha << std::hex;
    os << "RSPACK = " << sig.RSPACK << std::endl;
    os << "RSPVAL = " << sig.RSPVAL << std::endl;
    os << "RDATA = " << sig.RDATA << std::endl;
    os << "REOP = " << sig.REOP << std::endl;
    os << "RERROR = " << sig.RERROR << std::endl;
    os << "RSRCID = " << sig.RSRCID << std::endl;
    os << "RTRDID = " << sig.RTRDID << std::endl;
    os << "RPKTID = " << sig.RPKTID << std::endl;

    os << "CMDACK = " << sig.CMDACK << std::endl;
    os << "CMDVAL = " << sig.CMDVAL << std::endl;
    os << "ADDRESS = " << sig.ADDRESS << std::endl;
    os << "BE = " << sig.BE << std::endl;
    os << "CMD = " << sig.CMD << std::endl;
    os << "CONTIG = " << sig.CONTIG << std::endl;
    os << "WDATA = " << sig.WDATA << std::endl;
    os << "EOP = " << sig.EOP << std::endl;
    os << "CONS = " << sig.CONS << std::endl;
    os << "PLEN = " << sig.PLEN << std::endl;
    os << "WRAP = " << sig.WRAP << std::endl;
    os << "CFIXED = " << sig.CFIXED << std::endl;
    os << "CLEN = " << sig.CLEN << std::endl;
    os << "SRCID = " << sig.SRCID << std::endl;
    os << "TRDID = " << sig.TRDID << std::endl;
    os << "PKTID = " << sig.PKTID << std::endl;
    os << std::dec;
    return os;
  }

};

/*** VCI ADVANCED target ports ***/

template <int ADDRSIZE,
          int CELLSIZE,
          int ERRSIZE,
	  int PLENSIZE,
	  int CLENSIZE,
	  int SRCIDSIZE,
	  int TRDIDSIZE,
	  int PKTIDSIZE>

struct ADVANCED_VCI_TARGET  {

sc_in<bool>                        RSPACK;  
sc_out<bool>                       RSPVAL;  
sc_out<sc_uint<8*CELLSIZE> >       RDATA;   
sc_out<bool>                       REOP;   
sc_out<sc_uint<ERRSIZE> >          RERROR;
sc_out<sc_uint<SRCIDSIZE>  >       RSRCID;   
sc_out<sc_uint<TRDIDSIZE>  >       RTRDID;  
sc_out<sc_uint<PKTIDSIZE>  >       RPKTID; 

sc_out<bool>                       CMDACK; 
sc_in<bool>                        CMDVAL; 
sc_in<sc_uint<ADDRSIZE> >          ADDRESS;
sc_in<sc_uint<CELLSIZE> >          BE;    
sc_in<sc_uint<2> >                 CMD;   
sc_in<bool>                        CONTIG;
sc_in<sc_uint<8*CELLSIZE> >        WDATA; 
sc_in<bool>                        EOP;    
sc_in<bool>                        CONS;   
sc_in<sc_uint<PLENSIZE> >          PLEN;  
sc_in<bool>                        WRAP; 
sc_in<bool>                        CFIXED;  
sc_in<sc_uint<CLENSIZE> >          CLEN;    
sc_in<sc_uint<SRCIDSIZE>  >        SRCID; 
sc_in<sc_uint<TRDIDSIZE>  >        TRDID;
sc_in<sc_uint<PKTIDSIZE>  >        PKTID;
   
void operator () (ADVANCED_VCI_SIGNALS<ADDRSIZE,
					CELLSIZE,
					ERRSIZE,
					PLENSIZE,
					CLENSIZE,
					SRCIDSIZE,
					TRDIDSIZE,
					PKTIDSIZE> &sig) {
CMDACK  (sig.CMDACK);
ADDRESS (sig.ADDRESS);
BE      (sig.BE);
CFIXED  (sig.CFIXED);
CLEN    (sig.CLEN);
CMD     (sig.CMD);
CMDVAL  (sig.CMDVAL);
CONS    (sig.CONS);
CONTIG  (sig.CONTIG);
EOP     (sig.EOP);
PKTID   (sig.PKTID);
PLEN    (sig.PLEN);
RDATA   (sig.RDATA);
REOP    (sig.REOP);
RERROR  (sig.RERROR);
RPKTID  (sig.RPKTID);
RSRCID  (sig.RSRCID);
RSPACK  (sig.RSPACK);
RSPVAL  (sig.RSPVAL);
RTRDID  (sig.RTRDID);
SRCID   (sig.SRCID);
TRDID   (sig.TRDID);
WDATA   (sig.WDATA);
WRAP    (sig.WRAP);
};

void operator () (ADVANCED_VCI_TARGET<ADDRSIZE, 
					CELLSIZE,
					ERRSIZE,
					PLENSIZE,
					CLENSIZE,
					SRCIDSIZE,
					TRDIDSIZE,
					PKTIDSIZE> &ports) {
CMDACK  (ports.CMDACK);
ADDRESS (ports.ADDRESS);
BE      (ports.BE);
CFIXED  (ports.CFIXED);
CLEN    (ports.CLEN);
CMD     (ports.CMD);
CMDVAL  (ports.CMDVAL);
CONS    (ports.CONS);
CONTIG  (ports.CONTIG);
EOP     (ports.EOP);
PKTID   (ports.PKTID);
PLEN    (ports.PLEN);
RDATA   (ports.RDATA);
REOP    (ports.REOP);
RERROR  (ports.RERROR);
RPKTID  (ports.RPKTID);
RSRCID  (ports.RSRCID);
RSPACK  (ports.RSPACK);
RSPVAL  (ports.RSPVAL);
RTRDID  (ports.RTRDID);
SRCID   (ports.SRCID);
TRDID   (ports.TRDID);
WDATA   (ports.WDATA);
WRAP    (ports.WRAP);
};

  friend std::ostream& operator << (std::ostream& os, const ADVANCED_VCI_TARGET<ADDRSIZE,
				    CELLSIZE, ERRSIZE,PLENSIZE,CLENSIZE,SRCIDSIZE,
				    TRDIDSIZE,PKTIDSIZE>& sig) {
    os << std::boolalpha << std::hex;
    os << "RSPACK = " << sig.RSPACK << std::endl;
    os << "RSPVAL = " << sig.RSPVAL << std::endl;
    os << "RDATA = " << sig.RDATA << std::endl;
    os << "REOP = " << sig.REOP << std::endl;
    os << "RERROR = " << sig.RERROR << std::endl;
    os << "RSRCID = " << sig.RSRCID << std::endl;
    os << "RTRDID = " << sig.RTRDID << std::endl;
    os << "RPKTID = " << sig.RPKTID << std::endl;

    os << "CMDACK = " << sig.CMDACK << std::endl;
    os << "CMDVAL = " << sig.CMDVAL << std::endl;
    os << "ADDRESS = " << sig.ADDRESS << std::endl;
    os << "BE = " << sig.BE << std::endl;
    os << "CMD = " << sig.CMD << std::endl;
    os << "CONTIG = " << sig.CONTIG << std::endl;
    os << "WDATA = " << sig.WDATA << std::endl;
    os << "EOP = " << sig.EOP << std::endl;
    os << "CONS = " << sig.CONS << std::endl;
    os << "PLEN = " << sig.PLEN << std::endl;
    os << "WRAP = " << sig.WRAP << std::endl;
    os << "CFIXED = " << sig.CFIXED << std::endl;
    os << "CLEN = " << sig.CLEN << std::endl;
    os << "SRCID = " << sig.SRCID << std::endl;
    os << "TRDID = " << sig.TRDID << std::endl;
    os << "PKTID = " << sig.PKTID << std::endl;
    os << std::dec;
    return os;
  }

};


/*** VCI ADVANCED initiator ports  ***/

template <int ADDRSIZE,
          int CELLSIZE,
	  int ERRSIZE,
	  int PLENSIZE,
	  int CLENSIZE,
	  int SRCIDSIZE,
	  int TRDIDSIZE,
	  int PKTIDSIZE> 

struct ADVANCED_VCI_INITIATOR {

sc_out<bool>                        RSPACK;  
sc_in<bool>                         RSPVAL;
sc_in<sc_uint<8*CELLSIZE> >         RDATA;  
sc_in<bool>                         REOP;
sc_in<sc_uint<ERRSIZE> >            RERROR;  
sc_in<sc_uint<SRCIDSIZE>  >         RSRCID; 
sc_in<sc_uint<TRDIDSIZE>  >         RTRDID; 
sc_in<sc_uint<PKTIDSIZE>  >         RPKTID; 
   
sc_in<bool>                         CMDACK;
sc_out<bool>                        CMDVAL; 
sc_out<sc_uint<ADDRSIZE> >          ADDRESS;
sc_out<sc_uint<CELLSIZE> >          BE;   
sc_out<sc_uint<2> >                 CMD;  
sc_out<bool>                        CONTIG;
sc_out<sc_uint<8*CELLSIZE> >        WDATA; 
sc_out<bool>                        EOP;  
sc_out<bool>                        CONS;
sc_out<sc_uint<PLENSIZE> >          PLEN;   
sc_out<bool>                        WRAP;  
sc_out<bool>                        CFIXED;
sc_out<sc_uint<CLENSIZE> >          CLEN;   
sc_out<sc_uint<SRCIDSIZE> >         SRCID; 
sc_out<sc_uint<TRDIDSIZE> >         TRDID;
sc_out<sc_uint<PKTIDSIZE> >         PKTID; 

void operator () (ADVANCED_VCI_SIGNALS<ADDRSIZE,
					CELLSIZE,
					ERRSIZE,
					PLENSIZE,
					CLENSIZE,
					SRCIDSIZE,
					TRDIDSIZE,
					PKTIDSIZE> &sig) {
CMDACK  (sig.CMDACK);
ADDRESS (sig.ADDRESS);
BE      (sig.BE);
CFIXED  (sig.CFIXED);
CLEN    (sig.CLEN);
CMD     (sig.CMD);
CMDVAL  (sig.CMDVAL);
CONS    (sig.CONS);
CONTIG  (sig.CONTIG);
EOP     (sig.EOP);
PKTID   (sig.PKTID);
PLEN    (sig.PLEN);
RDATA   (sig.RDATA);
REOP    (sig.REOP);
RERROR  (sig.RERROR);
RPKTID  (sig.RPKTID);
RSRCID  (sig.RSRCID);
RSPACK  (sig.RSPACK);
RSPVAL  (sig.RSPVAL);
RTRDID  (sig.RTRDID);
SRCID   (sig.SRCID);
TRDID   (sig.TRDID);
WDATA   (sig.WDATA);
WRAP    (sig.WRAP);
};

void operator () (ADVANCED_VCI_INITIATOR<ADDRSIZE,
					CELLSIZE,
					ERRSIZE,
					PLENSIZE,
					CLENSIZE,
					SRCIDSIZE,
					TRDIDSIZE,
					PKTIDSIZE> &ports) {
CMDACK  (ports.CMDACK);
ADDRESS (ports.ADDRESS);
BE      (ports.BE);
CFIXED  (ports.CFIXED);
CLEN    (ports.CLEN);
CMD     (ports.CMD);
CMDVAL  (ports.CMDVAL);
CONS    (ports.CONS);
CONTIG  (ports.CONTIG);
EOP     (ports.EOP);
PKTID   (ports.PKTID);
PLEN    (ports.PLEN);
RDATA   (ports.RDATA);
REOP    (ports.REOP);
RERROR  (ports.RERROR);
RPKTID  (ports.RPKTID);
RSRCID  (ports.RSRCID);
RSPACK  (ports.RSPACK);
RSPVAL  (ports.RSPVAL);
RTRDID  (ports.RTRDID);
SRCID   (ports.SRCID);
TRDID   (ports.TRDID);
WDATA   (ports.WDATA);
WRAP    (ports.WRAP);
};

  friend std::ostream& operator << (std::ostream& os, const ADVANCED_VCI_INITIATOR<ADDRSIZE,
				    CELLSIZE, ERRSIZE,PLENSIZE,CLENSIZE,SRCIDSIZE,
				    TRDIDSIZE,PKTIDSIZE>& sig) {
    os << std::boolalpha << std::hex;
    os << "RSPACK = " << sig.RSPACK << std::endl;
    os << "RSPVAL = " << sig.RSPVAL << std::endl;
    os << "RDATA = " << sig.RDATA << std::endl;
    os << "REOP = " << sig.REOP << std::endl;
    os << "RERROR = " << sig.RERROR << std::endl;
    os << "RSRCID = " << sig.RSRCID << std::endl;
    os << "RTRDID = " << sig.RTRDID << std::endl;
    os << "RPKTID = " << sig.RPKTID << std::endl;

    os << "CMDACK = " << sig.CMDACK << std::endl;
    os << "CMDVAL = " << sig.CMDVAL << std::endl;
    os << "ADDRESS = " << sig.ADDRESS << std::endl;
    os << "BE = " << sig.BE << std::endl;
    os << "CMD = " << sig.CMD << std::endl;
    os << "CONTIG = " << sig.CONTIG << std::endl;
    os << "WDATA = " << sig.WDATA << std::endl;
    os << "EOP = " << sig.EOP << std::endl;
    os << "CONS = " << sig.CONS << std::endl;
    os << "PLEN = " << sig.PLEN << std::endl;
    os << "WRAP = " << sig.WRAP << std::endl;
    os << "CFIXED = " << sig.CFIXED << std::endl;
    os << "CLEN = " << sig.CLEN << std::endl;
    os << "SRCID = " << sig.SRCID << std::endl;
    os << "TRDID = " << sig.TRDID << std::endl;
    os << "PKTID = " << sig.PKTID << std::endl;
    os << std::dec;
    return os;
  }
};

#endif
