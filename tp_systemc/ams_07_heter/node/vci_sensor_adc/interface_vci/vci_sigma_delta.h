/* -*- c++ -*-
 *
 * SOCLIB_LGPL_HEADER_BEGIN
 * 
 * This file is part of SoCLib, GNU LGPLv2.1.
 * 
 * SoCLib is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; version 2.1 of the License.
 * 
 * SoCLib is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with SoCLib; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 * 
 * SOCLIB_LGPL_HEADER_END
 *
 * Copyright (c) UPMC, Lip6, Asim
 *         Antoine Leveque <antoine.leveque@lip6.fr>, 2009
 *
 */

#ifndef SOCLIB_CABA_VCI_SIGMA_DELTA_H
#define SOCLIB_CABA_VCI_SIGMA_DELTA_H

#include <systemc>
#include "vci_target_fsm.h"
#include "caba_base_module.h"
#include "mapping_table.h"

namespace soclib {
namespace caba {

using namespace sc_core;

template<typename vci_param,int nb_bits_decim>
class VciSigmaDelta
  : public soclib::caba::BaseModule
{
  
private:
  soclib::caba::VciTargetFsm<vci_param, true> m_vci_fsm;
  uint32_t m_ident; //identifiant essentiellement pour debug
  bool first;
  bool on_write(int seg, typename vci_param::addr_t addr, typename vci_param::data_t data, int be);
  bool on_read(int seg, typename vci_param::addr_t addr, typename vci_param::data_t &data);
  void transition();
  void genMoore();

  sc_signal<bool> r_irq;
  sc_signal<typename vci_param::data_t> r_sigma;
  sc_signal<typename vci_param::data_t> r_counter;
  
protected:
  SC_HAS_PROCESS(VciSigmaDelta);
public:
  sc_in<sc_dt::sc_uint<nb_bits_decim> > p_sigma;
  sc_in<bool> p_resetn;
  sc_in<bool> p_clk;
  soclib::caba::VciTarget<vci_param> p_vci;
  sc_out<bool> p_irq;
  
  VciSigmaDelta(
     sc_module_name name,
     const soclib::common::IntTab &index,
     const soclib::common::MappingTable &mt,
     uint32_t ident);
  ~VciSigmaDelta();
  
};
 
}}

#endif
