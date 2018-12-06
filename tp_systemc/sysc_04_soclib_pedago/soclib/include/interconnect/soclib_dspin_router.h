/**************************************************************************
 * File : DSPIN_ROUTER.h
 * Date : 11/1/2005
 * authors : Alain Greiner & Romain Michard & Ivan MIRO
 * This program is released under the GNU public license
 * Copyright : UPMC - LIP6
 *
 * The DSPIN router is used to interconnect several subsystems, by a
 * network on chip respecting a two dimensional mesh topology.
 * There is one DSPIN router for each sub-system. Each router is connected
 * to five entities :
 * - the NORTH router, 
 * - the SOUTH router,
 * - the EAST router, 
 * - the WEST router,
 * - the LOCAL subsystem.
 * The local subsystem can be a VCI initiator, a VCI target or both.
 * In order to avoid dead_lock condition, there is two separated REQ 
 * and RSP networks for transmission of VCI requests and VCI responses.
 * If a subsystem is only a target (resp. an initiator), the local routers
 * will be simplified : the local REQ router will not have a LOCAL input port
 * (resp. output), and the local RSP router will not have a LOCAL output port
 * (resp. input).
 * 
 * This router is intended to be used with two external VCI wrappers :
 * - DSPIN_VCI_MASTER_WRAPPER 
 * - DSPIN_VCI_TARGET_WRAPPER
 *
 * The DSPIN router contains a bi-synchronous FIFO on each input port
 * and a synchronous FIFO on each output port, but for sake of simplicity,
 * this systemC model is fully synchronous. 
 *
 * This component has two "template" parameters : IN_FIFO_SIZE 
 * and OUT_FIFO_SIZE define the FIFOs depths.
 * It has one "constructor" parameters : IDENT define the router
 * local coordinates (X = 4 LSB bits / Y = 4MSB bits).
 *
 * The destination coordinates are contained in the right byte of the
 * DSPIN packet header : |YYYY|XXXX| (4 bits for X and 4 bits for Y).
 * Both X and Y coordinates must be larger or equal to 0 an smaller than 16. 
 * The routing algorithm is X-first, and is totally determistic, in
 * order to garanty the "in order delivery property.
 * The allocation policy for a given output port requested by several
 * input ports is round-robin.
***************************************************************************/

#ifndef SOCLIB_DSPIN_ROUTER_H
#define SOCLIB_DSPIN_ROUTER_H

#include <systemc.h>
#include "interconnect/soclib_dspin_interfaces.h"
#include "shared/soclib_generic_fifo.h"

template<int DATA_SIZE,
	 unsigned short IN_FIFO_SIZE, 
	 unsigned short OUT_FIFO_SIZE>

struct SOCLIB_DSPIN_ROUTER : sc_module {

// EXTERNAL PORTS

sc_in<bool>             CLK;
sc_in<bool>		RESETN;
DSPIN_IN<DATA_SIZE>   		IN[5];
DSPIN_OUT<DATA_SIZE>  		OUT[5];

// REGISTERS

sc_signal<bool>	ALLOC_OUT[5];	// output ports allocation state 
sc_signal<bool>	ALLOC_IN[5];	// input ports allocation state 
sc_signal<int>	INDEX_OUT[5];	// index of the connected input FIFO 
sc_signal<int>	INDEX_IN[5];	// index of the connected output FIFO 

soclib_generic_fifo<IN_FIFO_SIZE,DATA_SIZE>	FIFO_OUT[5];	// output fifos
soclib_generic_fifo<OUT_FIFO_SIZE,DATA_SIZE>	FIFO_IN[5];	// input fifos

// INSTANCE NAME & COORDINATES
const	char*	NAME;
int		XLOCAL;
int		YLOCAL;

// MNEMONICS
enum{
	NORTH	= 0,
	SOUTH	= 1,
	EAST	= 2,
	WEST	= 3,
	LOCAL	= 4
};


#ifdef _DEBUG_ROUTER_DSPIN
int 	DEBUG_TIME;
int	DEBUG_ALLOCATED[5];
int	DEBUG_ALLOCATED_NON_USED[5];
int	DEBUG_FIFO_IN_ELEMS[5];
int	DEBUG_FIFO_OUT_ELEMS[5];
int	DEBUG_BLOCKED_BY[5][5];
#endif

/////////////////////////////////////////////
//           Transition()
/////////////////////////////////////////////
 
void Transition()
{
	int			i,j,k;
	int			xreq,yreq;
	bool			fifo_in_write[5];	// control signals
	bool    		fifo_in_read[5];	// for the input fifos
	sc_uint<DATA_SIZE>	fifo_in_data[5];
	bool			fifo_out_write[5];	// control signals
	bool    		fifo_out_read[5];	// for the output fifos
	sc_uint<DATA_SIZE>	fifo_out_data[5];
	bool			req[5][5];		// REQ[i][j] signals from
							// input i to output j
if(RESETN == false) {
	for(i = 0 ; i < 5 ; i++) {
		ALLOC_IN[i] = false;
		ALLOC_OUT[i] = false;
		INDEX_IN[i] = 0;
		INDEX_OUT[i] = 0;
		FIFO_IN[i].init();
		FIFO_OUT[i].init();
#ifdef _DEBUG_ROUTER_DSPIN
		DEBUG_ALLOCATED[i]=0;
		DEBUG_ALLOCATED_NON_USED[i]=0;
		DEBUG_FIFO_IN_ELEMS[i]=0;
		DEBUG_FIFO_OUT_ELEMS[i]=0;
		for(j=0;j<5;j++)
			DEBUG_BLOCKED_BY[i][j]=0;
#endif
	}
#ifdef _DEBUG_ROUTER_DSPIN
	DEBUG_TIME=0;
#endif


} else {

// fifo_in_write[i] and fifo_in_data[i]

for(i = 0 ; i < 5 ; i++) {
	fifo_in_write[i] = IN[i].WRITE.read();
	fifo_in_data[i] = IN[i].DATA.read();
}

// fifo_out_read[i] 

for(i = 0 ; i < 5 ; i++) {
	fifo_out_read[i] = OUT[i].READ.read();
}

// req[i][j]  : implement the X first routing algorithm

for(i = 0 ; i < 5 ; i++) { // loop on the input ports
	if((FIFO_IN[i].rok() == true) && 
		(((FIFO_IN[i].read() >> (DATA_SIZE-4) ) & DSPIN_BOP) == DSPIN_BOP)) {
	        xreq = (int)(FIFO_IN[i].read() & 0xF);
		yreq = (int)((FIFO_IN[i].read() >> 4) & 0xF);
		if(xreq < XLOCAL) {
		        req[i][LOCAL] = false;
			req[i][NORTH] = false;
			req[i][SOUTH] = false;
			req[i][EAST]  = false;
			req[i][WEST]  = true;
		} else if(xreq > XLOCAL) {
			req[i][LOCAL] = false;
			req[i][NORTH] = false;
			req[i][SOUTH] = false;
			req[i][EAST]  = true;
			req[i][WEST]  = false;
		} else if(yreq < YLOCAL) {
	        	req[i][LOCAL] = false;
			req[i][NORTH] = false;
			req[i][SOUTH] = true;
			req[i][EAST]  = false;
			req[i][WEST]  = false;
		} else if(yreq > YLOCAL) {
		        req[i][LOCAL] = false;
			req[i][NORTH] = true;
			req[i][SOUTH] = false;
			req[i][EAST]  = false;
			req[i][WEST]  = false;
		} else {
		        req[i][LOCAL] = true;
			req[i][NORTH] = false;
			req[i][SOUTH] = false;
			req[i][EAST]  = false;
			req[i][WEST]  = false;
		}
	} else {
			req[i][LOCAL] = false;
			req[i][NORTH] = false;
			req[i][SOUTH] = false;
			req[i][EAST]  = false;
			req[i][WEST]  = false;
	}
} // end loop on the inputs
			
// fifo_in_read[i]

for(i = 0 ; i < 5 ; i++) { // loop on the inputs
	if(ALLOC_IN[i] == true) {
		fifo_in_read[i] = FIFO_OUT[INDEX_IN[i]].wok();
	} else {
		fifo_in_read[i] = false;
	}
} // end loop on the inputs

// fifo_out_write[j] and fifo_out_data[j]

for(j = 0 ; j < 5 ; j++) { // loop on the outputs
	if(ALLOC_OUT[j] == true) {
		fifo_out_write[j] = FIFO_IN[INDEX_OUT[j]].rok();
		fifo_out_data[j]  = FIFO_IN[INDEX_OUT[j]].read();
	} else {
		fifo_out_write[j] = false;
	}
} // end loop on the outputs

// ALLOC_IN, ALLOC_OUT, INDEX_IN et INDEX_OUT : implements the round-robin allocation policy

for(j = 0 ; j < 5 ; j++) { // loop on the outputs
	if(ALLOC_OUT[j] == false) { // possible new allocation
	  
		//Routage par round-robin
		for(k = INDEX_OUT[j]+1 ; k < (INDEX_OUT[j] + 6) ; k++) { // loop on the inputs
			i = k % 5;
			if(req[i][j] == true ) {
				ALLOC_OUT[j] = true;
				INDEX_OUT[j] = i;
				ALLOC_IN[i] = true;
				INDEX_IN[i] = j;
				break;
			}
		} // end loop on the inputs



	} else { // possible desallocation
	  //if((((FIFO_IN[INDEX_OUT[j]].read() >> (DATA_SIZE-4) ) & DSPIN_EOP) == DSPIN_EOP) && (FIFO_OUT[j].wok() == true )) {
	  if((((FIFO_IN[INDEX_OUT[j]].read() >> (DATA_SIZE-4) ) & DSPIN_EOP) == DSPIN_EOP) && (FIFO_OUT[j].wok() == true ) && FIFO_IN[INDEX_OUT[j]].rok()) {
			ALLOC_OUT[j] = false;
			ALLOC_IN[INDEX_OUT[j]] = false;
		}
	}
} // end loop on the outputs

//  FIFOS

for(i = 0 ; i < 5 ; i++) {
	if((fifo_in_write[i] == true ) && (fifo_in_read[i] == true ))
		{FIFO_IN[i].put_and_get(fifo_in_data[i]);}
	if((fifo_in_write[i] == true ) && (fifo_in_read[i] == false))
		{FIFO_IN[i].simple_put(fifo_in_data[i]);}
	if((fifo_in_write[i] == false) && (fifo_in_read[i] == true ))
		{FIFO_IN[i].simple_get();}
}
for(i = 0 ; i < 5 ; i++) {
	if((fifo_out_write[i] == true ) && (fifo_out_read[i] == true ))
		{FIFO_OUT[i].put_and_get(fifo_out_data[i]);}
	if((fifo_out_write[i] == true ) && (fifo_out_read[i] == false))
		{FIFO_OUT[i].simple_put(fifo_out_data[i]);}
	if((fifo_out_write[i] == false) && (fifo_out_read[i] == true ))
		{FIFO_OUT[i].simple_get();}
}

#ifdef _DEBUG_ROUTER_DSPIN
DEBUG_TIME++;		//Time is runing
for(i=0;i<5;i++) {
	if(ALLOC_OUT[i]==true) {
		//FIFO allocated
		DEBUG_ALLOCATED[i]++;
		if( (FIFO_IN[INDEX_OUT[i]].rok()==false) || (FIFO_OUT[i].wok()==false)  ) {
			//Data can't be sent between fifos 
			DEBUG_ALLOCATED_NON_USED[i]++;
			DEBUG_BLOCKED_BY[i][INDEX_OUT[i]]++;
		}
	}
	DEBUG_FIFO_IN_ELEMS[i]  += FIFO_IN[i].filled_status();	
	DEBUG_FIFO_OUT_ELEMS[i] += FIFO_OUT[i].filled_status();	
}



#endif

} // end else RESETN

};  // end Transition()

/////////////////////////////////////////////
//           GenMoore()
/////////////////////////////////////////////
 
void GenMoore()
{
int	i;

// input ports : READ signals
for(i = 0 ; i < 5 ; i++) { 
	IN[i].READ = FIFO_IN[i].wok();
}

// output ports : DATA & WRITE signals
for(i = 0 ; i < 5 ; i++) { 
	OUT[i].DATA = FIFO_OUT[i].read(); 
	OUT[i].WRITE =  FIFO_OUT[i].rok();
}

}; // end GenMoore()

////////////////////////////////////////
//           Constructor   
////////////////////////////////////////

SC_HAS_PROCESS(SOCLIB_DSPIN_ROUTER);

SOCLIB_DSPIN_ROUTER (sc_module_name name, char ident)/*:
	FIFO_IN("fifo_in"),
	FIFO_OUT("fifo_out")*/
{
int 	x =  ident & 0x0000000F;
int	y = (ident & 0x000000F0) >> 4;

if((x < 0) || (x > 14) || (y < 0) || (y > 15)) {
	std::cerr << "Configuration Error in component DSPIN_ROUTER" << std::endl;
	std::cerr << "Both X and Y coordinates must be larger or equal to 0" << std::endl;
	std::cerr << "and no larger than 15" << std::endl;
	sc_stop(); 
}

SC_METHOD(Transition);
sensitive_pos << CLK;

SC_METHOD(GenMoore);
sensitive_neg << CLK;

NAME = (const char *) name;
XLOCAL = x;
YLOCAL = y;

printf("Successful instanciation of SOCLIB_DSPIN_ROUTER: %s \n",NAME);

}; // end constructor

~SOCLIB_DSPIN_ROUTER() {
	//Destroy
#ifdef _DEBUG_ROUTER_DSPIN
	int i;
	float allocate_rate[5];
	float allocate_non_used_rate[5];
	float throughput[5];
	float fifo_in_elems[5];
	float fifo_out_elems[5];
	int id = (YLOCAL<<4)+XLOCAL;
	for(i=0;i<5;i++) {
		allocate_rate[i] = DEBUG_ALLOCATED[i]*100.0/DEBUG_TIME;
		if (DEBUG_ALLOCATED[i]>0)
			allocate_non_used_rate[i] = DEBUG_ALLOCATED_NON_USED[i]*100.0/DEBUG_ALLOCATED[i];
		else
			allocate_non_used_rate[i] = 0;
		fifo_in_elems[i]  = DEBUG_FIFO_IN_ELEMS[i]*1.0/DEBUG_TIME;
		fifo_out_elems[i] = DEBUG_FIFO_OUT_ELEMS[i]*1.0/DEBUG_TIME;
		throughput[i]=(DEBUG_ALLOCATED[i]-DEBUG_ALLOCATED_NON_USED[i])*100.0/DEBUG_TIME;
//		printf("Time %i, DEBUG_ALLOCATED %i\n",(int)DEBUG_TIME,(int)DEBUG_ALLOCATED[0]);
	}
	printf("Router %.2x outFIFO throughput(allocate_rate)",id);
	for(i=0;i<5;i++) {
    		printf("  %5.1f(%5.1f)",throughput[i],allocate_rate[i]);
//    		printf("  %5.1f(%5.1f)[%5.1f]",throughput[i],allocate_rate[i], allocate_non_used_rate[i]);
	}
	printf("\n");
	for(i=0;i<5;i++) {
		printf("Router %.2x FIFO blocked by                     %10i    %10i    %10i    %10i    %10i\n",id, DEBUG_BLOCKED_BY[0][i],DEBUG_BLOCKED_BY[1][i],DEBUG_BLOCKED_BY[2][i],DEBUG_BLOCKED_BY[3][i],DEBUG_BLOCKED_BY[4][i]);
	}
	
	
	
	printf("\nRouter %.2x fifo_in  %.1f %.1f %.1f %.1f %.1f\n",id,fifo_in_elems[0],fifo_in_elems[1],fifo_in_elems[2],fifo_in_elems[3],fifo_in_elems[4]);
	printf("Router %.2x fifo_out %.1f %.1f %.1f %.1f %.1f\n",id,fifo_out_elems[0],fifo_out_elems[1],fifo_out_elems[2],fifo_out_elems[3],fifo_out_elems[4]);
#endif	
};

}; // end struct DSPIN_ROUTER

#endif 
