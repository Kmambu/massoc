#include <tlm>

#define DEBUG

#include "minimips.h"
#include "memory.h"

class Top : public sc_core::sc_module
{
public:
  Minimips *minimips;
  Memory   *memory;

  Top(sc_core::sc_module_name nm)
  {
    /* Instantiate components */
    minimips = new Minimips("minimips");
    memory   = new Memory("memory");

    /* TME: Bind the minimips to the memory */
    /* ... */
  }
};


#define MAX_CYCLES 100

int sc_main(int argc, char* argv[])
{
  Top top("top");

  sc_core::sc_time cycle = sc_core::sc_time(10, sc_core::SC_NS);
  sc_core::sc_start(cycle * MAX_CYCLES);

  SC_REPORT_INFO("sc_main", "Stop after simulating MAX_CYCLES");
  return 0;
}
