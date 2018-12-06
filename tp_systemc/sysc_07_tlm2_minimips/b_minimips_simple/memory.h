#ifndef MEMORY_H_INCLUDED
#define MEMORY_H_INCLUDED

#include <tlm>
#include <tlm_utils/simple_target_socket.h>

/*
 * MEMORY component
 */
struct Memory: sc_core::sc_module
{
  /*
   * port definition
   */
  tlm_utils::simple_target_socket<Memory> p_proc; /* port from processor */

  /*
   * internal variables
   */
  // TODO: Add variables providing storage to the memory
  // ...

  /*
   * constructor
   */
  Memory(sc_core::sc_module_name nm /*TODO: Add parameters initialize memory (start address, size, content) */)
  : p_proc("p_proc") /* TODO: Initialize member variables */
  {
    /* Register callback for incoming b_transport interface method call */
    p_proc.register_b_transport(this, &Memory::b_transport);
  }

  /* TLM-2 blocking transport method */
  virtual void b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay)
  {
    tlm::tlm_command cmd = trans.get_command();
    sc_dt::uint64    adr = trans.get_address();
    unsigned char*   ptr = trans.get_data_ptr();
    unsigned int     len = trans.get_data_length();
    unsigned char*   byt = trans.get_byte_enable_ptr();
    unsigned int     wid = trans.get_streaming_width();
    if (adr >= 0 /* TODO: Test for valid address */ || byt != 0 || len > 4 || wid < len) {
      SC_REPORT_ERROR("TLM-2",
                      "Target does not support given generic payload transaction.");
      trans.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
      return;
    }
    switch (cmd) {
    case tlm::TLM_READ_COMMAND:
      /* TODO: Implement read from memory. */
    case tlm::TLM_WRITE_COMMAND:
      /* TODO: Implement write to memory. */
    default:
      SC_REPORT_ERROR("TLM-2",
                      "Target does not support given generic payload command.");
      trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
    }
  }
};

#endif // MEMORY_H_INCLUDED

/*
 * Local Variables:
 * mode: C++
 * End:
 */
