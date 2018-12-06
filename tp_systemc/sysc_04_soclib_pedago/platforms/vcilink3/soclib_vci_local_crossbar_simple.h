//////////////////////////////////////////////////////////////////////////
// File : soclib_vci_local_crossbar.h
// Date : 15/04/2004
// author :  Alain Greiner
// Copyright : UPMC - LIP6
//
// This component is a modified version of the generic VCI ADVANCED local crossbar.
// It can be used in a one-level hierarchical interconnect.
//
// This component contains two fully independant crossbars :
// - The requests crossbar contains as many multiplexors as the number
// of initiator ports (i.e. NB_TARGET) . Each multiplexor is controled
// by a private FSM. Each initiator port is either "not allocated",
// or "allocated" to a single target port.
// - The response crossbar contains as many multiplexors as the number
// of target ports (i.e. NB_INIT) . Each multiplexor is controled
// by a private FSM. Each target port is either "not allocated",
// or "allocated" to a single initiator port.
//
// The CMDACK and RSPACK signals are not controled by the corresponding FSM but by every FSM of the other type,
// using a logical door .
//
//
// Only the FSM where the packet is supposed to go is allocated to the VCI port the packet comes from.
// The FSM where it comes from is not aware that an FSM is allocated to its port, therefore it can be allocated
// to an other port.
//
// Those crossbar are not "fully connected":
// - a local target port can be connected to a local initiator port.
// - a local target port can be connected to a micro-network initiator port.
//
// There is no buffer to store VCI requests or responses in this component,
// and all output signals are Mealy signals.
//
//////////////////////////////////////////////////////////////////////////
// This component has three "constructor" parameters :
// - char*          name         : instance name
// - int*           indexes      : global index of the subsystem
// - SOCLIB_MAPPING_TABLE	segtab  : the segment table
//////////////////////////////////////////////////////////////////////////
// This component has 10 "template" parameters :
// - int NB_INIT        : number of VCI initiators
// - int NB_TARGET      : number of VCI targets
//
//  (Using shorthand VCI_PARAM for these ones)
// - int ADDRSIZE	: number of bits for the VCI ADDRESS field
// - int CELLSIZE	: number of bytes for the VCI DATA field
// - int ERRSIZE	: number of extra bits for the VCI ERROR field
// - int PLENSIZE	: number of extra bits for the VCI PLEN field
// - int CLENSIZE	: number of extra bits for the VCI CLEN field
// - int SRCIDSIZE     	: number of bits for the VCI SRCID field
// - int TRDIDSIZE     	: number of bits for the VCI TRDID field
// - int PKTIDSIZE     	: number of bits for the VCI PKTID field
//////////////////////////////////////////////////////////////////////////

#ifndef SOCLIB_VCI_LOCAL_CROSSBAR_SIMPLE_H
#define SOCLIB_VCI_LOCAL_CROSSBAR_SIMPLE_H

#include <systemc.h>
#include "shared/soclib_vci_interfaces.h"
#include "shared/soclib_mapping_table.h"

///////////////////////////////////////////////////////////
//	Structure definition
//////////////////////////////////////////////////////////

template<int NB_INIT,
         int NB_TARGET,
         VCI_PARAM_DECLAR>

struct SOCLIB_VCI_LOCAL_CROSSBAR_SIMPLE : sc_module{

  // STRUCTURAL PARAMETERS

  const char *NAME; // instance name
  int *GLOBAL_ROUTING_TABLE;
  int *LOCAL_ROUTING_TABLE;
  int GLOBAL_ADDR_OFFSET, GLOBAL_ADDR_MASK, GLOBAL_ADDR;
  int LOCAL_ADDR_OFFSET, LOCAL_ADDR_MASK;
  int GLOBAL_ID_OFFSET, GLOBAL_ID_MASK, GLOBAL_ID;
  int LOCAL_ID_OFFSET, LOCAL_ID_MASK;

#define GLOBAL_ADDR_OF(x) (((x)>>GLOBAL_ADDR_OFFSET)&GLOBAL_ADDR_MASK)
#define LOCAL_ADDR_OF(x) (((x)>>LOCAL_ADDR_OFFSET)&LOCAL_ADDR_MASK)
#define GLOBAL_ID_OF(x) (((x)>>GLOBAL_ID_OFFSET)&GLOBAL_ID_MASK)
#define LOCAL_ID_OF(x) (((x)>>LOCAL_ID_OFFSET)&LOCAL_ID_MASK)

  // I/O PORTS

  sc_in<bool> CLK ;
  sc_in<bool> RESETN;
  // VCI Target ports connected to the local initiators
  ADVANCED_VCI_TARGET<VCI_PARAM> TLOC_VCI[NB_INIT];
  // VCI Initiator ports connected to the local targets
  ADVANCED_VCI_INITIATOR<VCI_PARAM> ILOC_VCI[NB_TARGET];

  // REGISTERS

  // TLOC FSMs
  sc_signal<bool> TLOC_ALLOCATED[NB_INIT]; // state of the local target ports
  sc_signal<int>  TLOC_INDEX[NB_INIT];


  // ILOC FSMs
  sc_signal<bool> ILOC_ALLOCATED[NB_TARGET]; // state of the local initiator ports
  sc_signal<int>  ILOC_INDEX[NB_TARGET];

  ////////////////////////////////////////////////
  //	constructor
  ////////////////////////////////////////////////

  SC_HAS_PROCESS(SOCLIB_VCI_LOCAL_CROSSBAR_SIMPLE);

  SOCLIB_VCI_LOCAL_CROSSBAR_SIMPLE(sc_module_name insname,
                                   int *indexes,
                                   SOCLIB_MAPPING_TABLE mapping_table)
  {
    int level;
#ifdef SOCVIEW
    char newname[100];

    for (int i=0;i<NB_INIT;i++)
      {
        sprintf(newname,"TLOC_ALLOCATED_%2.2d",i);
        TLOC_ALLOCATED[i].rename(newname);
        sprintf(newname,"TLOC_INDEX_%2.2d",i);
        TLOC_INDEX[i].rename(newname);
      }

    for (int i=0;i<NB_TARGET;i++)
      {
        sprintf(newname,"ILOC_ALLOCATED_%2.2d",i);
        ILOC_ALLOCATED[i].rename(newname); // allocation register
        sprintf(newname,"ILOC_INDEX_%2.2d",i);
        ILOC_INDEX[i].rename(newname); // state of the target output port
      }
#endif
    SC_METHOD(transition);
    sensitive << CLK.pos();

    SC_METHOD(genMealy);
    sensitive << CLK.neg();

    for (int i = 0 ; i < NB_TARGET ; i++) {
      sensitive << 	ILOC_VCI[i].CMDACK;
      sensitive << 	ILOC_VCI[i].RSPVAL;
      sensitive << 	ILOC_VCI[i].RDATA;
      sensitive << 	ILOC_VCI[i].REOP;
      sensitive << 	ILOC_VCI[i].RERROR;
      sensitive << 	ILOC_VCI[i].RTRDID;
      sensitive << 	ILOC_VCI[i].RPKTID;
      sensitive << 	ILOC_VCI[i].RSRCID;
    }

    for (int i = 0 ; i < NB_INIT ; i++) {
      sensitive << 	TLOC_VCI[i].RSPACK;
      sensitive << 	TLOC_VCI[i].CMDVAL;
      sensitive << 	TLOC_VCI[i].ADDRESS;
      sensitive << 	TLOC_VCI[i].WDATA;
      sensitive << 	TLOC_VCI[i].CMD;
      sensitive << 	TLOC_VCI[i].EOP;
      sensitive << 	TLOC_VCI[i].PLEN;
      sensitive << 	TLOC_VCI[i].CONS;
      sensitive << 	TLOC_VCI[i].CONTIG;
      sensitive << 	TLOC_VCI[i].CFIXED;
      sensitive << 	TLOC_VCI[i].WRAP;
      sensitive << 	TLOC_VCI[i].CLEN;
      sensitive << 	TLOC_VCI[i].TRDID;
      sensitive << 	TLOC_VCI[i].PKTID;
      sensitive << 	TLOC_VCI[i].SRCID;
    }

    NAME  = (const char*) insname;
    for (level = 0; indexes[level]!=-1; ++level)
      /**/;
    GLOBAL_ID = mapping_table.ident(level, indexes);
    NAME  = (const char*) insname;
    mapping_table.paramsTo( level,
                            GLOBAL_ADDR_OFFSET, GLOBAL_ADDR_MASK,
                            GLOBAL_ID_OFFSET, GLOBAL_ID_MASK);
    mapping_table.localParams( level,
                               LOCAL_ADDR_OFFSET, LOCAL_ADDR_MASK,
                               LOCAL_ID_OFFSET, LOCAL_ID_MASK );

    GLOBAL_ROUTING_TABLE = new int[GLOBAL_ADDR_MASK+1];
    mapping_table.initRoutingTableTo(level, indexes, GLOBAL_ROUTING_TABLE);

    LOCAL_ROUTING_TABLE = new int[LOCAL_ADDR_MASK+1];
    mapping_table.initLocalRoutingTable(level, indexes, LOCAL_ROUTING_TABLE);

    //	Checking structural parameters

    if ((NB_TARGET > 16) || (NB_TARGET < 1)) {
      printf("Error in the SOCLIB_VCI_LOCAL_CROSSBAR component : %s\n", NAME);
      printf("The NB_TARGET parameter cannot be larger than 16\n");
      sc_stop();
    }

    if ((NB_INIT > 16) || (NB_INIT < 1)) {
      printf("Error in the SOCLIB_VCI_LOCAL_CROSSBAR component : %s\n", NAME);
      printf("The NB_INIT parameter cannot be larger than 16\n");
      sc_stop();
    }

    printf("Successful Instanciation of SOCLIB_VCI_LOCAL_CROSSBAR : %s\n", NAME);

  };  // end constructor

  /////////////////////////////////////////////////////////////
  //	transition()
  /////////////////////////////////////////////////////////////

  void transition() {
    // INITIALISATION
    if (RESETN == false) {
      for (int n = 0 ; n < NB_TARGET ; n++) {
        ILOC_ALLOCATED[n] = false;
        ILOC_INDEX[n]     = 0;
      }
      for (int n = 0 ; n < NB_INIT ; n++) {
        TLOC_ALLOCATED[n] = false;
        TLOC_INDEX[n]     = 0;
      }
      return;
    } // end RESETN

    //	Loop on the TLOC FSMs

    for(int t = 0 ; t < NB_INIT ; t++) {
      if(TLOC_ALLOCATED[t] == false) { // local target port not allocated
        for(int i = 0 ; i < NB_TARGET ; i++) {
          int l = (i + TLOC_INDEX[t] + 1) % NB_TARGET;
          if(ILOC_VCI[l].RSPVAL == true) {
            int id = (int)ILOC_VCI[l].RSRCID.read();
            int global_index = GLOBAL_ID_OF(id);
            int local_index = LOCAL_ID_OF(id);
            if ((global_index == GLOBAL_ID) && (local_index == t)) {
              TLOC_ALLOCATED[t] = true;
              TLOC_INDEX[t] = l;
              break;
            }
          }
        } // end for i
      } else {			// local target port allocated
        // it is allocated to a local initiator port
        if((ILOC_VCI[TLOC_INDEX[t]].RSPVAL == true) &&
           (TLOC_VCI[t].RSPACK == true) &&
           (ILOC_VCI[TLOC_INDEX[t]].REOP == true)) { TLOC_ALLOCATED[t] = false; }
      }
    } // end for t

    //	Loop on the ILOC FSMs

    for(int i = 0 ; i < NB_TARGET ; i++) {
      if(ILOC_ALLOCATED[i] == false) { // local initiator port not allocated
        for(int t = 0 ; t < NB_INIT ; t++) {
          int u = (t + ILOC_INDEX[i] + 1) % NB_INIT;
          if(TLOC_VCI[u].CMDVAL == true) {
            unsigned int address = TLOC_VCI[u].ADDRESS.read();
            int global_index = GLOBAL_ROUTING_TABLE[GLOBAL_ADDR_OF(address)];
            int local_index = LOCAL_ROUTING_TABLE[LOCAL_ADDR_OF(address)];
            if (local_index == i && global_index == GLOBAL_ID) {
              ILOC_ALLOCATED[i] = true;
              ILOC_INDEX[i] = u;

              break;
            }
          }
        } // end for t
      } else {			// local initiator port allocated
        if((TLOC_VCI[ILOC_INDEX[i]].CMDVAL == true) &&
           (ILOC_VCI[i].CMDACK == true) &&(TLOC_VCI[ILOC_INDEX[i]].EOP == true))
          { ILOC_ALLOCATED[i] = false;}
      }
    } // end for i


  };  // end transition()

  //////////////////////////////////////////////////////:
  //	genMealy()
  //////////////////////////////////////////////////////:

  void genMealy()
  {

    //  VCI local target ports

    for (int i=0 ; i<NB_INIT ; i++) {
      if (TLOC_ALLOCATED[i] == true) { // allocated
        int k = TLOC_INDEX[i];

        TLOC_VCI[i].RSPVAL	= ILOC_VCI[k].RSPVAL.read();
        TLOC_VCI[i].RERROR	= ILOC_VCI[k].RERROR.read();
        TLOC_VCI[i].REOP	= ILOC_VCI[k].REOP.read();
        TLOC_VCI[i].RTRDID	= ILOC_VCI[k].RTRDID.read();
        TLOC_VCI[i].RPKTID	= ILOC_VCI[k].RPKTID.read();
        TLOC_VCI[i].RSRCID	= ILOC_VCI[k].RSRCID.read();
        TLOC_VCI[i].RDATA	= ILOC_VCI[k].RDATA.read();

      } else { // not allocated
        TLOC_VCI[i].RSPVAL	= false;
        TLOC_VCI[i].RERROR	= 0;
        TLOC_VCI[i].REOP	= false;
        TLOC_VCI[i].RTRDID	= 0;
        TLOC_VCI[i].RPKTID	= 0;
        TLOC_VCI[i].RSRCID	= 0;
        TLOC_VCI[i].RDATA	= 0;
      }
    }//endfor

    //  VCI local initiator ports

    for (int i=0 ; i<NB_TARGET ; i++) {
      if (ILOC_ALLOCATED[i] == true) { // allocated
        int j = ILOC_INDEX[i];

        ILOC_VCI[i].CMDVAL	 =	TLOC_VCI[j].CMDVAL.read();
        ILOC_VCI[i].ADDRESS =	TLOC_VCI[j].ADDRESS.read();
        ILOC_VCI[i].WDATA	 =	TLOC_VCI[j].WDATA.read();
        ILOC_VCI[i].BE	 =	TLOC_VCI[j].BE.read();
        ILOC_VCI[i].CMD  	 =	TLOC_VCI[j].CMD.read();
        ILOC_VCI[i].PLEN	 =	TLOC_VCI[j].PLEN.read();
        ILOC_VCI[i].EOP	 =	TLOC_VCI[j].EOP.read();
        ILOC_VCI[i].CONS	 =	TLOC_VCI[j].CONS.read();
        ILOC_VCI[i].CONTIG	 =	TLOC_VCI[j].CONTIG.read();
        ILOC_VCI[i].CFIXED	 =	TLOC_VCI[j].CFIXED.read();
        ILOC_VCI[i].WRAP	 =	TLOC_VCI[j].WRAP.read();
        ILOC_VCI[i].CLEN	 =	TLOC_VCI[j].CLEN.read();
        ILOC_VCI[i].TRDID	 =	TLOC_VCI[j].TRDID.read();
        ILOC_VCI[i].PKTID	 =	TLOC_VCI[j].PKTID.read();
        ILOC_VCI[i].SRCID	 =	TLOC_VCI[j].SRCID.read();
      } else {  // not allocated
        ILOC_VCI[i].CMDVAL  = 	false;
        ILOC_VCI[i].ADDRESS = 	0;
        ILOC_VCI[i].WDATA   = 	0;
        ILOC_VCI[i].BE      = 	0;
        ILOC_VCI[i].CMD     = 	0;
        ILOC_VCI[i].PLEN    = 	0;
        ILOC_VCI[i].EOP     = 	false;
        ILOC_VCI[i].CONS    = 	false;
        ILOC_VCI[i].CONTIG  = 	false;
        ILOC_VCI[i].CFIXED  = 	false;
        ILOC_VCI[i].WRAP    = 	false;
        ILOC_VCI[i].CLEN    = 	0;
        ILOC_VCI[i].TRDID   = 	0;
        ILOC_VCI[i].PKTID   = 	0;
        ILOC_VCI[i].SRCID   = 	0;
      }
    } // end for

    // Each ack output is controlled by the FSM that is allocated on the VCI port at this time

    //for the ILOCs
    bool iloc_rspack;
    for(int j = 0; j<NB_TARGET; j++){
      iloc_rspack = 0;
      for ( int i = 0; i<NB_INIT; i++){
        iloc_rspack = iloc_rspack | ((TLOC_VCI[i].RSPACK)&&(TLOC_ALLOCATED[i])&&(TLOC_INDEX[i]==j));
      }
      ILOC_VCI[j].RSPACK=iloc_rspack;
    }

    //for the TLOCs
    bool tloc_cmdack;
    for(int i = 0; i < NB_INIT; i++){
      tloc_cmdack = 0;
      for ( int j = 0; j<NB_TARGET; j++){
        tloc_cmdack = tloc_cmdack | ((ILOC_VCI[j].CMDACK)&&(ILOC_ALLOCATED[j])&&(ILOC_INDEX[j]==i));
      }
      TLOC_VCI[i].CMDACK=tloc_cmdack;
    }


  };  // end genMealy()


}; // end structure SOCLIB_VCI_LOCAL_CROSSBAR_SIMPLE

#undef LOCAL_PORT_ALLOCATED
#undef GLOBAL_ADDR_OF
#undef LOCAL_ADDR_OF
#undef GLOBAL_ID_OF
#undef LOCAL_ID_OF

#endif
