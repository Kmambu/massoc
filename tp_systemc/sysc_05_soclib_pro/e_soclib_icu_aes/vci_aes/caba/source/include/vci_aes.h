#ifndef SOCLIB_VCI_AES_H
#define SOCLIB_VCI_AES_H

#include <systemc>
#include "vci_target_fsm.h"
#include "caba_base_module.h"
#include "mapping_table.h"

namespace soclib {
namespace caba {

using namespace sc_core;

template<typename vci_param>
class VciAes : public caba::BaseModule
{
private:
  /* TME:
   * add vci target fsm declaration
   * and corresponding callbacks (on_write, on_read) prototypes
   */
  // ...
  // ...
  // ...

  /* configuration registers */
  sc_signal<int> r_size;
  sc_signal<int> r_mode;
  sc_signal<bool> r_irq;

  /* data segment */
  typedef typename vci_param::fast_data_t data_t;
  data_t r_data[1024];

  void transition();
  void genMoore();

protected:
  SC_HAS_PROCESS(VciAes);

public:
  sc_in<bool> p_clk;
  sc_in<bool> p_resetn;
  soclib::caba::VciTarget<vci_param> p_vci;
  sc_out<bool> p_irq;

  VciAes(sc_module_name name,
         const soclib::common::IntTab &index,
         const soclib::common::MappingTable &mt);

  ~VciAes();
};

}}

#endif
