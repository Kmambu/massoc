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
 * Copyright (c) UPMC, Lip6, SoC
 */

#ifndef _NODE_H
#define _NODE_H

#include <iostream>
#include <cstdlib>
#include "systemc-ams.h"
#include "caba_base_module.h"
#include "mapping_table.h"
#include "mips32.h"
#include "vci_xcache_wrapper.h"
#include "vci_ram.h"
#include "vci_multi_tty.h"
#include "vci_vgmn.h"
#include "vci_sensor_adc.h"
#include "vci_uart.h"
#include "vci_timer.h"

#include "../segmentation.h"
#include "../coords.h"

using namespace sc_core;
using soclib::common::IntTab;
using soclib::common::Segment;

template<typename vci_param,uint8_t nb_bits_decim>
class node : public soclib::caba::BaseModule
{
  const int OSR;
  const double BW;
  const double amplitude;
  std::ostringstream ttyname;

  sc_clock s_clk_comparator;
  uint32_t m_ident;
  
public:
  soclib::common::MappingTable *maptab;
  soclib::caba::VciXcacheWrapper<vci_param, soclib::common::Mips32ElIss > *mips0;
  soclib::common::Loader *loader;
  soclib::caba::VciRam<vci_param> *vcimultiram;
  soclib::caba::VciMultiTty<vci_param> *vcitty;
  soclib::caba::VciVgmn<vci_param> *vgmn;
  soclib::caba::VciUart<vci_param> *uart;
  soclib::caba::VciSensorAdc<vci_param,nb_bits_decim> *sensor;
  soclib::caba::VciTimer<vci_param> *timer;

  sc_signal<bool> signal_mips0_it0; 
  sc_signal<bool> signal_mips0_it1; 
  sc_signal<bool> signal_mips0_it2; 
  sc_signal<bool> signal_mips0_it3; 
  sc_signal<bool> signal_mips0_it4; 
  sc_signal<bool> signal_mips0_it5;
  sc_signal<bool> signal_tty_irq0 ; 
  
  soclib::caba::VciSignals<vci_param> signal_vci_m0;
  soclib::caba::VciSignals<vci_param> signal_vci_tty;
  soclib::caba::VciSignals<vci_param> signal_vci_uart;
  soclib::caba::VciSignals<vci_param> signal_vci_vcimultiram;
  soclib::caba::VciSignals<vci_param> signal_vci_sensor;
  soclib::caba::VciSignals<vci_param> signal_vci_timer;

  sc_core::sc_in <bool> p_clk;
  sc_core::sc_in <bool> p_resetn;
  sc_core::sc_in <bool> p_rx;
  sc_core::sc_out<bool> p_tx;
  sc_core::sc_out<bool> p_tx_en;
  sc_core::sc_out<bool> p_rx_en;
  sca_tdf::sca_in < double > p_sensor;
	
  node(sc_core::sc_module_name nm, int ident, char *softname)
    : soclib::caba::BaseModule(nm),
    OSR(32),
    BW(50e3),
    amplitude(0.56),
    s_clk_comparator("s_clk_comparator", sc_time(1/(2.0 * BW * OSR),SC_SEC)),
    m_ident(ident),
    signal_mips0_it0("signal_mips0_it0"),
    signal_mips0_it1("signal_mips0_it1"),
    signal_mips0_it2("signal_mips0_it2"),
    signal_mips0_it3("signal_mips0_it3"),
    signal_mips0_it4("signal_mips0_it4"),
    signal_mips0_it5("signal_mips0_it5"),
    signal_tty_irq0 ("signal_tty_irq0"),
    signal_vci_m0   ("signal_vci_m0"),		       
    signal_vci_tty  ("signal_vci_tty"),   
    signal_vci_uart ("signal_vci_uart"),
    signal_vci_vcimultiram("signal_vci_vcimultiram"),
    signal_vci_sensor ("signal_vci_sensor")
      {
	ttyname << "Sensor " << m_ident;
	maptab = new soclib::common::MappingTable(32, IntTab(8), IntTab(8), 0x00300000);

	maptab->add(Segment("reset", RESET_BASE, RESET_SIZE, IntTab(0), true));
	maptab->add(Segment("excep", EXCEP_BASE, EXCEP_SIZE, IntTab(0), true));
	maptab->add(Segment("text" , TEXT_BASE , TEXT_SIZE , IntTab(0), true));
	maptab->add(Segment("data" , DATA_BASE , DATA_SIZE , IntTab(0), true));
	maptab->add(Segment("tty"  , TTY_BASE  , TTY_SIZE  , IntTab(1), false));
	maptab->add(Segment("uart" , UART_BASE , UART_SIZE , IntTab(2), false));
	maptab->add(Segment("sensor",SENSOR_BASE,SENSOR_SIZE,IntTab(3), false));
	maptab->add(Segment("timer" , TIMER_BASE , TIMER_SIZE , IntTab(4), false));
      
	mips0  = new soclib::caba::VciXcacheWrapper<vci_param, soclib::common::Mips32ElIss >
	  ("mips0",0 ,*maptab, IntTab(0), 4,1,8,4,1,8);
	loader = new soclib::common::Loader(softname);
	vcimultiram = new soclib::caba::VciRam<vci_param>("vcimultiram", IntTab(0), *maptab, *loader);
	vcitty = new soclib::caba::VciMultiTty<vci_param>("vcitty", IntTab(1), *maptab, (ttyname.str()).c_str(), NULL);
	vgmn   = new soclib::caba::VciVgmn<vci_param>("vgmn",*maptab, 1, 5, 2, 8);
	uart   = new soclib::caba::VciUart<vci_param>("uart",IntTab(2), *maptab,false,m_ident);
	sensor = new soclib::caba::VciSensorAdc<vci_param,nb_bits_decim>("sensor",IntTab(3), *maptab,m_ident);
	timer  = new soclib::caba::VciTimer<vci_param>("timer", IntTab(4), *maptab, 1);

	vcimultiram->p_clk(p_clk);
	vcimultiram->p_resetn(p_resetn);
	vcimultiram->p_vci(signal_vci_vcimultiram);

	mips0->p_clk(p_clk);  
	mips0->p_resetn(p_resetn);  
	mips0->p_irq[0](signal_mips0_it0);
	mips0->p_irq[1](signal_mips0_it1); 
	mips0->p_irq[2](signal_mips0_it2); 
	mips0->p_irq[3](signal_mips0_it3); 
	mips0->p_irq[4](signal_mips0_it4); 
	mips0->p_irq[5](signal_mips0_it5); 
	mips0->p_vci(signal_vci_m0);
	
	vcitty->p_clk(p_clk);
	vcitty->p_resetn(p_resetn);
	vcitty->p_vci(signal_vci_tty);
	vcitty->p_irq[0](signal_tty_irq0); 

	uart->p_clk(p_clk);
	uart->p_resetn(p_resetn);
	uart->p_irq(signal_mips0_it1);
	uart->p_vci(signal_vci_uart);
	uart->p_tx(p_tx);
	uart->p_rx(p_rx);
	uart->p_tx_en(p_tx_en);
	uart->p_rx_en(p_rx_en);

	vgmn->p_clk(p_clk);
	vgmn->p_resetn(p_resetn);
	vgmn->p_to_initiator[0](signal_vci_m0);
	vgmn->p_to_target[0](signal_vci_vcimultiram);
	vgmn->p_to_target[1](signal_vci_tty);
	vgmn->p_to_target[2](signal_vci_uart);
	vgmn->p_to_target[3](signal_vci_sensor);
	vgmn->p_to_target[4](signal_vci_timer);

	sensor->p_clk(p_clk);
	sensor->p_resetn(p_resetn);
	sensor->p_clk_comparator(s_clk_comparator);
	sensor->p_vci(signal_vci_sensor);
	sensor->p_sensor(p_sensor);
	sensor->p_irq(signal_mips0_it0);
	sensor->init(OSR,BW,amplitude);

	timer->p_clk(p_clk);
	timer->p_resetn(p_resetn);
	timer->p_vci(signal_vci_timer);
	timer->p_irq[0](signal_mips0_it2); 

      }
};

#endif

