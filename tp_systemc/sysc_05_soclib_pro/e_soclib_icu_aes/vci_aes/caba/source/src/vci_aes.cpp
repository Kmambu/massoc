#include "aes.h"
#include "../include/vci_aes.h"
#include "../include/internal_aes.h"

namespace soclib {
namespace caba {

using namespace soclib;

#define tmpl(t) template<typename vci_param> t VciAes<vci_param>

tmpl(bool)::on_write(int seg, typename vci_param::addr_t addr, typename vci_param::data_t data, int be)
{
  unsigned int cell = (int)addr / vci_param::B;

  if (seg == 0) /* configuration segment */
    {
      /* TME: setup the proper configuration register according to the value
       * of cell:
       *  r_irq, r_mode and r_size
       */
      // ...
    }
  else if (seg == 1) /* data segment */
    {
      /* error checking */
      if (cell >= 1024)
        return false;

      unsigned int cur = r_data[cell];
      typename vci_param::data_t mask = vci_param::be2mask(be);
      r_data[cell] = (cur & ~mask) | (data & mask);
    }
  else {
    printf("segment unknown");
    assert(0);
  }

  return true;
}

tmpl(bool)::on_read(int seg, typename vci_param::addr_t addr, typename vci_param::data_t &data)
{
  unsigned int cell = (int)addr / vci_param::B;

  if (seg == 1) /* data segment */
    {
      /* error checking */
      if (cell >= 1024)
        return false;

      data = r_data[cell];
    }

  return true;
}

tmpl(void)::transition()
{
  if (!p_resetn.read()) {
    // TME: reset vci target fsm
    // ...

    /*
     * TME: initialize the configuration registers:
     * - r_size, r_mode and r_irq
     */
    // ...

    return;
  }

  // TME: call the transition function of vci target fsm
  // ...

  /* TME: launch AES if requested
   * (for the actual decryption, call the internal function named:
   * decrypt_data(unsigned char *data, unsigned int len))
   */
  if (r_mode.read() & SOCLIB_AES_RUNNING)
    {
      // ...
    }
}

tmpl(void)::genMoore()
{
  // TME: call the genMoore function of vci target fsm
  // ...

  /* TME: setup p_irq */
  // ...
}

tmpl(/**/)::VciAes(sc_module_name name,
                   const IntTab &index,
                   const MappingTable &mt)
           : caba::BaseModule(name),
           // TME: bind the vci target fsm to the p_vci port and the associated list
           // of segments
           // ...
           p_clk("clk"),
           p_resetn("resetn"),
           p_vci("vci"),
           p_irq("irq")
{
  // TME: bind the vci target fsm to the proper callbacks
  // ...

  SC_METHOD(transition);
  dont_initialize();
  sensitive << p_clk.pos();

  SC_METHOD(genMoore);
  dont_initialize();
  sensitive << p_clk.neg();
}

tmpl(/**/)::~VciAes() { }

}}
