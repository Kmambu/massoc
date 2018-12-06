#include <tlm>

#define DEBUG

#include "segmentation.h"
#include "minimips.h"
#include "router.h"
#include "memory.h"

class Top : public sc_core::sc_module
{
public:
  Minimips *minimips;
  Router   *router;
  Memory   *memory_text;
  Memory   *memory_data;

  Top(sc_core::sc_module_name nm)
  {
    /* Instantiate components */
    minimips = new Minimips("minimips");
    router = new Router("router");

    /* Memory with program */
    /* TODO: Initialize text segment memory and pass it to memory module */
    memory_text = new Memory("memory_text" /* TODO: initialize text segment */);

    /* Memory with data (begins where text segment ends) */
    /* TODO: Initialize data segment memory and pass it to memory module */
    memory_data = new Memory("memory_data" /* TODO: initialize data segment */);

    /* TODO: Bind the minimips to the memories via the router */
  }

  virtual ~Top() {
    delete minimips;
    delete router;
    delete memory_text;
    delete memory_data;
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
