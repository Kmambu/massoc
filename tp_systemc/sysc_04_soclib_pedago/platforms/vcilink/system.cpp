// Step 1.1: C/C++ mandatory includes

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <signal.h>

// Step 1.2: Soclib includes

#include "shared/soclib_vci_interfaces.h"
#include "soclib_vci_simpleinitiator.h"
#include "soclib_vci_simpletarget.h"

// Step 1.3: Address space segmentation include
// None here in this exercise.

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

#define SEGTYPEMASK     0x00300000 // Not used here.

// Step 3: Defining custom values for system parameterization

// Step 4: The clock generation function
// None.

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

  // Step 5.1: Declaring all the toplevel signals
  ///////////////////////////////////////////////////////
  //      SIGNALS DECLARATION
  //////////////////////////////////////////////////////

  sc_clock signal_clk ("signal_clk");
  sc_signal < bool > signal_resetn ("signal_resetn");

  ADVANCED_VCI_SIGNALS <VCI_PARAM> link ("link");

  // Step 5.2: Instanciating the system components
  /////////////////////////////////////////////////////////
  //      INSTANCIATED  COMPONENTS
  /////////////////////////////////////////////////////////

  SOCLIB_VCI_SIMPLEINITIATOR < VCI_PARAM > i0 ("i0");
  SOCLIB_VCI_SIMPLETARGET < VCI_PARAM > t0 ("t0");

  // Step 5.3: Initializing the system memories with appropriate code and data
  //////////////////////////////////////////////////////////
  //      Segments Initialisation
  //////////////////////////////////////////////////////////

  // Step 5.4: Building the toplevel netlist
  //////////////////////////////////////////////////////////
  //      Net-List
  //////////////////////////////////////////////////////////

  // TME exercise: fill-in the netlist here !

  sc_start(sc_core::sc_time(0, SC_NS));
  signal_resetn = false;
  sc_start(sc_core::sc_time(1, SC_NS));
  signal_resetn = true;

  sc_start ();
  return EXIT_SUCCESS;
}
