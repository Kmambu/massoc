#ifndef MEMORY_H_INCLUDED
#define MEMORY_H_INCLUDED

#include <tlm>

#include "tlm_utils/simple_target_socket.h"

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
  const static uint8_t prg_text_size = 32;
  const static uint8_t prg_data_size = 32;
  const static uint8_t mem_size = prg_text_size + prg_data_size;

  uint32_t mem[mem_size];

  /*
   * constructor
   */
  Memory(sc_core::sc_module_name nm)
  : p_proc("p_proc")
  {
    /* TODO: Register callback for incoming b_transport interface method call */
    /* ... */

    /* Initialize memory */
    memset(mem, 0, mem_size * sizeof(uint32_t));

    /* with program */
    uint8_t i = 0;
    mem[i++] = 0x20010080; // addi $1, $0, 0x80
    mem[i++] = 0x8C220000; // lw $2, 0($1)
    mem[i++] = 0x8C230004; // lw $3, 4($1)
    mem[i++] = 0x00432020; // add $4, $2, $3
    mem[i++] = 0xac240008; // sw $4, 8($1)
    mem[i++] = 0x0000000D; // break

    /* and data */
    i = prg_text_size; // data segment begins where text segment ends
    mem[i++] = 0x00000001;
    mem[i++] = 0x00000002;
  }

  /* TLM-2 blocking transport method */
  virtual void b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay)
  {
    /* TODO: respond to the transaction */
    /* ... */
  }
};

#endif // MEMORY_H_INCLUDED

/*
 * Local Variables:
 * mode: C++
 * End:
 */
