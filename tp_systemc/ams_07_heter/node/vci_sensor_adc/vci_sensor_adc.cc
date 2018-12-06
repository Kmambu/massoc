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

#include "vci_sensor_adc.h" 

namespace soclib {
namespace caba {

#define tmpl(x) template<typename vci_param,int nb_bits_decim> x VciSensorAdc<vci_param,nb_bits_decim>

tmpl(/**/)::VciSensorAdc(sc_module_name insname,
			 const IntTab &index, 
			 const MappingTable &mt,
			 uint32_t ident)
    : soclib::caba::BaseModule(insname),
      m_ident(ident),
      i_vci_interface("i_vci_interface",index, mt, m_ident),
      i_decimator("i_decimator"),
      i_sigmadelta("i_sigmadelta"),
	   //i_sensor("i_sensor"),
      p_clk("clk"),
      p_clk_comparator("clk_comparator"),
      p_resetn("resetn"),
      p_irq("irq"),
      p_sensor("sensor"),
      p_vci("vci")
{
  //  i_sensor.in_sensor(p_sensor); // uniquement pour le debug
  i_sigmadelta.in (p_sensor);
  i_sigmadelta.ck (p_clk_comparator);
  i_sigmadelta.out(s_sigmadelta_decim);
  i_decimator.in (s_sigmadelta_decim);
  i_decimator.out(s_decim_vci);
  i_vci_interface.p_sigma(s_decim_vci);
  i_vci_interface.p_vci(p_vci);
  i_vci_interface.p_clk(p_clk);
  i_vci_interface.p_resetn(p_resetn);
  i_vci_interface.p_irq(p_irq);
}
  
tmpl(/**/)::~VciSensorAdc(){;}
 
tmpl(void)::init(int OSR, double BW, double ampl){
  const int sim_rate = 10;
  this->ampl = ampl;
  this->fs= 2.0 * BW * OSR;
  i_sigmadelta.init(fs, sim_rate);
  i_decimator.init(ampl,sim_rate);
  //i_sensor.set_ident(m_ident);
}
  
}}
