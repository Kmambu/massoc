
//////////////////////////////////////////////////////////////////////////
// File : soclib_vci_gmn.h
// Date : 01/02/2005
// authors : Laurent Mortiez, Alain Greiner
// This program is released under the GNU public license
// Copyright : UPMC - LIP6
//
// This component is a generic VCI ADVANCED micro-network.
// It behaves as two independant packet switched routing networks,
// one for the VCI requests, one for the VCI responses.
// It gives the system designer a generic "communication black box"
// with a variable number of VCI target ports and a variable number
// of VCI initiator ports.
//
// The saturarion threshold and minimal latency can be parameterized
// to fit the behaviour of a specific physical network on chip.
// - The latency can be adjusted with the MIN_LATENCY parameter, that
// defines the latency of an empty network (without contention).
// - It is possible to increase the saturation threshold (i.e. the maximal
// load accepted by the network) by increasing the FIFO_DEPTH parameter.
//
// This component has 12 "template" parameters :
// - int NB_INITIATOR       	: the number of VCI initiators
// - short NB_TARGET          	: the number of VCI targets 
// - short MIN_MIN_LATENCY    	: minimal latency (number of cycles)
// - int FIFO_DEPTH         	: internal buffer capacity (number of VCI words)
//
//  Using shorthand VCI_PARAM for these ones
// - int ADDRSIZE 		: number of bits of the VCI ADDRESS field
// - int CELLSIZE 		: number of bytes for the VCI DATA field
// - int ERRSIZE  		: number of extra bits for the VCI ERROR field
// - int PLENSIZE		: number of extra bits for the VCI PLEN field
// - int CLENSIZE		: number of extra bits for the VCI CLEN field
// - int SRCIDSIZE		: number of bits of the VCI SRCID field
// - int TRDIDSIZE		: number of bits of the VCI TRDID field
// - int PKTIDSIZE		: number of bits of the VCI PKTID field
//
// Warning:
// The VCI SRCID field of an initiator must match the vgmn target port index
// The VCI target index must match the vgmn initiator port index
//
// The number of bits for the SRCID field should never be larger than 16
// The number of bits for the PKTID field should never be larger than 4
// The number of bits for the TRDID field should never be larger than 8
//
// This component has 2 "constructor" parameters :
// - char	*NAME	     : Instance name
// - SOCLIB_MAPPING_TABLE SEG : mapping table
//
//////////////////////////////////////////////////////////////////////////

#ifndef SOCLIB_VCI_GMN_H
#define SOCLIB_VCI_GMN_H

#include <systemc.h>
#include <assert.h>
#include "shared/soclib_vci_interfaces.h"
#include "shared/soclib_mapping_table.h"

///////////////////////////////////////////////////////////
//	Structure definition
//////////////////////////////////////////////////////////

#define MATCHED_ID(x) (((unsigned int)(x))>>DROP_ID_BITS)
#define MATCHED_ADDR(x) (((unsigned int)(x))>>DROP_ADDR_BITS)

template<int NB_INITIAT,
		short NB_TARGET,
		short MIN_LATENCY,
		int   FIFO_DEPTH,
		VCI_PARAM_DECLAR>

struct SOCLIB_VCI_GMN : sc_module {

//	STRUCTURAL PARAMETERS

  const char  	*NAME;		// instance name
  int	*ROUTING_TABLE;		// pointer on the routing table

//	I/O PORTS

    sc_in<bool> CLK ;
    sc_in<bool> RESETN;

    ADVANCED_VCI_TARGET<VCI_PARAM> T_VCI[NB_INITIAT];
    ADVANCED_VCI_INITIATOR<VCI_PARAM> I_VCI[NB_TARGET];

    int ADDR_MASK, DROP_ADDR_BITS;
    int ID_MASK, DROP_ID_BITS;

// REGISTERS
 

  // TARGET FSMs
  sc_signal<short int> I_ALLOC_VALUE[NB_TARGET]; // allocation register 
  sc_signal<bool> I_ALLOC_STATE[NB_TARGET]; // state of the target output port


  // INITIATOR FSMs
  sc_signal<short int> T_ALLOC_VALUE[NB_INITIAT]; // allocation register  
  sc_signal<bool> T_ALLOC_STATE[NB_INITIAT]; // state of the initiator output port

  //  CMD FIFOs

  sc_signal<int>  CMD_FIFO_DATA   [NB_TARGET][NB_INITIAT][FIFO_DEPTH];
  sc_signal<int>  CMD_FIFO_ADR    [NB_TARGET][NB_INITIAT][FIFO_DEPTH];
  sc_signal<int>  CMD_FIFO_CMD    [NB_TARGET][NB_INITIAT][FIFO_DEPTH];
  sc_signal<int>  CMD_FIFO_ID     [NB_TARGET][NB_INITIAT][FIFO_DEPTH];
  sc_signal<int>  CMD_FIFO_PTR    [NB_TARGET][NB_INITIAT];
  sc_signal<int>  CMD_FIFO_PTW    [NB_TARGET][NB_INITIAT];
  sc_signal<int>  CMD_FIFO_STATE  [NB_TARGET][NB_INITIAT];

  //  RSP FIFOs
  sc_signal<int>  RSP_FIFO_DATA   [NB_INITIAT][NB_TARGET][FIFO_DEPTH];
  sc_signal<int>  RSP_FIFO_CMD    [NB_INITIAT][NB_TARGET][FIFO_DEPTH];
  sc_signal<int>  RSP_FIFO_PTR    [NB_INITIAT][NB_TARGET];
  sc_signal<int>  RSP_FIFO_PTW    [NB_INITIAT][NB_TARGET];
  sc_signal<int>  RSP_FIFO_STATE  [NB_INITIAT][NB_TARGET];

  // TARGET DELAY_FIFOs
  sc_signal<int>  I_DELAY_VCIDATA[NB_TARGET][MIN_LATENCY];
  sc_signal<int>  I_DELAY_VCICMD [NB_TARGET][MIN_LATENCY];
  sc_signal<bool> I_DELAY_VALID  [NB_TARGET][MIN_LATENCY];
  sc_signal<short int> I_DELAY_PTR    [NB_TARGET];
  sc_signal<bool> I_DELAY_ACK    [NB_TARGET];

  //  INITIATOR DELAY_FIFOs
  sc_signal<int>  T_DELAY_VCIDATA[NB_INITIAT][MIN_LATENCY];
  sc_signal<int>  T_DELAY_VCIADR [NB_INITIAT][MIN_LATENCY];
  sc_signal<int>  T_DELAY_VCICMD [NB_INITIAT][MIN_LATENCY];
  sc_signal<int>  T_DELAY_VCIID  [NB_INITIAT][MIN_LATENCY];
  sc_signal<bool> T_DELAY_VALID  [NB_INITIAT][MIN_LATENCY];
  sc_signal<short int> T_DELAY_PTR    [NB_INITIAT];

  sc_signal<int>  GLOBAL_RSP_STATE[NB_INITIAT]; //  total number of words in RSP FIFOs
  sc_signal<int>  GLOBAL_CMD_STATE[NB_TARGET];  //  total number of words in CMD FIFOs

  // TRAFIC COUNTERS
  sc_signal<int>  RSP_COUNTER    [NB_INITIAT][NB_TARGET]; // number of transmitted RSP words
  sc_signal<int>  CMD_COUNTER    [NB_TARGET][NB_INITIAT]; // number of transmitted CMD words
/////////////////////////////////////////////////////////////
//	transition()
/////////////////////////////////////////////////////////////

void transition()
{
int 		i,j,k;
  bool 		wok, full, valid;
  int		ptr, ptw;
  int 		t_vci_command;
  int 		t_vci_id;
  int 		i_vci_cmd_id; 


  int global_rsp_temp[NB_INITIAT];
  int global_cmd_temp[NB_TARGET];

  int rsp_temp[NB_INITIAT][NB_TARGET];
  int cmd_temp[NB_TARGET][NB_INITIAT];

  ////////////////////////////////////
  //	INITIALISATION
  ////////////////////////////////////

  if (RESETN == false) {
  	for (i=0 ; i<NB_INITIAT ; i++) {
      		T_ALLOC_STATE[i]     = false;
      		T_ALLOC_VALUE[i]     = 0;
		GLOBAL_RSP_STATE[i]  = 0;
		for (k=0 ; k<MIN_LATENCY ; k++){
			T_DELAY_VALID[i][k] = false;
			T_DELAY_VCICMD[i][k] = 0;
			T_DELAY_VCIDATA[i][k] = 0;
			T_DELAY_VCIID[i][k] = 0;
		}
		T_DELAY_PTR[i] = 0;
		for (j=0; j<NB_TARGET ; j++){
			CMD_FIFO_PTR[j][i]   = 0;
			CMD_FIFO_PTW[j][i]   = 0;
			CMD_FIFO_STATE[j][i] = 0;
			CMD_COUNTER [j][i]   = 0;
		}
	}
  	for (i=0 ; i<NB_TARGET ; i++) {
      		I_ALLOC_STATE[i]     = false;
      		I_ALLOC_VALUE[i]     = 0;
		GLOBAL_CMD_STATE[i]  = 0;
		for (k=0 ; k<MIN_LATENCY; k++){
			I_DELAY_VALID[i][k] = false;
			I_DELAY_VCIDATA[i][k] = 0;
			I_DELAY_VCICMD[i][k] = 0;
		}
		I_DELAY_PTR[i] = 0;
		for (j=0 ; j<NB_INITIAT ; j++){
			RSP_FIFO_PTR[j][i]   = 0;
			RSP_FIFO_PTW[j][i]   = 0;
			RSP_FIFO_STATE[j][i] = 0;
			RSP_COUNTER [j][i]   = 0;
		}
	}
	return;
  } // end RESETN

  ////////////////////////////////////////////////////////////
  //	LOOP 0
  // initialise the temporary variables for the
  // CMD_FIFO & RSP_FIFO states
  ////////////////////////////////////////////////////////////

  for (i=0 ; i<NB_INITIAT ; i++) {
    global_rsp_temp[i]=GLOBAL_RSP_STATE[i];
    for (j=0 ; j<NB_TARGET ; j++) {
      rsp_temp[i][j]=RSP_FIFO_STATE[i][j];
    }
  }

  for (i=0 ; i<NB_TARGET ; i++) {
    global_cmd_temp[i]=GLOBAL_CMD_STATE[i];
    for (j=0 ; j<NB_INITIAT ; j++) {
      cmd_temp[i][j]=CMD_FIFO_STATE[i][j];
    }
  }

  ////////////////////////////////////////////////////////////
  //	LOOP 1
  // - write T_DELAY_FIFOs
  // - write CMD_FIFOs
  ////////////////////////////////////////////////////////////

  for (i=0 ; i<NB_INITIAT ; i++) {
      wok = T_VCI[i].CMDVAL.read();

      ptr = T_DELAY_PTR[i];

      k = (int) ROUTING_TABLE[MATCHED_ADDR(T_DELAY_VCIADR[i][ptr].read())];

      valid = T_DELAY_VALID[i][ptr];

      full = (CMD_FIFO_STATE[k][i] == FIFO_DEPTH);

      ptw = CMD_FIFO_PTW[k][i];

      // Write fifo retard if there is an empty slot
      // or the fifo cmd is not full
      if (!valid || !full) {
          // VCI command word
          t_vci_command =
              ((int)T_VCI[i].BE.read()    <<0 )+
              ((int)T_VCI[i].CMD.read()   <<4 )+
              ((int)T_VCI[i].PLEN.read()  <<6 )+
              ((int)T_VCI[i].EOP.read()   <<14)+
              ((int)T_VCI[i].CONS.read()  <<15)+
              ((int)T_VCI[i].CONTIG.read()<<16)+
              ((int)T_VCI[i].CFIXED.read()<<17)+
              ((int)T_VCI[i].WRAP.read()  <<18)+
              ((int)T_VCI[i].CLEN.read()  <<19);

          // VCI ident word
          t_vci_id =
              ((int)T_VCI[i].TRDID.read() <<0)+
              ((int)T_VCI[i].PKTID.read() <<8)+
              ((int)T_VCI[i].SRCID.read() <<16);

	T_DELAY_PTR[i]          = (ptr + 1) % MIN_LATENCY ;
	T_DELAY_VCIDATA[i][ptr] = (int)T_VCI[i].WDATA.read();
	T_DELAY_VCIADR[i][ptr]  = (int)T_VCI[i].ADDRESS.read();
	T_DELAY_VCICMD[i][ptr]  = t_vci_command;
	T_DELAY_VCIID[i][ptr]   = t_vci_id;
      }

      //  update the VALID flag in the fifo delay
      T_DELAY_VALID[i][ptr] =  wok || (T_DELAY_VALID[i][ptr] && full);

      //  Write fifo cmd when it is not full and
      //  the fifo delay has a valid data
      if (!full && valid){
	cmd_temp[k][i] += 1;
	global_cmd_temp[k] += 1;



	CMD_FIFO_DATA[k][i][ptw] = T_DELAY_VCIDATA[i][ptr];
	CMD_FIFO_ADR [k][i][ptw] = T_DELAY_VCIADR [i][ptr];
	CMD_FIFO_CMD [k][i][ptw] = T_DELAY_VCICMD [i][ptr];
	CMD_FIFO_ID  [k][i][ptw] = T_DELAY_VCIID  [i][ptr];
	CMD_FIFO_PTW [k][i]      = (ptw + 1) % FIFO_DEPTH;

	CMD_COUNTER  [k][i] = CMD_COUNTER [k][i] + 1;

        if(wok == true)
                if (i != ((int)T_VCI[i].SRCID.read())>>DROP_ID_BITS) {
                        perror("Error in the SOCLIB_VCI_GMN component\n");
                        perror("The SRCID field does not match the port index!!!\n");
                        printf("i= %d, SRCID =%d\n\n",i,(int)T_VCI[i].SRCID.read() );
                        sc_stop();
                }

      }
  } // end loop 1

  ////////////////////////////////////////////////////////////
  //	LOOP 2
  // - write I_DELAY_FIFOs
  // - write RSP_FIFOs
  ////////////////////////////////////////////////////////////

  for (j=0; j<NB_TARGET;j++) {

      ptr = I_DELAY_PTR[j];

      k = I_DELAY_VCICMD[j][ptr] >> (16+DROP_ID_BITS);

      full = (RSP_FIFO_STATE[k][j] == FIFO_DEPTH);

      valid = I_DELAY_VALID[j][ptr];

      // Write fifo retard if there is an empty slot
      // or the fifo rsp is not full
      if (!valid || !full) {
          i_vci_cmd_id =
              (int)I_VCI[j].RERROR.read()        + // RERROR = bits 2 to 0
              ((int)I_VCI[j].REOP.read()   << 3) + // REOP = bit 3
              ((int)I_VCI[j].RTRDID.read() << 4) + // RTRDID = bits 11 to 4
              ((int)I_VCI[j].RPKTID.read() << 12)+ // RPKTID = bits 15 to 12
              ((int)I_VCI[j].RSRCID.read() << 16); // RSRCID = bits 31 to 16
	I_DELAY_PTR[j]          = (ptr + 1) % MIN_LATENCY;
	I_DELAY_VCIDATA[j][ptr] = (int)I_VCI[j].RDATA.read();
	I_DELAY_VCICMD [j][ptr] = i_vci_cmd_id;
      }

      //  update the VALID flag in the fifo retard
      wok = I_VCI[j].RSPVAL.read();
      I_DELAY_VALID[j][ptr] = wok || (I_DELAY_VALID[j][ptr] && full);

      //  Write fifo rsp when it is not full and
      //  the fifo retard has a valid data
      if (!full && valid){
	rsp_temp[k][j] += 1;
	global_rsp_temp[k] += 1;

  ptw = RSP_FIFO_PTW[k][j];
	RSP_FIFO_DATA[k][j][ptw] = I_DELAY_VCIDATA[j][ptr];
	RSP_FIFO_CMD [k][j][ptw] = I_DELAY_VCICMD [j][ptr];
	RSP_FIFO_PTW [k][j]      = (ptw + 1) % FIFO_DEPTH;

	RSP_COUNTER  [k][j] = RSP_COUNTER [k][j] + 1;
      }
  } // end Loop 2

  ////////////////////////////////////////////////////////////
  //	LOOP 3
  // - Write T_ALLOC registers
  // - Read RSP FIFOs
  // The T_ALLOC FSM controls the initiator output port
  // allocation to a given target input port.
  // The allocation policy is round-robin.
  ////////////////////////////////////////////////////////////

  for (i=0; i<NB_INITIAT ; i++) {

      if (T_ALLOC_STATE[i] == false) {
	      if (GLOBAL_RSP_STATE[i] != 0) { // new allocation
		      for(j = 0 ; ((j < NB_TARGET)) ; j++) { 
			      k = (j + T_ALLOC_VALUE[i]) % NB_TARGET;
			      if (RSP_FIFO_STATE[i][k] != 0) {
			      	T_ALLOC_VALUE[i] = k;
			      	T_ALLOC_STATE[i] = true;
              break; 
			      }
		      } // end for
	      }
      } else {
	      k = T_ALLOC_VALUE[i];
	      ptr = RSP_FIFO_PTR[i][k];
	      if ((RSP_FIFO_STATE[i][k] != 0) && T_VCI[i].RSPACK.read()){
		      global_rsp_temp[i] -= 1;
		      rsp_temp[i][k] -= 1;
		      RSP_FIFO_PTR[i][k] = (RSP_FIFO_PTR[i][k] + 1) % FIFO_DEPTH;

		      if (((int)RSP_FIFO_CMD[i][k][ptr] & 0x00000008) == 0x00000008){
		        T_ALLOC_STATE[i] = false; //  desallocation
		      }

/******************************************************************************/
/*  		      if (((int)RSP_FIFO_CMD[i][k][ptr] & 0x00000008) == 0x00000008) {  */
/*  			      T_ALLOC_STATE[i] = false; //  desallocation                      */
/*  		      }                                                                 */
/******************************************************************************/
	      }
      } // end else
  } // end Loop 3

  ////////////////////////////////////////////////////////////
  //	LOOP 4
  // - Write I_ALLOC registers
  // - Read CMD FIFOs
  // The I_ALLOC FSM controls the target output port
  // allocation to a given initiator input port.
  // The allocation policy is round-robin.
  ////////////////////////////////////////////////////////////

  for (i=0 ; i<NB_TARGET ; i++) {

      if (I_ALLOC_STATE[i] == false) {
	      if (GLOBAL_CMD_STATE[i] != 0) { // new allocation
		      for(j = 0 ; (j < NB_INITIAT) ; j++) { 
			      k = (j + I_ALLOC_VALUE[i]) % NB_INITIAT;
			      if (CMD_FIFO_STATE[i][k] != 0) {
			      	I_ALLOC_VALUE[i] = k;
			      	I_ALLOC_STATE[i] = true;
              break;
			      }
		      } // end for
	      }
      } else {
	      k = I_ALLOC_VALUE[i];
	      ptr = CMD_FIFO_PTR[i][k];
	      if ((CMD_FIFO_STATE[i][k] != 0) && I_VCI[i].CMDACK.read()){
		      global_cmd_temp[i] -= 1;
		      cmd_temp[i][k] -= 1;
		      CMD_FIFO_PTR[i][k] = (CMD_FIFO_PTR[i][k] + 1) % FIFO_DEPTH;
		      if (((int)CMD_FIFO_CMD[i][k][ptr] & 0x00004000) == 0x00004000) {
			      I_ALLOC_STATE[i] = false; //  desallocation
		      }

/******************************************************************************/
/*  		      if (((int)CMD_FIFO_CMD[i][k][ptr] & 0x00004000) == 0x00004000) {  */
/*  			      I_ALLOC_STATE[i] = false; //  desallocation                      */
/*  		      }                                                                 */
/******************************************************************************/
	      }
      } // end else
  } // end Loop 4

  ////////////////////////////////////////////////////////////
  //	LOOP 5
  // - Write FIFO RSP & FIFO CMD global states
  ////////////////////////////////////////////////////////////

  for (i=0 ; i<NB_INITIAT ; i++) {
    GLOBAL_RSP_STATE[i] = global_rsp_temp[i];
    for (j=0 ; j<NB_TARGET ; j++) {
      RSP_FIFO_STATE[i][j] = rsp_temp[i][j];
    }
  }

  for (i=0 ; i<NB_TARGET ; i++) {
    GLOBAL_CMD_STATE[i] = global_cmd_temp[i];
    for (j=0 ; j<NB_INITIAT ; j++) {
      CMD_FIFO_STATE[i][j] = cmd_temp[i][j];
    }
  }
};  // end transition()

//////////////////////////////////////////////////////:
//	genMoore()
//////////////////////////////////////////////////////:

void genMoore()
{
int	i,j,k;
	unsigned char	ptr;
  int   cmd, id;
	bool	full,valid;

  //////////////////////////////////
  //  loop on the initiator ports
  //////////////////////////////////

  for (i=0 ; i<NB_INITIAT ; i++) {

     k 	   = T_ALLOC_VALUE[i];
     ptr   = RSP_FIFO_PTR[i][k];
     cmd   = RSP_FIFO_CMD[i][k][ptr];

     if ((T_ALLOC_STATE[i] == true) && (RSP_FIFO_STATE[i][k] !=0)) {
       T_VCI[i].RSPVAL	= true;
       T_VCI[i].RERROR	= (sc_uint<ERRSIZE>) 	(cmd	& 0x00000007);
       T_VCI[i].REOP	= (bool) 		(cmd>>3	& 0x00000001);
       T_VCI[i].RTRDID	= (sc_uint<TRDIDSIZE>)	(cmd>>4 & 0x000000FF);
       T_VCI[i].RPKTID	= (sc_uint<PKTIDSIZE>)	(cmd>>12 & 0x0000000F);
       T_VCI[i].RSRCID	= (sc_uint<SRCIDSIZE>)	(cmd>>16 & 0x0000FFFF);
       T_VCI[i].RDATA	= (sc_uint<8*CELLSIZE>) RSP_FIFO_DATA[i][k][ptr];
     }else{
       T_VCI[i].RSPVAL	= false;
#if 0 // DISABLED USELESS SIGNALS
       T_VCI[i].RERROR	= (sc_uint<ERRSIZE>) 0; 
       T_VCI[i].REOP	= false;
       T_VCI[i].RTRDID	= (sc_uint<TRDIDSIZE>)    0;
       T_VCI[i].RPKTID	= (sc_uint<PKTIDSIZE>)    0;
       T_VCI[i].RSRCID	= (sc_uint<SRCIDSIZE>)	   0;
       T_VCI[i].RDATA	= (sc_uint<8*CELLSIZE>)  0;
#endif
     }

     ptr = T_DELAY_PTR[i];

     k = (int)ROUTING_TABLE[MATCHED_ADDR(T_DELAY_VCIADR[i][ptr].read())];
     full = (CMD_FIFO_STATE[k][i] == FIFO_DEPTH);
     valid = T_DELAY_VALID[i][ptr];

     T_VCI[i].CMDACK =  (bool) (!valid ||  !full);

  } // end loop initiators

  //////////////////////////////////
  //	loop on the target ports
  //////////////////////////////////

  for (j=0 ; j<NB_TARGET ; j++) {

     k      = I_ALLOC_VALUE[j];
     ptr    = CMD_FIFO_PTR[j][k];
     cmd    = CMD_FIFO_CMD[j][k][ptr];
     id     = CMD_FIFO_ID [j][k][ptr];

     if ((I_ALLOC_STATE[j] == true) && (CMD_FIFO_STATE[j][k] !=0)) {
       I_VCI[j].CMDVAL	= true;
       I_VCI[j].ADDRESS = (sc_uint<ADDRSIZE>) 	CMD_FIFO_ADR[j][k][ptr];
       I_VCI[j].WDATA   = (sc_uint<8*CELLSIZE>) CMD_FIFO_DATA[j][k][ptr];
       I_VCI[j].BE      = (sc_uint<CELLSIZE>) 	(cmd & 0xF);
       I_VCI[j].CMD     = (sc_uint<2>)		(cmd>>4 & 0x3);
       I_VCI[j].PLEN    = (sc_uint<PLENSIZE>)	(cmd>>6 & 0x3);
       I_VCI[j].EOP     = (bool) 		(cmd>>14 & 0x1);
       I_VCI[j].CONS    = (bool) 		(cmd>>15 & 0x1);
       I_VCI[j].CONTIG  = (bool) 		(cmd>>16 & 0x1);
       I_VCI[j].CFIXED  = (bool) 		(cmd>>17 & 0x1);
       I_VCI[j].WRAP    = (bool) 		(cmd>>18 & 0x1);
       I_VCI[j].CLEN    = (sc_uint<CLENSIZE>)	(cmd>>19 & 0xFF);
       I_VCI[j].TRDID   = (sc_uint<TRDIDSIZE>)	(id & 0xFF);
       I_VCI[j].PKTID   = (sc_uint<PKTIDSIZE>)	(id>>8 & 0xF);
       I_VCI[j].SRCID   = (sc_uint<SRCIDSIZE>)	(id>>16 & 0xFFFF);
     }else{
       I_VCI[j].CMDVAL	= false;
#if 0 // DISABLED USELESS SIGNALS
       I_VCI[j].ADDRESS = (sc_uint<ADDRSIZE>)	0;
       I_VCI[j].WDATA   = (sc_uint<8*CELLSIZE>)	0;
       I_VCI[j].BE      = (sc_uint<CELLSIZE>)		0;
       I_VCI[j].CMD     = (sc_uint<2>)	0;
       I_VCI[j].PLEN    = (sc_uint<PLENSIZE>)	0;
       I_VCI[j].EOP     = false;
       I_VCI[j].CONS    = false;
       I_VCI[j].CONTIG  = false;
       I_VCI[j].CFIXED  = false;
       I_VCI[j].WRAP    = false;
       I_VCI[j].CLEN    = (sc_uint<CLENSIZE>)	0;
       I_VCI[j].TRDID   = (sc_uint<TRDIDSIZE>)		0;
       I_VCI[j].PKTID   = (sc_uint<PKTIDSIZE>)		0;
       I_VCI[j].SRCID   = (sc_uint<SRCIDSIZE>)		0;
#endif
     }

     ptr = I_DELAY_PTR[j];
     k = I_DELAY_VCICMD[j][ptr] >> (16+DROP_ID_BITS);
     full = (RSP_FIFO_STATE[k][j] == FIFO_DEPTH);
     valid = I_DELAY_VALID[j][ptr];

     I_VCI[j].RSPACK =  (!valid) ||  (!full);

  } // end loop targets
};  // end genMoore()

////////////////////////////////////////////////
//	constructor
////////////////////////////////////////////////

SC_HAS_PROCESS(SOCLIB_VCI_GMN);

SOCLIB_VCI_GMN (sc_module_name 		insname,
                SOCLIB_MAPPING_TABLE &mapping_table)
{
#ifdef NONAME_RENAME
	char newname[100];

	for (int i=0;i<NB_TARGET;i++)
	{
		sprintf(newname,"I_ALLOC_VALUE_%2.2d",i);
		I_ALLOC_VALUE[i].rename(newname); // allocation register
		sprintf(newname,"I_ALLOC_STATE_%2.2d",i);
		I_ALLOC_STATE[i].rename(newname); // state of the target output port
	}

	for (int i=0;i<NB_INITIAT;i++)
	{
		sprintf(newname,"T_ALLOC_VALUE_%2.2d",i);
		T_ALLOC_VALUE[i].rename(newname); // allocation register
		sprintf(newname,"T_ALLOC_STATE_%2.2d",i);
		T_ALLOC_STATE[i].rename(newname); // state of the initiator output port
	}

	for (int i=0;i<NB_TARGET;i++)
	{
		for (int j=0;j<NB_INITIAT;j++)
		{
			for (int k=0;k<FIFO_DEPTH;k++)
			{
				sprintf(newname,"CMD_FIFO_DATA_%2.2d_%2.2d_%2.2d",i,j,k);
				CMD_FIFO_DATA   [i][j][k].rename(newname);
				sprintf(newname,"CMD_FIFO_ADR_%2.2d_%2.2d_%2.2d",i,j,k);
				CMD_FIFO_ADR   [i][j][k].rename(newname);
				sprintf(newname,"CMD_FIFO_CMD_%2.2d_%2.2d_%2.2d",i,j,k);
				CMD_FIFO_CMD   [i][j][k].rename(newname);
				sprintf(newname,"CMD_FIFO_ID_%2.2d_%2.2d_%2.2d",i,j,k);
				CMD_FIFO_ID   [i][j][k].rename(newname);
			}
			sprintf(newname,"CMD_FIFO_PTR_%2.2d_%2.2d",i,j);
			CMD_FIFO_PTR   [i][j].rename(newname);
			sprintf(newname,"CMD_FIFO_PTW_%2.2d_%2.2d",i,j);
			CMD_FIFO_PTW   [i][j].rename(newname);
			sprintf(newname,"CMD_FIFO_STATE_%2.2d_%2.2d",i,j);
			CMD_FIFO_STATE   [i][j].rename(newname);
			sprintf(newname,"CMD_COUNTER_%2.2d_%2.2d",i,j);
			CMD_COUNTER   [i][j].rename(newname);
		}
	}

        for (int i=0;i<NB_INITIAT;i++)
        {
                for (int j=0;j<NB_TARGET;j++)
                {
                        for (int k=0;k<FIFO_DEPTH;k++)
                        {
                                sprintf(newname,"RSP_FIFO_DATA_%2.2d_%2.2d_%2.2d",i,j,k);
                                RSP_FIFO_DATA   [i][j][k].rename(newname);
                                sprintf(newname,"RSP_FIFO_CMD_%2.2d_%2.2d_%2.2d",i,j,k);
                                RSP_FIFO_CMD   [i][j][k].rename(newname);
                        }
                        sprintf(newname,"RSP_FIFO_PTR_%2.2d_%2.2d",i,j);
                        RSP_FIFO_PTR   [i][j].rename(newname);
                        sprintf(newname,"RSP_FIFO_PTW_%2.2d_%2.2d",i,j);
                        RSP_FIFO_PTW   [i][j].rename(newname);
                        sprintf(newname,"RSP_FIFO_STATE_%2.2d_%2.2d",i,j);
                        RSP_FIFO_STATE   [i][j].rename(newname);
                        sprintf(newname,"RSP_COUNTER_%2.2d_%2.2d",i,j);
                        RSP_COUNTER   [i][j].rename(newname);
                }
        }

        for (int i=0;i<NB_TARGET;i++)
        {
                for (int j=0;j<MIN_LATENCY;j++)
                {
                        sprintf(newname,"I_DELAY_VCIDATA_%2.2d_%2.2d",i,j);
                        I_DELAY_VCIDATA   [i][j].rename(newname);
                        sprintf(newname,"I_DELAY_VCICMD_%2.2d_%2.2d",i,j);
                        I_DELAY_VCICMD   [i][j].rename(newname);
                        sprintf(newname,"I_DELAY_VALID_%2.2d_%2.2d",i,j);
                        I_DELAY_VALID   [i][j].rename(newname);
                }
                sprintf(newname,"I_DELAY_PTR_%2.2d",i);
                I_DELAY_PTR   [i].rename(newname);
                sprintf(newname,"I_DELAY_ACK_%2.2d",i);
                I_DELAY_ACK   [i].rename(newname);
                sprintf(newname,"GLOBAL_CMD_STATE_%2.2d",i);
                GLOBAL_CMD_STATE   [i].rename(newname);
        }

        for (int i=0;i<NB_INITIAT;i++)
        {
                for (int j=0;j<MIN_LATENCY;j++)
                {
                        sprintf(newname,"T_DELAY_VCIDATA_%2.2d_%2.2d",i,j);
                        T_DELAY_VCIDATA   [i][j].rename(newname);
                        sprintf(newname,"T_DELAY_VCIADR_%2.2d_%2.2d",i,j);
                        T_DELAY_VCIADR   [i][j].rename(newname);
                        sprintf(newname,"T_DELAY_VCICMD_%2.2d_%2.2d",i,j);
                        T_DELAY_VCICMD   [i][j].rename(newname);
                        sprintf(newname,"T_DELAY_VCIID_%2.2d_%2.2d",i,j);
                        T_DELAY_VCIID   [i][j].rename(newname);
                        sprintf(newname,"T_DELAY_VALID_%2.2d_%2.2d",i,j);
                        T_DELAY_VALID   [i][j].rename(newname);
                }
                sprintf(newname,"T_DELAY_PTR_%2.2d",i);
                T_DELAY_PTR   [i].rename(newname);
                sprintf(newname,"GLOBAL_RSP_STATE_%2.2d",i);
                GLOBAL_RSP_STATE   [i].rename(newname);
        }

#endif

  SC_METHOD(transition);
  dont_initialize();
  sensitive_pos << CLK;

  SC_METHOD(genMoore);
  dont_initialize();
  sensitive_neg << CLK;

  NAME  = (const char*) insname;

  //	Routing Table allocation and initialisation

  mapping_table.localParams( 0,
                             DROP_ADDR_BITS, ADDR_MASK,
                             DROP_ID_BITS, ID_MASK );


  ROUTING_TABLE = new int [ADDR_MASK+1];
  mapping_table.initLocalRoutingTable(0, intList(-1), ROUTING_TABLE);

  //	Checking structural parameters

  if (NB_INITIAT > 255) {
	  perror("Error in the SOCLIB_VCI_GMN component\n");
	  perror("The NB_INITIAT parameter cannot be larger than 256\n");
	  sc_stop();
  }
  if (NB_TARGET > 255){
	  perror("Error in the SOCLIB_VCI_GMN component\n");
	  perror("The NB_TARGET parameter cannot be larger than 256\n");
	  sc_stop();
  }
  if (FIFO_DEPTH <= 0){
	  perror("Error in the SOCLIB_VCI_GMN component\n");
	  perror("The FIFO_DEPTH parameter must be larger than 0\n");
	  sc_stop();
  }
  if (MIN_LATENCY <= 0){
	  perror("Error in the SOCLIB_VCI_GMN component\n");
	  perror("The MIN_LATENCY parameter must be larger than 0\n");
	  sc_stop();
  }

  if (ERRSIZE >3){
	  perror("Error in the SOCLIB_VCI_GMN component\n");
  	  perror("The ERRSIZE parameter cannot be larger than 3\n");
	  sc_stop();
  }

  if (PKTIDSIZE > 4){
  	perror("Error in the SOCLIB_VCI_GMN component\n");
	perror("The PKTIDSIZE parameter cannot be larger than 4");
	sc_stop();
	}

  if (TRDIDSIZE > 8){
  	perror("Error in the SOCLIB_VCI_GMN component\n");
	perror("The TRDIDSIZE parameter cannot be larger than 8");
	sc_stop();
	}

  if (SRCIDSIZE > 16){
  	perror("Error in the SOCLIB_VCI_GMN component\n");
	perror("The SCRIDSIZE parameter cannot be larger than 16");
	sc_stop();
	}

  printf("Successful Instanciation of SOCLIB_VCI_GMN : %s\n", NAME);

};  // end constructor

}; // end structure SOCLIB_VCI_GMN

#undef MATCHED_ID
#undef MATCHED_ADDR
#endif
