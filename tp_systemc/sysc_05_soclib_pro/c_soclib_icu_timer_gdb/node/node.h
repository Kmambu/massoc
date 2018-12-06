
/*
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

#include "mapping_table.h"
#include "mips32.h"
#include "vci_xcache_wrapper.h"
#include "vci_ram.h"
#include "vci_multi_tty.h"
#include "vci_timer.h"
#include "vci_icu.h"
#include "vci_vgmn.h"
#include "../segmentation.h"

SC_MODULE(node)
{
  sc_core::sc_in<bool> p_clk;
  sc_core::sc_in<bool> p_resetn;

  uint32_t m_ident;

  node(sc_core::sc_module_name nm, int ident, char *softname)
    : sc_core::sc_module(nm),
    m_ident(ident)
    {
      using namespace sc_core;
      using soclib::common::IntTab;
      using soclib::common::Segment;

      ////////////////////////////////////////////////////////////////////
      // Part 2 : Mapping table                                         //
      ////////////////////////////////////////////////////////////////////

      soclib::common::MappingTable *maptab;
      maptab = new soclib::common::MappingTable(32, IntTab(8), IntTab(8), 0x00300000);

      maptab->add(Segment("reset", RESET_BASE, RESET_SIZE, IntTab(0), true));
      maptab->add(Segment("excep", EXCEP_BASE, EXCEP_SIZE, IntTab(0), true));
      maptab->add(Segment("text" , TEXT_BASE , TEXT_SIZE , IntTab(0), true));
      maptab->add(Segment("data" , DATA_BASE , DATA_SIZE , IntTab(0), true));

      maptab->add(Segment("tty"   , TTY_BASE   , TTY_SIZE   , IntTab(1), false));
      maptab->add(Segment("timer" , TIMER_BASE , TIMER_SIZE , IntTab(2), false));
      maptab->add(Segment("icu"   , ICU_BASE   , ICU_SIZE   , IntTab(3), false));

      // Define our VCI parameters
      typedef	soclib::caba::VciParams<4,6,32,1,1,1,8,1,1,1> vci_param;

      ////////////////////////////////////////////////////////////////////
      // Part 3 : Signals declaration                                   //
      ////////////////////////////////////////////////////////////////////

      sc_core::sc_signal<bool> *s_mips_it0;
      sc_core::sc_signal<bool> *s_mips_it1;
      sc_core::sc_signal<bool> *s_mips_it2;
      sc_core::sc_signal<bool> *s_mips_it3;
      sc_core::sc_signal<bool> *s_mips_it4;
      sc_core::sc_signal<bool> *s_mips_it5;
      sc_core::sc_signal<bool> *s_tty_irq;
      sc_core::sc_signal<bool> *s_timer_irq;
      soclib::caba::VciSignals<vci_param> *s_m;
      soclib::caba::VciSignals<vci_param> *s_ram;
      soclib::caba::VciSignals<vci_param> *s_tty;
      soclib::caba::VciSignals<vci_param> *s_timer;
      soclib::caba::VciSignals<vci_param> *s_icu;
      soclib::caba::VciSignals<vci_param> *s_sensor;

      s_mips_it0 = new sc_core::sc_signal<bool> ("s_mips_it0");
      s_mips_it1 = new sc_core::sc_signal<bool> ("s_mips_it1");
      s_mips_it2 = new sc_core::sc_signal<bool> ("s_mips_it2");
      s_mips_it3 = new sc_core::sc_signal<bool> ("s_mips_it3");
      s_mips_it4 = new sc_core::sc_signal<bool> ("s_mips_it4");
      s_mips_it5 = new sc_core::sc_signal<bool> ("s_mips_it5");

      s_tty_irq = new sc_core::sc_signal<bool> ("s_tty_irq");
      s_timer_irq = new sc_core::sc_signal<bool> ("s_timer_irq");
      s_m = new soclib::caba::VciSignals<vci_param> ("s_m");
      s_ram = new soclib::caba::VciSignals<vci_param> ("s_ram");
      s_tty = new soclib::caba::VciSignals<vci_param> ("s_tty");
      s_timer = new soclib::caba::VciSignals<vci_param> ("s_timer");
      s_icu = new soclib::caba::VciSignals<vci_param> ("s_icu");

      ////////////////////////////////////////////////////////////////////
      // Part 4 : instances                                             //
      ////////////////////////////////////////////////////////////////////

      soclib::caba::VciXcacheWrapper<vci_param, soclib::common::Mips32ElIss > *cache;
      soclib::common::Loader *loader;
      soclib::caba::VciRam<vci_param> *ram;
      soclib::caba::VciMultiTty<vci_param> *tty;
      soclib::caba::VciTimer<vci_param> *timer;
      soclib::caba::VciIcu<vci_param> *icu;
      soclib::caba::VciVgmn<vci_param> *vgmn;

      cache = new soclib::caba::VciXcacheWrapper<vci_param, soclib::common::Mips32ElIss >
	("cache",0 , *maptab,IntTab(0), 4,1,8, 4,1,8);
      loader = new soclib::common::Loader(softname);
      ram = new soclib::caba::VciRam<vci_param>("ram", IntTab(0), *maptab, *loader);
      char ttyname[20];
      sprintf(ttyname,"tty%d",m_ident);
      tty = new soclib::caba::VciMultiTty<vci_param> ("tty",IntTab(1), *maptab, ttyname, NULL);
      timer = new soclib::caba::VciTimer<vci_param>("timer", IntTab(2), *maptab, 1);
      icu = new soclib::caba::VciIcu<vci_param>("icu",IntTab(3), *maptab,1);
      vgmn = new soclib::caba::VciVgmn<vci_param>("vgmn",*maptab, 1, 4, 2, 8);

      ////////////////////////////////////////////////////////////////////
      // Part 5 : netlist                                               //
      ////////////////////////////////////////////////////////////////////

      cache->p_clk(p_clk);
      cache->p_resetn(p_resetn);
      cache->p_irq[0](*s_mips_it0);
      cache->p_irq[1](*s_mips_it1);
      cache->p_irq[2](*s_mips_it2);
      cache->p_irq[3](*s_mips_it3);
      cache->p_irq[4](*s_mips_it4);
      cache->p_irq[5](*s_mips_it5);
      cache->p_vci(*s_m);

      ram->p_clk(p_clk);
      ram->p_resetn(p_resetn);
      ram->p_vci(*s_ram);

      tty->p_clk(p_clk);
      tty->p_resetn(p_resetn);
      tty->p_irq[0](*s_tty_irq);
      tty->p_vci(*s_tty);

      timer->p_clk(p_clk);
      timer->p_resetn(p_resetn);
      timer->p_vci(*s_timer);
      timer->p_irq[0](*s_timer_irq);

      icu->p_clk(p_clk);
      icu->p_resetn(p_resetn);
      icu->p_vci(*s_icu);
      icu->p_irq_in[0](*s_timer_irq);
      icu->p_irq(*s_mips_it0);

      vgmn->p_clk(p_clk);
      vgmn->p_resetn(p_resetn);
      vgmn->p_to_initiator[0](*s_m);
      vgmn->p_to_target[0](*s_ram);
      vgmn->p_to_target[1](*s_tty);
      vgmn->p_to_target[2](*s_timer);
      vgmn->p_to_target[3](*s_icu);
    }
};
#endif
