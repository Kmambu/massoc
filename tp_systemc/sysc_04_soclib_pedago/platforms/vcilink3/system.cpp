// Step 1.1: C/C++ mandatory includes

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <signal.h>

// Step 1.2: Soclib includes

#include "shared/soclib_mapping_table.h"
#include "shared/soclib_vci_interfaces.h"
#include "soclib_vci_simpleinitiator.h"
#include "soclib_vci_simpletarget.h"
#include "soclib_vci_local_crossbar_simple.h"

// Step 1.3: Address space segmemtation include

// Step 2.1: Defining VCI parameters and address decoding bits

#define CELLSIZE        4       // Data are 4 cells(=8bits) wide=32 bits
#define ERRSIZE         1       // Error size is 1 bit
#define PLENSIZE        1       //
#define CLENSIZE        1       //
#define TRDIDSIZE       1       // TRDID unused
#define PKTIDSIZE       1       // PKTID unused too

// Step 2.2: Defining system parameters for decoding and memory mapping

#define ADDRSIZE     	32
#define SRCIDSIZE    	8

#define SEGTYPEMASK     0x00300000

// Step 3: Defining custom values for system parameterization

// Step 5: the sc_main function
////////////////////////////////////////////////////
//      MAIN
////////////////////////////////////////////////////

int sc_main (int argc, char *argv[])
{

  // Step 5.1: Creating the segment table
  ////////////////////////////////////////////////////
  //      SEGMENT_TABLE DEFINITION
  ////////////////////////////////////////////////////

  // TME exercise: fill-in mapping table properties.
  // Refer to soclib/include/shared/soclib_mapping_table.h file
  SOCLIB_MAPPING_TABLE  maptab ( /* ... */ );

  // TME exercise: fill-in segments.

  // Step 5.1: Declaring all the toplevel signals 
  ///////////////////////////////////////////////////////
  //      SIGNALS DECLARATION
  //////////////////////////////////////////////////////

  sc_clock signal_clk ("signal_clk");
  sc_signal < bool > signal_resetn ("signal_resetn");

  ADVANCED_VCI_SIGNALS <VCI_PARAM> link_i ("link_i");
  ADVANCED_VCI_SIGNALS <VCI_PARAM> link_t0 ("link_t0");
  ADVANCED_VCI_SIGNALS <VCI_PARAM> link_t1 ("link_t1");

  // Step 5.2: Instanciating the system components 
  /////////////////////////////////////////////////////////
  //      INSTANCIATED  COMPONENTS
  /////////////////////////////////////////////////////////

  SOCLIB_VCI_SIMPLEINITIATOR < VCI_PARAM > i0 ("i0");
  SOCLIB_VCI_SIMPLETARGET < VCI_PARAM > t0 ("t0");
  SOCLIB_VCI_SIMPLETARGET < VCI_PARAM > t1 ("t1");
  SOCLIB_VCI_LOCAL_CROSSBAR_SIMPLE<
    1,
    2,
    VCI_PARAM
    > local_crossbar ("local_crossbar", intList(), maptab);

  // Step 5.3: Initializing the system memories with appropriate code and data 
  //////////////////////////////////////////////////////////
  //      Segments Initialisation
  //////////////////////////////////////////////////////////

  // Step 5.4: Building the toplevel netlist 
  //////////////////////////////////////////////////////////
  //      Net-List
  //////////////////////////////////////////////////////////

  i0.CLK(signal_clk);
  i0.RESETN(signal_resetn);
  i0.VCI_INITIATOR(link_i);

  t0.CLK(signal_clk);
  t0.RESETN(signal_resetn);
  t0.VCI_TARGET(link_t0);

  t1.CLK(signal_clk);
  t1.RESETN(signal_resetn);
  t1.VCI_TARGET(link_t1);

  // TME exercise: fill-in the netlist.

  sc_start(sc_core::sc_time(0, SC_NS));
  signal_resetn = false;
  sc_start(sc_core::sc_time(1, SC_NS));
  signal_resetn = true;

  sc_start ();

  return EXIT_SUCCESS;
}
