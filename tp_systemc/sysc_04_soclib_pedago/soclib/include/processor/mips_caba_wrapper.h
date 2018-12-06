#ifndef MIPS_CABA_WRAPPER_H
#define MIPS_CABA_WRAPPER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <systemc.h>
#include "cache/soclib_xcache_interface.h"
#include "processor/mipsr3000.h"

///////////////////////////////////////////////////////
//	Processor structure definition
///////////////////////////////////////////////////////
 
template<bool	MULTI_CONTEXT,
	 int 	N_CTXT,
 	 int	TIME_OUT>

struct MIPS_CABA_WRAPPER:sc_module {
				
	//	I/O PORTS  
	
	sc_in<bool> 			CLK;
	sc_in<bool> 			RESETN;
	ICACHE_PROCESSOR_PORTS 		ICACHE; // instruction cache
	DCACHE_PROCESSOR_PORTS 		DCACHE; // data cache
	sc_in<bool>   			IT_5;	// IRQ 5
	sc_in<bool>   			IT_4;	// IRQ 4
	sc_in<bool>   			IT_3;	// IRQ 3
	sc_in<bool>   			IT_2;	// IRQ 2
	sc_in<bool>   			IT_1;	// IRQ 1
	sc_in<bool>   			IT_0;	// IRQ 0
  	sc_in<int>			I_FRZ;  // for multi-context only
  	sc_in<int>			D_FRZ;  // for multi-context only
   
	//	STRUCTURAL PARAMETERS
	
	const	char 			*NAME;		 // Name of the current instance 
	int				NBC;	 // number of harware contexts
	int				MSB_NUMBER;	 // number of address MSB bits
	int				MSB_MASK;	 // mask for the uncached table index

	MIPSR3000	mips;

///////////////////////////////////////////////////////////
//	constructor
///////////////////////////////////////////////////////////

SC_HAS_PROCESS(MIPS_CABA_WRAPPER);

MIPS_CABA_WRAPPER(sc_module_name 		insname, 
			int 			ident)
{
	SC_METHOD (transition);
  dont_initialize();
	sensitive_pos << CLK;

	SC_METHOD (genMoore);
  dont_initialize();
	sensitive_neg << CLK;
	
  NAME  = (const char*) insname;

	mips.set_ident(ident);
	mips.reset();

  printf("Successful Instanciation of MIPS_CABA_WRAPPER : %s\n",NAME);

};  // end constructor

///////////////////////////////////////////////////////
//	Transition()
///////////////////////////////////////////////////////

void transition()
{
	int interrupt_signal;

	if(RESETN == false) {
		mips.reset();
		return;
	};  // end RESET

	if (DCACHE.BERR==true)
		mips.setDataBerr();

// printf("transition before frz\n");

	if ((ICACHE.FRZ==true) || (DCACHE.FRZ==true))
		return;

// printf("transition after frz\n");
        interrupt_signal = (int)IT_0.read() << 10 |
                     (int)IT_1.read() << 11 |
                     (int)IT_2.read() << 12 |
                     (int)IT_3.read() << 13 |
                     (int)IT_4.read() << 14 |
                     (int)IT_5.read() << 15 ;
	mips.setIrq(interrupt_signal);

	mips.setInstruction(ICACHE.BERR.read(),(int)ICACHE.INS.read());	
	mips.setRdata(DCACHE.BERR.read(),(int)DCACHE.RDATA.read());	
	mips.step();
};  // end transition()

////////////////////////////////////////////////////////////
//	genMoore()
////////////////////////////////////////////////////////////

void genMoore()
{
	int i_type,i_adr;
	int d_type,d_adr,d_wdata;

	bool i_req=mips.getInstructionRequest(i_type,i_adr);
//printf("i_req=%d i_adr=%x\n",i_req,i_adr);
	ICACHE.REQ=i_req;
	if (i_req)
	{
		ICACHE.ADR=(sc_uint<32>)i_adr;
	}
	
	bool d_req=mips.getDataRequest(d_type, d_adr, d_wdata);
	DCACHE.REQ=d_req;
//printf("d_req=%d d_adr=%x\n",d_req,d_adr);
	if (d_req)
	{
//printf("type=%d adr=%x wdata=%x\n",d_type,d_adr,d_wdata);
		DCACHE.TYPE=(sc_uint<3>)d_type;
		DCACHE.ADR=(sc_uint<32>)d_adr;
		DCACHE.WDATA=(sc_uint<32>)d_wdata;
		DCACHE.UNC=false;
	}

}; // end genMoore()

}; // end struct MIPS_CABA_WRAPPER

#endif
