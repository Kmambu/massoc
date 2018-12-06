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

#ifndef SOCLIB_CABA_VCI_SENSOR_ADC_H
#define SOCLIB_CABA_VCI_SENSOR_ADC_H

#include "systemc-ams.h"
#include <systemc>
#include "caba_base_module.h"
#include "interface_vci/vci_sigma_delta.h"
#include "vci_target.h"
//#include "sensor/sensor.h"
#include "sigmadelta.h"
#include "decimator.h"

namespace soclib {
namespace caba {

using namespace sc_core;

template<typename vci_param,int nb_bits_decim>
class VciSensorAdc
  : public soclib::caba::BaseModule
{
public:
  uint32_t m_ident;
  soclib::caba::VciSigmaDelta<vci_param,nb_bits_decim> i_vci_interface;
  decimator<nb_bits_decim> i_decimator;
  sigmadelta i_sigmadelta;
  //  sensor i_sensor; 
private:

  sca_tdf::sca_signal <double> s_sigmadelta_decim;
  sc_signal<sc_dt::sc_uint<nb_bits_decim> > s_decim_vci;
  
  sc_time Tb;
  double ampl;
  double fs;

 protected:
  SC_HAS_PROCESS(VciSensorAdc);

 public:
  sc_in<bool> p_clk;
  sc_in<bool> p_clk_comparator;
  sc_in<bool> p_resetn;
  sc_out<bool> p_irq;
  sca_tdf::sca_in < double  > p_sensor;
  soclib::caba::VciTarget<vci_param> p_vci;
  
  VciSensorAdc(sc_module_name insname,
	       const soclib::common::IntTab &index,
	       const soclib::common::MappingTable &mt,
	       uint32_t ident);
  ~VciSensorAdc();
  void init(int OSR, double BW, double ampl);
  
};
 
}}

#endif
