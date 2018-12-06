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

#include "vci_sigma_delta.h" 
#include "sigma_delta.h"

namespace soclib {
namespace caba {

#define tmpl(x) template<typename vci_param,int nb_bits_decim> x VciSigmaDelta<vci_param,nb_bits_decim>

  tmpl(bool)::on_write(int seg, typename vci_param::addr_t addr, typename vci_param::data_t data, int be)
  {
    return false;
  }
  
  tmpl(bool)::on_read(int seg, typename vci_param::addr_t addr, typename vci_param::data_t &data)
  {
    int cell = (int)addr / vci_param::B;
    int reg = cell % SD_SPAN;
    
    switch (reg) {
    case SD_DATA:
      data = r_sigma.read();
      r_irq = false;
      return true;
    case SD_COUNTER:
      data = r_counter.read();
      r_irq = false;
      return true;
    default:
      return false;
    }
  }
  
  tmpl(void)::transition()
  {
    if (!p_resetn) {
      m_vci_fsm.reset();
      r_counter=0;
      first=true;
      return;
    }
    
    m_vci_fsm.transition();
    
    r_sigma = (typename vci_param::data_t)p_sigma.read() ;

//     if (first && ((r_sigma.read() > 130 ) || (r_sigma.read() < 125 )) && (r_sigma.read() != 0 )){
    if (first && (r_sigma.read() > 130) ) {
      std::cout << "Sensor " << m_ident << " " 
		<< sc_time_stamp().to_seconds() << " " 
		<< r_sigma.read() << std::endl;
      r_irq=true;
      first=false;
    }

    r_counter=r_counter.read()+1;
  }

  tmpl(void)::genMoore()
  {
    m_vci_fsm.genMoore();
    p_irq = r_irq.read();
  }

  tmpl(/**/)::VciSigmaDelta(
    sc_module_name insname,
    const IntTab &index,
    const MappingTable &mt,
    uint32_t ident)
     : soclib::caba::BaseModule(insname),
     m_vci_fsm(p_vci, mt.getSegmentList(index)),
     m_ident(ident),
     first(true),
     r_irq("r_irq"),
     r_sigma("r_sigma"),
     r_counter("r_counter"),
     p_sigma("sigma"),
     p_resetn("resetn"),
     p_clk("clk"),
     p_vci("vci"),
     p_irq("irq")
  {
    m_vci_fsm.on_read_write(on_read, on_write);
    
    SC_METHOD(transition);
    dont_initialize();
    sensitive << p_clk.pos();
	
    SC_METHOD(genMoore);
    dont_initialize();
    sensitive << p_clk.neg();

  }

  tmpl(/**/)::~VciSigmaDelta(){;}

}}
