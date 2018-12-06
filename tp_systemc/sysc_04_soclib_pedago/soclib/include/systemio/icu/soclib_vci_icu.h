/*******************************************************************************
 * File : soclib_icu.h
 * Date : 12/10/2005
 * Author : Franck Wajsbürt
 * Copyright UPMC-LIP6
 * It is released under the GNU Public License.
 *
 * This component is a basic interrupt controller. 
 * It multiplexes from 1 to 32 entries of interruptions to 1 exit. 
 * Each interruption input can be individualy masked. 
 * 
 * Interrupt inputs are latched in a read only register at address 0.
 * To acknowledge an interrupt, the user must access directly 
 * the vci target from where the interrupt comes.
 * A request write to this register has no effect.
 * Interrupt input [i] is placed into the bit [i].
 * This register synchronizes the interruptions on VCI clock. 
 * Consequently, it adds a time between interruption acknowledgement 
 * and the interrupt output fall.
 * 
 * The mask of interrupts is a register at address 4.
 * A 0 in bit [i] of the register of mask blocks interruption [i]
 * Two write only registers allows the user to change the mask value.
 * at address 8 : mask_set 
 * at address C : mask_clear
 * the value writes into mask_set defines which bits in mask register must be 
 * set, ex write 0b0010010 sets bit 1 and 4 in mask regsiter.
 * the value writes into mask_clear defines which bits in mask register must be 
 * cleared, ex write 0b0010010 clears bit 1 and 4 in mask regsiter.
 * 
 * The interrupt vector table begins at address 0x20 and goes up to 0x20+(4*NIRQ)
 * Each entry points to the address of the Interrupt structure (in case of Mutek
 * use)
 *
 * To get the highest priority interrupt, you must read in the register at @ 0x10

 * This component can be used in a two level hierarchical interconnected system
 * That is why it has got two VCI indexes : a global index for the upper level
 * and a local for the lower
 *
 * This component has 4 "generator" parameters :
 * - name            : instance name
 * - globalIndex     : VCI index of the subsystem which contains the timer 
 * - localindex      : VCI index of the target in the subsystem
 * - maptab          : pointer on the system mapping table.
 *
 * This component has 9 template parameter :
 * - int NIRQ        : number of independant IRQ inputs
 * - int ADDRSIZE    : number of bits of the VCI ADDRESS field
 * - int CELLSIZE    : number of bytes of the VCI DATA field
 * - int ERRSIZE     : number of bits of the VCI ERROR field
 * - int PLENSIZE    : number of bits of the VCI PLEN field
 * - int CLENSIZE    : number of bits of the VCI CLEN field
 * - int SRCIDSIZE   : number of bits of the VCI SRCID field
 * - int TRDIDSIZE   : number of bits of the VCI TRDID field
 * - int PKTIDSIZE   : number of bits of the VCI PKTID field
 *
 *              IRQ
 *               |
 *          +----+----+
 *          |         |----- IRQI[0]
 *          |         |----- IRQI[1]
 *      /---\         |
 *     | VCI |        |
 *      \---/         |
 *          |         |
 *          |         |----- IRQI[NIRQ-1]
 *          +---------+
 *
 *
 * IRQ = OR[i = 0 to (NIRQ-1)] ( AND (IRQI[i], MASK[i]))
 *
 * where IRQI[i] is the latched (on clk) value of IRQI[i] input (at address 0)
 *       MASK[i] is a r/w internal register                     (at address 4)
 *
 *******************************************************************************/

#ifndef SOCLIB_VCI_ICU_H
#define SOCLIB_VCI_ICU_H

#include <systemc.h>
#include "shared/soclib_vci_interfaces.h"
#include "shared/soclib_mapping_table.h"
#include "soclib_vci_icu_map.h"

#ifndef _SOCLIB_VCI_ICU_VHDL

////////////////////////////////////////////////////////////
//      Structure definition
////////////////////////////////////////////////////////////

template <
   unsigned int NIRQ,
   VCI_PARAM_DECLAR > struct SOCLIB_VCI_ICU : sc_module
{

// I/O Ports

   sc_in < bool > CLK;
   sc_in < bool > RESETN;
   ADVANCED_VCI_TARGET < VCI_PARAM > VCI;
   sc_in < bool > IRQI[NIRQ];
   sc_out < bool > IRQ;

// Structural parameters

   const char *NAME;
   unsigned int BASE;
   unsigned int SIZE;
   const char *segNAME;


// Registers

   sc_signal < sc_uint<NIRQ> >         ICU_INT;
   sc_signal < sc_uint<NIRQ> >         ICU_MASK;
/*    sc_signal < sc_uint<32> >           ICU_VECTOR[NIRQ]; */
   sc_signal < sc_uint<3>         >    TARGET_FSM;
   sc_signal < sc_uint<SRCIDSIZE> >    BUF_SRCID;        // save SRCID
   sc_signal < sc_uint<TRDIDSIZE> >    BUF_TRDID;        // save TRDID
   sc_signal < sc_uint<PKTIDSIZE> >    BUF_PKTID;        // save PKTID
   sc_signal < sc_uint<8> >            BUF_ADR;          // save selected register address
   sc_signal < sc_uint<8 * CELLSIZE> > BUF_WDATA;        // save data

// FSM states
   
   enum
   {
      TARGET_IDLE,
      TARGET_ERROR_RSP,
      TARGET_ERROR_EOP,
      TARGET_WRITE_RSP,
      TARGET_WRITE_EOP,
      TARGET_READ_RSP,
      TARGET_READ_EOP
   };

// register mapping
   
   enum
   {
      IT_VECTOR_ADDRESS = 0x0,
      MASK_ADDRESS = 0x4,
      MASK_SET_ADDRESS = 0x8,
      MASK_CLEAR_ADDRESS = 0xC,
      INT_ADDRESS = 0x10
/*    IT_VECTORS_ADDRESS = 0x20 */
   };

////////////////////////////////////////////////////////////
//      Constructor
////////////////////////////////////////////////////////////

   SC_HAS_PROCESS (SOCLIB_VCI_ICU);

   SOCLIB_VCI_ICU (sc_module_name name, // instance name
                         unsigned int index, // VCI target index
                         SOCLIB_MAPPING_TABLE maptab) // mapping table pointer
   {
#ifdef SOCVIEW
      ICU_INT.rename ("ICU_INT");
      ICU_MASK.rename ("ICU_MASK");
      BUF_SRCID.rename ("BUF_SRCID");  
      BUF_TRDID.rename ("BUF_TRDID"); 
      BUF_PKTID.rename ("BUF_PKTID"); 
      BUF_NUM.rename ("BUF_NUM");     
      BUF_ADR.rename ("BUF_ADR");     
      BUF_WDATA.rename ("BUF_WDATA"); 
#endif

      SC_METHOD (transition);
      sensitive_pos << CLK;

      SC_METHOD (genMoore);
      sensitive_neg << CLK;

      NAME = (const char *) name;

      // checking the vci parameters

      if (ADDRSIZE != 32)
      {
         printf (" Error in SOCLIB_VCI_ICU component : %s\n", NAME);
         printf (" the ADDRSIZE parameter\n");
         printf (" must be equal to 32\n");
         sc_stop ();
      }
      if (CELLSIZE != 4)
      {
         printf (" Error in SOCLIB_VCI_ICU component : %s\n", NAME);
         printf (" the CELLSIZE parameter\n");
         printf (" must be equal to 4\n");
         sc_stop ();
      }
      if (ERRSIZE != 1)
      {
         printf (" Error in SOCLIB_VCI_ICU component : %s\n", NAME);
         printf (" the ERRSIZE parameter\n");
         printf (" must be equal to 1\n");
         sc_stop ();
      }
      if (SRCIDSIZE > 16)
      {
         printf (" Error in SOCLIB_VCI_ICU component : %s\n", NAME);
         printf (" the SRCIDSIZE parameter\n");
         printf (" cannot be larger than 16\n");
         sc_stop ();
      }
      if (TRDIDSIZE > 32)
      {
         printf (" Error in SOCLIB_VCI_ICU component : %s\n", NAME);
         printf (" the TRDIDSIZE parameter\n");
         printf (" cannot be larger than 32\n");
         sc_stop ();
      }
      if (PKTIDSIZE > 32)
      {
         printf (" Error in SOCLIB_VCI_ICU component : %s\n", NAME);
         printf (" the PKTIDSIZE parameter\n");
         printf (" cannot be larger than 32\n");
         sc_stop ();
      }

      // segment definition

      std::list < SEGMENT_TABLE_ENTRY > seglist = maptab.getSegmentList (index);
      BASE = (*seglist.begin ()).getBase ();
      SIZE = (*seglist.begin ()).getSize ();
      segNAME = (*seglist.begin ()).getName ();

      // checking parameters

      if ((NIRQ < 1) || (NIRQ > 32))
      {
         printf ("Error in component SOCLIB_VCI_ICU %s\n", NAME);
         printf ("The NIRQ parameter (number of interrupt inputs) \n");
         printf ("must be from 1 to 32\n");
         sc_stop ();
      }
      if (SIZE < 8)
      {
         printf ("Error in component SOCLIB_VCI_ICU %s\n", NAME);
         printf ("The segment SIZE allocated to this component\n");
         printf ("must be equal or larger than 8 bytes\n");
         sc_stop ();
      }
      if ((BASE & 0x00000003) != 0x0)
      {
         printf ("Error in component SOCLIB_VCI_ICU %s\n", NAME);
         printf ("The BASE address must be multiple of 4\n");
         sc_stop ();
      }

      printf ("Successful Instanciation of SOCLIB_VCI_ICU : %s\n", NAME);
      printf ("segmentName : %s \n", segNAME);

   }; // end constructor

////////////////////////////////////////////////////////////
//      transition 
////////////////////////////////////////////////////////////

   void transition ()
   {
      if (RESETN == false)
      {
         TARGET_FSM = TARGET_IDLE;
         ICU_INT = 0;
         ICU_MASK = 0; // default all interrupt are masked
         return;
      }

      unsigned int address; // VCI address

      switch (TARGET_FSM.read())
      {
      case TARGET_IDLE:

         if (VCI.CMDVAL.read() == true)
         {
            address = (unsigned int) VCI.ADDRESS.read() & 0xFFFFFFFC;
            BUF_SRCID = VCI.SRCID.read();
            BUF_TRDID = VCI.TRDID.read();
            BUF_PKTID = VCI.PKTID.read();
            BUF_WDATA = VCI.WDATA.read();
            BUF_ADR = (address - BASE) & 0x000000FF;
            if ( ! (((address == (BASE+INT_ADDRESS))        && VCI.CMD.read () == VCI_CMD_READ) ||
                    ((address == (BASE+MASK_ADDRESS))       && VCI.CMD.read () == VCI_CMD_READ) ||
                    ((address == (BASE+MASK_ADDRESS))       && VCI.CMD.read () == VCI_CMD_WRITE)||
                    ((address == (BASE+MASK_SET_ADDRESS))   && VCI.CMD.read () == VCI_CMD_WRITE)||
                    ((address == (BASE+MASK_CLEAR_ADDRESS)) && VCI.CMD.read () == VCI_CMD_WRITE)||
                    ((address == (BASE+IT_VECTOR_ADDRESS))  && VCI.CMD.read () == VCI_CMD_READ)// ||
/*                  (((((address - BASE) & 0x000000FF)-IT_VECTORS_ADDRESS)>>2) < NIRQ ) */
                    )
                 )
               TARGET_FSM = (VCI.EOP.read() == true) ? TARGET_ERROR_EOP : TARGET_ERROR_RSP;
            else if (VCI.CMD.read () == VCI_CMD_WRITE)
               TARGET_FSM = (VCI.EOP.read() == true) ? TARGET_WRITE_EOP : TARGET_WRITE_RSP;
            else
               TARGET_FSM = (VCI.EOP.read() == true) ? TARGET_READ_EOP : TARGET_READ_RSP;
         }
         break;

      case TARGET_WRITE_RSP:
      case TARGET_WRITE_EOP:
        //int tmp = BUF_ADR.read();
        //int tmp2 = BUF_WDATA.read();
        switch (BUF_ADR.read()) 
        {
        case MASK_ADDRESS:
          ICU_MASK = BUF_WDATA.read();
          break;
          
        case MASK_SET_ADDRESS:
          ICU_MASK = ICU_MASK.read() | BUF_WDATA.read();
          break;
          
        case MASK_CLEAR_ADDRESS:
          ICU_MASK = ICU_MASK.read() & ~(BUF_WDATA.read());
          break;
          
        default:
/*        ICU_VECTOR[(BUF_ADR.read()-IT_VECTORS_ADDRESS)>>2] = BUF_WDATA.read(); */
          break;
        }
        
         if (VCI.RSPACK.read() == true)
            TARGET_FSM = TARGET_IDLE;
         break;

      case TARGET_READ_RSP:
      case TARGET_READ_EOP:
         if (VCI.RSPACK.read() == true)
            TARGET_FSM = TARGET_IDLE;
         break;

      case TARGET_ERROR_RSP:
      case TARGET_ERROR_EOP:
         if (VCI.RSPACK.read() == true)
            TARGET_FSM = TARGET_IDLE;
         break;
         
      } // end switch TARGET FSM

      // Latch interrupt inputs

      unsigned int tmp = 0;
      for (int i = (NIRQ-1); i>=0; --i)
      {
         tmp = (tmp<<1) | (unsigned int)(IRQI[i].read());
      }
      ICU_INT = tmp;
      tmp = ICU_MASK.read();

   }; // end transition()

////////////////////////////////////////////////////////////
//      genMoore
////////////////////////////////////////////////////////////

   void genMoore ()
   {

      // VCI signals

      switch ((unsigned int)TARGET_FSM.read())
      {
      case TARGET_IDLE:
        
         VCI.CMDACK = true;
         VCI.RSPVAL = false;
         VCI.RSRCID = 0;
         VCI.RTRDID = 0;
         VCI.RPKTID = 0;
         VCI.RERROR = 0;
         VCI.RDATA  = 0;
         VCI.REOP   = false;
         break;

      case TARGET_WRITE_RSP:

         VCI.CMDACK = false;
         VCI.RSPVAL = true;
         VCI.RSRCID = BUF_SRCID;
         VCI.RTRDID = BUF_TRDID;
         VCI.RPKTID = BUF_PKTID;
         VCI.RERROR = 0;
         VCI.RDATA  = 0;
         VCI.REOP   = false;
         break;
         
      case TARGET_WRITE_EOP:

         VCI.CMDACK = false;
         VCI.RSPVAL = true;
         VCI.RSRCID = BUF_SRCID;
         VCI.RTRDID = BUF_TRDID;
         VCI.RPKTID = BUF_PKTID;
         VCI.RERROR = 0;
         VCI.RDATA  = 0;
         VCI.REOP   = true;
         break;

      case TARGET_READ_RSP:

         VCI.CMDACK = false;
         VCI.RSPVAL = true;
         VCI.RSRCID = BUF_SRCID;
         VCI.RTRDID = BUF_TRDID;
         VCI.RPKTID = BUF_PKTID;
         VCI.RERROR = 0;
         switch (BUF_ADR.read()) 
         {
         case INT_ADDRESS:
           VCI.RDATA  = (sc_uint<32>)(ICU_INT.read());
           break;
           
         case MASK_ADDRESS:
           VCI.RDATA  = (sc_uint<32>)(ICU_MASK.read());
           break;

         case IT_VECTOR_ADDRESS: {
           // give the highest priority interrupt
           unsigned int nbr_int=NIRQ;;
           unsigned int req = ICU_INT.read() & ICU_MASK.read();
           for (unsigned int f=0; f<=NIRQ; ++f)
             if (req&(1<<f)) {
               nbr_int = f;
               break;
             }
           VCI.RDATA  = (sc_uint<32>)nbr_int;
           break;
         }

         default:
/*         VCI.RDATA = (sc_uint<32>)(ICU_VECTOR[(BUF_ADR.read()-IT_VECTORS_ADDRESS)>>2].read()); */
           break;
         }
         VCI.REOP   = false;
         break;

      case TARGET_READ_EOP:

         VCI.CMDACK = false;
         VCI.RSPVAL = true;
         VCI.RSRCID = BUF_SRCID;
         VCI.RTRDID = BUF_TRDID;
         VCI.RPKTID = BUF_PKTID;
         VCI.RERROR = 0;
         switch (BUF_ADR.read()) 
         {
         case INT_ADDRESS:
           VCI.RDATA  = (sc_uint<32>)(ICU_INT.read());
           break;
           
         case MASK_ADDRESS:
           VCI.RDATA  = (sc_uint<32>)(ICU_MASK.read());
           break;

         case IT_VECTOR_ADDRESS: {
           // give the highest priority interrupt
           unsigned int nbr_int;
           unsigned int req = ICU_INT.read() & ICU_MASK.read();
           for (unsigned int f=0; f<=NIRQ; ++f)
             if (req&(1<<f))
               nbr_int = f;
           VCI.RDATA  = (sc_uint<32>)nbr_int;
           break;
         }

         default:
/*         VCI.RDATA = (sc_uint<32>)(ICU_VECTOR[(BUF_ADR.read()-IT_VECTORS_ADDRESS)>>2].read()); */
           break;
         }
/*          VCI.RDATA  = (sc_uint<32>)(((unsigned int)BUF_ADR.read() == INT_ADDRESS) ? ICU_INT.read() : ICU_MASK.read()) ; */
         VCI.REOP   = true;
         break;

      case TARGET_ERROR_RSP:

         VCI.CMDACK = false;
         VCI.RSPVAL = true;
         VCI.RSRCID = BUF_SRCID;
         VCI.RTRDID = BUF_TRDID;
         VCI.RPKTID = BUF_PKTID;
         VCI.RERROR = 1;
         VCI.RDATA  = 0;
         VCI.REOP   = false;
         break;

      case TARGET_ERROR_EOP:

         VCI.CMDACK = false;
         VCI.RSPVAL = true;
         VCI.RSRCID = BUF_SRCID;
         VCI.RTRDID = BUF_TRDID;
         VCI.RPKTID = BUF_PKTID;
         VCI.RERROR = 1;
         VCI.RDATA  = 0;
         VCI.REOP   = true;
         break;

      }  // end switch TARGET FSM

      // Interrupt output

      int icu_int = (int)(ICU_INT.read());
      int icu_mask = (int)(ICU_MASK.read());
/*       if (icu_int != 0) */
/*      printf("interrupt %x at time %s\n",(unsigned int)ICU_INT.read(),sc_time_stamp().to_string().c_str()); */
      IRQ = (bool)((icu_int & icu_mask) != 0);
      /*
      if (IRQ)
        printf("send the interrupt signal at time %s\n",sc_time_stamp().to_string().c_str());
      */

   }; // end genMoore()

}; // end structure SOCLIB_VCI_ICU

#else // ifndef _SOCLIB_VCI_ICU_VHDL

//*****************************************************************
//        VHDL
//*****************************************************************        

template < 
   int NIRQ,
   int ADDRSIZE,
   int CELLSIZE,
   int ERRSIZE,
   int PLENSIZE,
   int CLENSIZE, 
   int SRCIDSIZE, 
   int TRDIDSIZE, 
   int PKTIDSIZE > class SOCLIB_VCI_ICU_VHDL:public sc_foreign_module
{
  
   public:
   
   sc_in  < bool >                     CLK;
   sc_in  < bool >                     RESETN;
   sc_in  < bool >                     VCI_CMDVAL;
   sc_in  < bool >                     VCI_RSPACK;
   sc_in  < sc_uint < ADDRSIZE > >     VCI_ADDRESS;
   sc_in  < sc_uint < CELLSIZE > >     VCI_BE;
   sc_in  < sc_uint < 2 > >            VCI_CMD;
   sc_in  < sc_uint < 8 * CELLSIZE > > VCI_WDATA;
   sc_in  < bool >                     VCI_EOP;
   sc_in  < sc_uint < SRCIDSIZE > >    VCI_SRCID;
   sc_out < bool >                     VCI_CMDACK;
   sc_out < bool >                     VCI_RSPVAL;
   sc_out < sc_uint < 8 * CELLSIZE > > VCI_RDATA;
   sc_out < bool >                     VCI_REOP;
   sc_out < sc_uint < ERRSIZE > >      VCI_RERROR;
   sc_out < sc_uint < SRCIDSIZE > >    VCI_RSRCID;
   sc_in  < sc_uint < NIRQ > >         IRQI;
   sc_out < bool >                     IRQ;

   SOCLIB_VCI_ICU_VHDL (sc_module_name nm, const char *hdl_name) : 
      sc_foreign_module (nm, hdl_name),
      CLK ("CLK"),
      RESETN ("RESETN"),
      VCI_CMDVAL ("VCI_CMDVAL"),
      VCI_RSPACK ("VCI_RSPACK"),
      VCI_ADDRESS ("VCI_ADDRESS"),
      VCI_BE ("VCI_BE"),
      VCI_CMD ("VCI_CMD"),
      VCI_WDATA ("VCI_WDATA"),
      VCI_EOP ("VCI_EOP"),
      VCI_SRCID ("VCI_SRCID"),
      VCI_CMDACK ("VCI_CMDACK"),
      VCI_RSPVAL ("VCI_RSPVAL"),
      VCI_RDATA ("VCI_RDATA"), 
      VCI_REOP ("VCI_REOP"), 
      VCI_RERROR ("VCI_RERROR"), 
      VCI_RSRCID ("VCI_RSRCID"), 
      IRQI ("IRQI")
      IRQ ("IRQ")
   {
   }
};

template <
   int NIRQ,
   int ADDRSIZE,
   int CELLSIZE,
   int ERRSIZE,
   int PLENSIZE, 
   int CLENSIZE, 
   int SRCIDSIZE,
   int TRDIDSIZE,
   int PKTIDSIZE > class SOCLIB_VCI_ICU : public sc_module
{
   public:

   // IO PORTS

   sc_in  < bool > CLK;
   sc_in  < bool > RESETN;
   ADVANCED_VCI_TARGET < ADDRSIZE, CELLSIZE, ERRSIZE, PLENSIZE, CLENSIZE, SRCIDSIZE, TRDIDSIZE, PKTIDSIZE > VCI;
   sc_in  < sc_uint<NIRQ> > IRQI;
   sc_out < bool > IRQ;

   // Structural parameters

   SOCLIB_VCI_ICU_VHDL <NIRQ, ADDRSIZE, CELLSIZE, ERRSIZE, PLENSIZE, CLENSIZE, SRCIDSIZE, TRDIDSIZE, PKTIDSIZE > *icu_vhd;

////////////////////////////////////////////////////////////
//      Constructor
////////////////////////////////////////////////////////////

   SC_HAS_PROCESS (SOCLIB_VCI_ICU);
   SOCLIB_VCI_ICU ( sc_module_name insname,      // instance name 
                    unsigned int globalIndex,    // VCI target index
                    unsigned int localIndex, 
                    SOCLIB_MAPPING_TABLE maptab) // segment table pointer
   : CLK ("CLK"), RESETN ("RESETN"), signal_irqi ("signal_irqi"), signal_irq ("signal_irq")
   {
      icu_vhd =
         new SOCLIB_VCI_ICU_VHDL < 
            NIRQ, 
            ADDRSIZE, 
            CELLSIZE, 
            ERRSIZE, 
            PLENSIZE, 
            CLENSIZE, 
            SRCIDSIZE, 
            TRDIDSIZE, 
            PKTIDSIZE > 
            ("icu_vhd", "soclib_vci_icu");

      icu_vhd -> CLK (CLK);
      icu_vhd -> RESETN (RESETN);
      icu_vhd -> VCI_CMDACK (VCI.CMDACK);
      icu_vhd -> VCI_RSPVAL (VCI.RSPVAL);
      icu_vhd -> VCI_RDATA (VCI.RDATA);
      icu_vhd -> VCI_REOP (VCI.REOP);
      icu_vhd -> VCI_RERROR (VCI.RERROR);
      icu_vhd -> VCI_RSRCID (VCI.RSRCID);
      icu_vhd -> VCI_CMDVAL (VCI.CMDVAL);
      icu_vhd -> VCI_RSPACK (VCI.RSPACK);
      icu_vhd -> VCI_ADDRESS (VCI.ADDRESS);
      icu_vhd -> VCI_BE (VCI.BE);
      icu_vhd -> VCI_CMD (VCI.CMD);
      icu_vhd -> VCI_WDATA (VCI.WDATA);
      icu_vhd -> VCI_EOP (VCI.EOP);
      icu_vhd -> VCI_SRCID (VCI.SRCID);
      icu_vhd -> IRQI (IRQI);
      icu_vhd -> IRQ (IRQ);
   };
};

#endif // ifndef _SOCLIB_VCI_ICU_VHDL
#endif // ifndef SOCLIB_VCI_ICU_H
