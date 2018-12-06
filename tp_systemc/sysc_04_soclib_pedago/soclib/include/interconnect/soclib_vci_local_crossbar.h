//////////////////////////////////////////////////////////////////////////
// File : soclib_vci_local_crossbar.h
// Date : 15/04/2004
// author :  Alain Greiner 
// Copyright : UPMC - LIP6
//
// This component is a generic VCI ADVANCED local crossbar.
// It can be used in a two-level hierarchical interconnect, where a VCI
// micro-network interconnects several sub-systems. Each sub-system contains
// several VCI targets and several VCI initiators, but each sub-system 
// is connected to the micro-network by a single VCI target port 
// and a single VCI initiator port. 
//
// For a sub-system, the local crossbar replaces both the local system bus,
// and the bridge between the micro-network and the local bus.
// The number of VCI targets is a parameter. It cannot be larger than 16, 
// and the VCI targets must be locally indexed from 0 to (NB_TARGET - 1).
// The number of VCI initiators is a parameter. It cannot be larger than 16, 
// and the VCI initiators must be locally indexed from 0 to (NB_INIT - 1).
// 
// Therefore, this component has a single VCI target port to the micro-network,
// a single VCI initiator port to the micro-network, NB_TARGET local VCI initiator 
// ports and NB_INIT local VCI target ports.
// 
// In order to route the requests, this component decodes both the MSB bits and
// LSB bits of the VCI ADDRESS. 
// The MSB fiels is decoded to decide if the request is for a local target
// or for a remote target. In case of a remote target, the request is sent
// to the micro-network initiator port. In case of a local target, the LSB
// field of the VCI is directly used to select the target. 
// The component contains a local routing table (ROM) to decode the MSB and LSB bits 
// of the VCI ADDRESS field. This ROM is initialised by the constructor, 
// thanks to the memory mapping defined in the segment table.
// If several local initiators are requesting the same target, the allocation
// policy is round robin.
//
// In order to route the responses, this component uses both the MSB and
// LSB bits of the VCI RSRCID.
// The MSB bits of SRCID define the global index (subsystem index), i
// and the LSB bits define the local initiator index.
//
// This component contains two fully independant crossbars : 
// - The requests crossbar contains as many multiplexors as the number
// of initiator ports (i.e. NB_TARGET + 1) . Each multiplexor is controled
// by a private FSM. Each initiator port is either "not allocated", 
// or "allocated" to a single target port.
// - The response crossbar contains as many multiplexors as the number
// of target ports (i.e. NB_INIT + 1) . Each multiplexor is controled
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
// - a micro-network target port can be connected to a local initiator port.
// - a micro-network target port cannot be connected to a micro-network initiator port.
//
// There is no buffer to store VCI requests or responses in this component,
// and all output signals are Mealy signals.
//
// Implementation note : When a local port is allocated to a micro-network
// port, the index value is set to LOCAL_PORT_ALLOCATED.
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

#ifndef SOCLIB_VCI_LOCAL_CROSSBAR_H
#define SOCLIB_VCI_LOCAL_CROSSBAR_H

#include <systemc.h>
#include "shared/soclib_vci_interfaces.h"
#include "shared/soclib_mapping_table.h"

#define LOCAL_PORT_ALLOCATED (1024)

///////////////////////////////////////////////////////////
//	Structure definition
//////////////////////////////////////////////////////////

template<int NB_INIT,
         int NB_TARGET,
         VCI_PARAM_DECLAR>

struct SOCLIB_VCI_LOCAL_CROSSBAR : sc_module{

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
  // VCI Target port to the micro-network
ADVANCED_VCI_TARGET<VCI_PARAM> TMN_VCI;
  // VCI Initiator port to the micro-network
ADVANCED_VCI_INITIATOR<VCI_PARAM> IMN_VCI;
  // VCI Target ports connected to the local initiators
ADVANCED_VCI_TARGET<VCI_PARAM> TLOC_VCI[NB_INIT];
  // VCI Initiator ports connected to the local targets
ADVANCED_VCI_INITIATOR<VCI_PARAM> ILOC_VCI[NB_TARGET];

  // REGISTERS
  
  // TMN FSM
  sc_signal<bool> TMN_ALLOCATED; //state of the target port to micro-network
  sc_signal<int>  TMN_INDEX;   

  
  // IMN FSM
  sc_signal<bool> IMN_ALLOCATED; // state of the initiator port to micro-network 
  sc_signal<int>  IMN_INDEX;   
 
   
  // TLOC FSMs
  sc_signal<bool> TLOC_ALLOCATED[NB_INIT]; // state of the local target ports 
  sc_signal<int>  TLOC_INDEX[NB_INIT];
 
    
  // ILOC FSMs
  sc_signal<bool> ILOC_ALLOCATED[NB_TARGET]; // state of the local initiator ports
  sc_signal<int>  ILOC_INDEX[NB_TARGET];
 
     ////////////////////////////////////////////////
    //	constructor
    ////////////////////////////////////////////////
 
    SC_HAS_PROCESS(SOCLIB_VCI_LOCAL_CROSSBAR);
    
    SOCLIB_VCI_LOCAL_CROSSBAR (	sc_module_name insname,
				int *indexes,
				SOCLIB_MAPPING_TABLE mapping_table)
	: TMN_ALLOCATED("TMN_ALLOCATED")
	 ,TMN_INDEX("TMN_INDEX")
	 ,IMN_ALLOCATED("IMN_ALLOCATED")
	 ,IMN_INDEX("IMN_INDEX")
{
    int addr_bits_to, id_bits_to, level;
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
      sensitive_pos << CLK;
      
      SC_METHOD(genMealy);
      sensitive_neg << CLK;

      sensitive << 	TMN_VCI.RSPACK;
      sensitive << 	TMN_VCI.CMDVAL;
      sensitive << 	TMN_VCI.ADDRESS;
      sensitive << 	TMN_VCI.WDATA;
      sensitive << 	TMN_VCI.CMD;
      sensitive << 	TMN_VCI.EOP;
      sensitive << 	TMN_VCI.PLEN;
      sensitive << 	TMN_VCI.CONS;
      sensitive << 	TMN_VCI.CONTIG;
      sensitive << 	TMN_VCI.CFIXED;
      sensitive << 	TMN_VCI.WRAP;
      sensitive << 	TMN_VCI.CLEN;
      sensitive << 	TMN_VCI.TRDID;
      sensitive << 	TMN_VCI.PKTID;
      sensitive << 	TMN_VCI.SRCID;
      
      sensitive << 	IMN_VCI.CMDACK;
      sensitive << 	IMN_VCI.RSPVAL;
      sensitive << 	IMN_VCI.RDATA;
      sensitive << 	IMN_VCI.REOP;
      sensitive << 	IMN_VCI.RERROR;
      sensitive << 	IMN_VCI.RTRDID;
      sensitive << 	IMN_VCI.RPKTID;
      sensitive << 	IMN_VCI.RSRCID;
      
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
                TMN_ALLOCATED       = false;
                TMN_INDEX           = 0;
                IMN_ALLOCATED       = false;
                IMN_INDEX           = 0;
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


            //	TMN FSM

            if (TMN_ALLOCATED == false) { // Target port to micro-network not allocated
                for(int n = 0 ; n < NB_TARGET ; n++) {
                    int m = (n + TMN_INDEX + 1) % NB_TARGET;
                    if(ILOC_VCI[m].RSPVAL == true) {
                        int id = (int)ILOC_VCI[m].RSRCID.read();
                        int globalIndex = GLOBAL_ID_OF(id);
                        if(globalIndex != GLOBAL_ID) {
                            TMN_ALLOCATED = true;
                            TMN_INDEX = m;
                            break;
                        }
                    }
                } // end for
            } else {			// Target port to micro-network allocated
                if((ILOC_VCI[TMN_INDEX].RSPVAL == true) && (TMN_VCI.RSPACK == true) && (ILOC_VCI[TMN_INDEX].REOP == true)) {
                    TMN_ALLOCATED = false;
                }
            }
    
            //	IMN FSM
    
            if (IMN_ALLOCATED == false) { // Initiator port to micro-network not allocated
                for(int n = 0 ; n < NB_INIT ; n++) {
                    int m = (n + IMN_INDEX + 1) % NB_INIT;
                    if(TLOC_VCI[m].CMDVAL == true) {
                        unsigned int address = TLOC_VCI[m].ADDRESS.read();
                        int index = GLOBAL_ROUTING_TABLE[GLOBAL_ADDR_OF(address)];
                        if(index != GLOBAL_ID) {
                            IMN_ALLOCATED = true;
                            IMN_INDEX = m;
                            break;
                        }
                    }
                } // end for
            } else {			// Initiator port to micro-network allocated
                if((TLOC_VCI[IMN_INDEX].CMDVAL == true) &&
                   (IMN_VCI.CMDACK == true) &&
                   (TLOC_VCI[IMN_INDEX].EOP == true)) {
                    IMN_ALLOCATED = false;
                }
            }
    
            //	Loop on the TLOC FSMs
    
            for(int t = 0 ; t < NB_INIT ; t++) { 
                if(TLOC_ALLOCATED[t] == false) { // local target port not allocated
                    int id = (int)IMN_VCI.RSRCID.read();
                    int local_target = LOCAL_ID_OF(id);
                    if ((IMN_VCI.RSPVAL==true)&&(local_target==t)){
                        TLOC_ALLOCATED[t] = true;
                        TLOC_INDEX[t] = LOCAL_PORT_ALLOCATED;
                    } else {
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
                    } 
                } else {			// local target port allocated
                    if(TLOC_INDEX[t] == LOCAL_PORT_ALLOCATED) { // it is allocated to the micro-network initiator port
                        if((IMN_VCI.RSPVAL == true) &&
                            (TLOC_VCI[t].RSPACK == true) &&
                           (IMN_VCI.REOP == true)) { TLOC_ALLOCATED[t] = false; 
                        }
                    } else {		// it is allocated to a local initiator port
                        if((ILOC_VCI[TLOC_INDEX[t]].RSPVAL == true) &&
                            (TLOC_VCI[t].RSPACK == true) &&
                           (ILOC_VCI[TLOC_INDEX[t]].REOP == true)) { TLOC_ALLOCATED[t] = false; }
                    }
                }
            } // end for t
    
            //	Loop on the ILOC FSMs
    
            for(int i = 0 ; i < NB_TARGET ; i++) {
                if(ILOC_ALLOCATED[i] == false) { // local initiator port not allocated 
                    unsigned int address = (int)TMN_VCI.ADDRESS.read();
                    int local_index = LOCAL_ROUTING_TABLE[LOCAL_ADDR_OF(address)];
                    if((TMN_VCI.CMDVAL == true) && (local_index == i)) { 
                        ILOC_ALLOCATED[i] = true; 
                        ILOC_INDEX[i] = LOCAL_PORT_ALLOCATED;

                    } else {
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
                    }
                } else {			// local initiator port allocated
                    if(ILOC_INDEX[i] == LOCAL_PORT_ALLOCATED) { // it is allocated to the micro-network target port
                        if((TMN_VCI.CMDVAL == true) &&
                           (ILOC_VCI[i].CMDACK == true) &&		  
                           (TMN_VCI.EOP == true)) { ILOC_ALLOCATED[i] = false;  }
                    } else {		// it is allocated to a local initiator port
                        if((TLOC_VCI[ILOC_INDEX[i]].CMDVAL == true) &&
                           (ILOC_VCI[i].CMDACK == true) &&(TLOC_VCI[ILOC_INDEX[i]].EOP == true)) 
                        { ILOC_ALLOCATED[i] = false;}
                    }
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
                    if (k == LOCAL_PORT_ALLOCATED) {
                        TLOC_VCI[i].RSPVAL	= IMN_VCI.RSPVAL.read(); 
                        TLOC_VCI[i].RERROR	= IMN_VCI.RERROR.read(); 
                        TLOC_VCI[i].REOP	= IMN_VCI.REOP.read(); 
                        TLOC_VCI[i].RTRDID	= IMN_VCI.RTRDID.read(); 
                        TLOC_VCI[i].RPKTID	= IMN_VCI.RPKTID.read(); 
                        TLOC_VCI[i].RSRCID	= IMN_VCI.RSRCID.read(); 
                        TLOC_VCI[i].RDATA	= IMN_VCI.RDATA.read();


                    } else { 
                        TLOC_VCI[i].RSPVAL	= ILOC_VCI[k].RSPVAL.read(); 
                        TLOC_VCI[i].RERROR	= ILOC_VCI[k].RERROR.read(); 
                        TLOC_VCI[i].REOP	= ILOC_VCI[k].REOP.read(); 
                        TLOC_VCI[i].RTRDID	= ILOC_VCI[k].RTRDID.read(); 
                        TLOC_VCI[i].RPKTID	= ILOC_VCI[k].RPKTID.read(); 
                        TLOC_VCI[i].RSRCID	= ILOC_VCI[k].RSRCID.read(); 
                        TLOC_VCI[i].RDATA	= ILOC_VCI[k].RDATA.read();

                    }
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
                    if (j == LOCAL_PORT_ALLOCATED) {
                        ILOC_VCI[i].CMDVAL	 =	TMN_VCI.CMDVAL.read();
                        ILOC_VCI[i].ADDRESS =	TMN_VCI.ADDRESS.read();
                        ILOC_VCI[i].WDATA	 =	TMN_VCI.WDATA.read();
                        ILOC_VCI[i].BE	 =	TMN_VCI.BE.read();
                        ILOC_VCI[i].CMD  	 =	TMN_VCI.CMD.read();
                        ILOC_VCI[i].PLEN	 =	TMN_VCI.PLEN.read();
                        ILOC_VCI[i].EOP	 =	TMN_VCI.EOP.read();
                        ILOC_VCI[i].CONS	 =	TMN_VCI.CONS.read();
                        ILOC_VCI[i].CONTIG	 =	TMN_VCI.CONTIG.read();
                        ILOC_VCI[i].CFIXED	 =	TMN_VCI.CFIXED.read();
                        ILOC_VCI[i].WRAP	 =	TMN_VCI.WRAP.read();
                        ILOC_VCI[i].CLEN	 =	TMN_VCI.CLEN.read();
                        ILOC_VCI[i].TRDID	 =	TMN_VCI.TRDID.read();
                        ILOC_VCI[i].PKTID	 =	TMN_VCI.PKTID.read();
                        ILOC_VCI[i].SRCID	 =	TMN_VCI.SRCID.read();
                    } else {
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
                    }
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
      
            // VCI micro-network target port
      
            if (TMN_ALLOCATED == true) { // allocated
                int k = TMN_INDEX;
                TMN_VCI.RSPVAL	= ILOC_VCI[k].RSPVAL.read(); 
                TMN_VCI.RERROR	= ILOC_VCI[k].RERROR.read(); 
                TMN_VCI.REOP	= ILOC_VCI[k].REOP.read(); 
                TMN_VCI.RTRDID	= ILOC_VCI[k].RTRDID.read(); 
                TMN_VCI.RPKTID	= ILOC_VCI[k].RPKTID.read(); 
                TMN_VCI.RSRCID	= ILOC_VCI[k].RSRCID.read(); 
                TMN_VCI.RDATA	= ILOC_VCI[k].RDATA.read();
            } else { // not allocated
                TMN_VCI.RSPVAL	= false;
                TMN_VCI.RERROR	= 0;
                TMN_VCI.REOP	= false;
                TMN_VCI.RTRDID	= 0;
                TMN_VCI.RPKTID	= 0;
                TMN_VCI.RSRCID	= 0;
                TMN_VCI.RDATA	= 0;
            }
      
            // VCI micro-network initiator port
      
            if (IMN_ALLOCATED == true) { // allocated
                int j = IMN_INDEX;
                IMN_VCI.CMDVAL	 =	TLOC_VCI[j].CMDVAL.read();
                IMN_VCI.ADDRESS	 =	TLOC_VCI[j].ADDRESS.read();
                IMN_VCI.WDATA	 =	TLOC_VCI[j].WDATA.read();
                IMN_VCI.BE	 =	TLOC_VCI[j].BE.read();
                IMN_VCI.CMD  	 =	TLOC_VCI[j].CMD.read();
                IMN_VCI.PLEN	 =	TLOC_VCI[j].PLEN.read();
                IMN_VCI.EOP	 =	TLOC_VCI[j].EOP.read();
                IMN_VCI.CONS	 =	TLOC_VCI[j].CONS.read();
                IMN_VCI.CONTIG	 =	TLOC_VCI[j].CONTIG.read();
                IMN_VCI.CFIXED	 =	TLOC_VCI[j].CFIXED.read();
                IMN_VCI.WRAP	 =	TLOC_VCI[j].WRAP.read();
                IMN_VCI.CLEN	 =	TLOC_VCI[j].CLEN.read();
                IMN_VCI.TRDID	 =	TLOC_VCI[j].TRDID.read();
                IMN_VCI.PKTID	 =	TLOC_VCI[j].PKTID.read();
                IMN_VCI.SRCID	 =	TLOC_VCI[j].SRCID.read();

            } else {  // not allocated
                IMN_VCI.CMDVAL   = 	false;
                IMN_VCI.ADDRESS	 = 	0;
                IMN_VCI.WDATA  	 = 	0;
                IMN_VCI.BE     	 = 	0;
                IMN_VCI.CMD    	 = 	0;
                IMN_VCI.PLEN   	 = 	0;
                IMN_VCI.EOP    	 = 	false;
                IMN_VCI.CONS   	 = 	false;
                IMN_VCI.CONTIG 	 = 	false;
                IMN_VCI.CFIXED 	 = 	false;
                IMN_VCI.WRAP   	 = 	false;
                IMN_VCI.CLEN   	 = 	0;
                IMN_VCI.TRDID  	 = 	0;
                IMN_VCI.PKTID  	 = 	0;
                IMN_VCI.SRCID  	 = 	0;
            }
      
// Each ack output is controlled by the FSM that is allocated on the VCI port at this time
	
            // for the IMN
            bool imn_rspack = false;
            for ( int i = 0; i<NB_INIT; i++){
                imn_rspack = imn_rspack |((TLOC_VCI[i].RSPACK)&&(TLOC_ALLOCATED[i])&&(TLOC_INDEX[i]==1024));
            }
            IMN_VCI.RSPACK=imn_rspack;
	
            //for the TMN
            bool tmn_cmdack = false;
            for ( int j = 0; j<NB_TARGET; j++){
                tmn_cmdack = tmn_cmdack | ((ILOC_VCI[j].CMDACK)&&(ILOC_ALLOCATED[j])&&(ILOC_INDEX[j]==1024));
            }
            TMN_VCI.CMDACK=tmn_cmdack;
	
            //for the ILOCs
            bool iloc_rspack;
            for(int j = 0; j<NB_TARGET; j++){
                iloc_rspack = ((TMN_VCI.RSPACK)&&(TMN_ALLOCATED)&&(TMN_INDEX==j));
                for ( int i = 0; i<NB_INIT; i++){
                    iloc_rspack = iloc_rspack | ((TLOC_VCI[i].RSPACK)&&(TLOC_ALLOCATED[i])&&(TLOC_INDEX[i]==j));
                }
                ILOC_VCI[j].RSPACK=iloc_rspack;
            }
 
            //for the TLOCs
            bool tloc_cmdack;
            for(int i = 0; i < NB_INIT; i++){
                tloc_cmdack = ((IMN_VCI.CMDACK)&&(IMN_ALLOCATED)&&(IMN_INDEX==i));
                for ( int j = 0; j<NB_TARGET; j++){
                    tloc_cmdack = tloc_cmdack | ((ILOC_VCI[j].CMDACK)&&(ILOC_ALLOCATED[j])&&(ILOC_INDEX[j]==i));
                }
                TLOC_VCI[i].CMDACK=tloc_cmdack;
            }
 
 
        };  // end genMealy()
    
    
  }; // end structure SOCLIB_VCI_LOCAL_CROSSBAR
    
#undef LOCAL_PORT_ALLOCATED
#undef GLOBAL_ADDR_OF
#undef LOCAL_ADDR_OF
#undef GLOBAL_ID_OF
#undef LOCAL_ID_OF

#endif
