
/**************************************************************************************
 * File : soclib_caches_interfaces.h
 * Date : O7/05/2005
 * Authors : A.Greiner and H.Charlery
 * It is released under the GNU Public License.
 * Copyright : UPMC/LIP6
 * 
 * This file defines a standard "cache interface" that can be used by several
 * processor models. 
 * Separate interfaces are defined for data cache and instruction cache.
 * 
 * HISTORY
 *
 * - 20/05/2005 A.Greiner 
 *   For the DCACHE, the command types  have been enriched in order to support both 
 *   the "word uncached" and the "burst uncached" modes. 
 *   For the ICACHE, a TYPE field has been introduced, in order to support both
 *   the "uncached read", and the "line invalidate" modes. 
 *   Simultaneouly, the UNC signals and ports have been removed from both the DCACHE 
 *   and ICACHE interfaces. 
**************************************************************************************/

#ifndef SOCLIB_CACHES_INTERFACES_H
#define SOCLIB_CACHES_INTERFACES_H

#include <systemc.h>

// Possible values of the TYPE command for the DCACHE

#define DTYPE_WW  	0x4	 /* Write Word */
#define DTYPE_WH  	0x5	 /* Write Half */
#define DTYPE_WB  	0x6	 /* Write Byte */
#define DTYPE_RW  	0x0	 /* Read Word Cached */
#define DTYPE_RZ  	0x3	 /* Line Invalidate */
//#define DTYPE_RU  	0x1	 /* Read Word Uncached */
//#define DTYPE_RB  	0x2	 /* Read Burst Uncached*/

// Possible values of the TYPE command for the ICACHE

#define ITYPE_RW  	0x0	 /* Read Word Cached */
#define ITYPE_RZ 	0x1	 /* Line Invalidate */
#define ITYPE_RU	0x2	 /* Read Word Uncached */


// Signals for ICACHE

struct ICACHE_SIGNALS {

	sc_signal<bool>          REQ;  // valid read request
	sc_signal<sc_uint<32> >  ADR;  // instruction address
	sc_signal<sc_uint<2> >   TYPE; // instruction address
	sc_signal<bool>          FRZ; // instruction not valid
	sc_signal<sc_uint<32> >  INS;  // 32 bits instruction
	sc_signal<bool>          BERR; // bus or memory error
	
ICACHE_SIGNALS (std::string name_ = (std::string) sc_gen_unique_name ("icache"))
	: REQ    (((std::string) (name_ + "_REQ" )).c_str()),
	  ADR    (((std::string) (name_ + "_ADR" )).c_str()),
	  TYPE   (((std::string) (name_ + "_TYPE")).c_str()),
	  FRZ    (((std::string) (name_ + "_FRZ" )).c_str()),
	  INS    (((std::string) (name_ + "_INS" )).c_str()),
	  BERR   (((std::string) (name_ + "_BERR")).c_str())
{
}

}; // end struct ICACHE_SIGNALS

/* ICACHE ports (processor side) */

struct ICACHE_PROCESSOR_PORTS {

	sc_out<bool> 	        REQ;
	sc_out<sc_uint<32> >    ADR;
	sc_out<sc_uint<2> >     TYPE;
	sc_in<bool> 	        FRZ;
	sc_in<sc_uint<32> >     INS;
	sc_in<bool>  	        BERR;
    
	void operator () (ICACHE_SIGNALS &sig) {
		REQ   (sig.REQ);
		ADR   (sig.ADR);
		TYPE  (sig.TYPE);
		FRZ  (sig.FRZ);
		INS   (sig.INS);
		BERR  (sig.BERR);
	};		
}; // end struct ICACHE_PROCESSOR_PORTS


/* ICACHE ports (cache side) */

struct ICACHE_CACHE_PORTS {

	sc_in<bool> 	        REQ;
	sc_in<sc_uint<32> >     ADR; 
	sc_in<sc_uint<2> >      TYPE; 
	sc_out<bool> 	        FRZ;
	sc_out<sc_uint<32> >    INS;
	sc_out<bool>            BERR;
    
	void operator () (ICACHE_SIGNALS &sig) {
		REQ   (sig.REQ);
		ADR   (sig.ADR);
		TYPE  (sig.TYPE);
		FRZ  (sig.FRZ);
		INS   (sig.INS);
		BERR  (sig.BERR);
	};		
}; // end struct ICACHE_CACHE_PORTS



/* Signals for DCACHE */

struct DCACHE_SIGNALS {

  sc_signal<bool>         REQ;	 // valid request
  sc_signal<sc_uint<3> >  TYPE;	 // request type
  sc_signal<bool>         UNC;	 // uncached request
  sc_signal<sc_uint<32> > WDATA; // data from processor
  sc_signal<sc_uint<32> > ADR;	 // address
  sc_signal<bool>         FRZ;	 // request not accepted
  sc_signal<sc_uint<32> > RDATA; // data from cache
  sc_signal<bool>         BERR;	 // bus or memory error 

DCACHE_SIGNALS (std::string name_ = (std::string) sc_gen_unique_name("dcache"))
 : REQ     (((std::string) (name_ + "_REQ"   )).c_str()),
   TYPE    (((std::string) (name_ + "_TYPE"  )).c_str()),
   UNC     (((std::string) (name_ + "_UNC"   )).c_str()),
   WDATA   (((std::string) (name_ + "_WDATA" )).c_str()),
   ADR     (((std::string) (name_ + "_ADR"   )).c_str()),
   FRZ     (((std::string) (name_ + "_FRZ"   )).c_str()),
   RDATA   (((std::string) (name_ + "_RDATA" )).c_str()),
   BERR    (((std::string) (name_ + "_BERR"  )).c_str())
{
}

}; // end struct DCACHE_SIGNALS


/* DCACHE ports (processor side) */

struct DCACHE_PROCESSOR_PORTS {

  sc_out<bool>         REQ;
  sc_out<sc_uint<3> >  TYPE;
  sc_out<bool>         UNC;
  sc_out<sc_uint<32> > WDATA;
  sc_out<sc_uint<32> > ADR;
  sc_in<bool>          FRZ;
  sc_in<sc_uint<32> >  RDATA;
  sc_in<bool>          BERR; 

	void operator () (DCACHE_SIGNALS &sig) {
		REQ    (sig.REQ);
		TYPE   (sig.TYPE);
		UNC    (sig.UNC);
		WDATA  (sig.WDATA);
		ADR    (sig.ADR);
		FRZ   (sig.FRZ);
		RDATA  (sig.RDATA);
		BERR   (sig.BERR);
	};		
}; // end DCACHE_PROCESSOR_PORTS


/* DCACHE ports (cache side) */

struct DCACHE_CACHE_PORTS {

  sc_in<bool>          REQ;
  sc_in<sc_uint<3> >   TYPE;
  sc_in<bool>          UNC;
  sc_in<sc_uint<32> >  WDATA;
  sc_in<sc_uint<32> >  ADR;
  sc_out<bool>         FRZ;
  sc_out<sc_uint<32> > RDATA;
  sc_out<bool>         BERR; 

    
	void operator () (DCACHE_SIGNALS &sig) {
		REQ    (sig.REQ);
		TYPE   (sig.TYPE);
		UNC    (sig.UNC);
		WDATA  (sig.WDATA);
		ADR    (sig.ADR);
		FRZ   (sig.FRZ);
		RDATA  (sig.RDATA);
		BERR   (sig.BERR);
	};		
}; // end struct DCACHE_CACHE_PORTS

#endif

