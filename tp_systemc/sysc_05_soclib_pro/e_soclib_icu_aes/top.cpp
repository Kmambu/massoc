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
 *         Nicolas Pouillon <nipo@ssji.net>, 2006-2007
 *
 * Maintainers: nipo
 */

#include <iostream>
#include <cstdlib>

////////////////////////////////////////////////////////////////////
// Part 1 : Include files                                         //
////////////////////////////////////////////////////////////////////

#include "mapping_table.h"

#include "mips32.h"
#include "vci_xcache_wrapper.h"
#include "vci_ram.h"
#include "vci_icu.h"
#include "vci_multi_tty.h"
#include "vci_aes.h"
#include "vci_vgmn.h"

#include "segmentation.h"

int _main(int argc, char *argv[])
{
  using namespace sc_core;
  // Avoid repeating these everywhere
  using soclib::common::IntTab;
  using soclib::common::Segment;

  ////////////////////////////////////////////////////////////////////
  // Part 2 : Mapping table                                         //
  ////////////////////////////////////////////////////////////////////

  // Define our VCI parameters
  typedef soclib::caba::VciParams<4,6,32,1,1,1,8,1,1,1> vci_param;

  // Mapping table

  soclib::common::MappingTable maptab(32, IntTab(8), IntTab(8), 0x00300000);

  // RAM segments.
  maptab.add(Segment("reset", RESET_BASE, RESET_SIZE, IntTab(0), true));
  maptab.add(Segment("excep", EXCEP_BASE, EXCEP_SIZE, IntTab(0), true));
  maptab.add(Segment("text" , TEXT_BASE , TEXT_SIZE , IntTab(0), true));
  maptab.add(Segment("data" , DATA_BASE , DATA_SIZE , IntTab(0), true));

  maptab.add(Segment("icu", ICU_BASE, ICU_SIZE, IntTab(1), false));
  maptab.add(Segment("tty", TTY_BASE, TTY_SIZE, IntTab(2), false));

  maptab.add(Segment("aes_conf", AES_CONF_BASE, AES_CONF_SIZE, IntTab(3), false));
  maptab.add(Segment("aes_data", AES_DATA_BASE, AES_DATA_SIZE, IntTab(3), false));

  ////////////////////////////////////////////////////////////////////
  // Part 3 : signals                                               //
  ////////////////////////////////////////////////////////////////////

  sc_clock        signal_clk("signal_clk");
  sc_signal<bool> signal_resetn("signal_resetn");

  sc_signal<bool> signal_mips_it0("signal_mips_it0");
  sc_signal<bool> signal_mips_it1("signal_mips_it1");
  sc_signal<bool> signal_mips_it2("signal_mips_it2");
  sc_signal<bool> signal_mips_it3("signal_mips_it3");
  sc_signal<bool> signal_mips_it4("signal_mips_it4");
  sc_signal<bool> signal_mips_it5("signal_mips_it5");

  sc_signal<bool> signal_tty_irq0("signal_tty_irq0");
  sc_signal<bool> signal_aes_irq0("signal_aes_irq0");

  soclib::caba::VciSignals<vci_param> signal_vci_mips("signal_vci_mips");
  soclib::caba::VciSignals<vci_param> signal_vci_ram("signal_vci_ram");
  soclib::caba::VciSignals<vci_param> signal_vci_icu("signal_vci_icu");
  soclib::caba::VciSignals<vci_param> signal_vci_tty("signal_vci_tty");
  soclib::caba::VciSignals<vci_param> signal_vci_aes("signal_vci_aes");


  ////////////////////////////////////////////////////////////////////
  // Part 4 : instances                                             //
  ////////////////////////////////////////////////////////////////////

  soclib::caba::VciXcacheWrapper<
    vci_param,
    soclib::common::Mips32ElIss
    > mips_cache("mips_cache", 0, maptab,IntTab(0), 4,1,8, 4,1,8);

  soclib::common::Loader loader("soft/bin.soft");
  soclib::caba::VciRam<vci_param> vciram("vciram", IntTab(0), maptab, loader);

  soclib::caba::VciIcu<vci_param> vciicu("vciicu", IntTab(1), maptab, 1);
  soclib::caba::VciMultiTty<vci_param> vcitty("vcitty", IntTab(2), maptab, "vcitty0", NULL);
  soclib::caba::VciAes<vci_param> vciaes("vciaes", IntTab(3), maptab);

  soclib::caba::VciVgmn<vci_param> vgmn("vgmn", maptab, 1, 4, 2, 8);

  ////////////////////////////////////////////////////////////////////
  // Part 5 : netlist                                               //
  ////////////////////////////////////////////////////////////////////

  mips_cache.p_clk(signal_clk);
  mips_cache.p_resetn(signal_resetn);
  mips_cache.p_irq[0](signal_mips_it0);
  mips_cache.p_irq[1](signal_mips_it1);
  mips_cache.p_irq[2](signal_mips_it2);
  mips_cache.p_irq[3](signal_mips_it3);
  mips_cache.p_irq[4](signal_mips_it4);
  mips_cache.p_irq[5](signal_mips_it5);
  mips_cache.p_vci(signal_vci_mips);

  vciram.p_clk(signal_clk);
  vciram.p_resetn(signal_resetn);
  vciram.p_vci(signal_vci_ram);

  vciicu.p_clk(signal_clk);
  vciicu.p_resetn(signal_resetn);
  vciicu.p_vci(signal_vci_icu);
  vciicu.p_irq(signal_mips_it0);
  vciicu.p_irq_in[0](signal_aes_irq0);

  vcitty.p_clk(signal_clk);
  vcitty.p_resetn(signal_resetn);
  vcitty.p_vci(signal_vci_tty);
  vcitty.p_irq[0](signal_tty_irq0);

  vciaes.p_clk(signal_clk);
  vciaes.p_resetn(signal_resetn);
  vciaes.p_vci(signal_vci_aes);
  vciaes.p_irq(signal_aes_irq0);

  vgmn.p_clk(signal_clk);
  vgmn.p_resetn(signal_resetn);
  vgmn.p_to_initiator[0](signal_vci_mips);
  vgmn.p_to_target[0](signal_vci_ram);
  vgmn.p_to_target[1](signal_vci_icu);
  vgmn.p_to_target[2](signal_vci_tty);
  vgmn.p_to_target[3](signal_vci_aes);

  ////////////////////////////////////////////////////////////////////
  // Part 5bis : reset                                              //
  ////////////////////////////////////////////////////////////////////

  sc_start(sc_core::sc_time(0, SC_NS));
  signal_resetn = false;
  sc_start(sc_core::sc_time(1, SC_NS));
  signal_resetn = true;

  ////////////////////////////////////////////////////////////////////
  // Part 6 : simulate                                              //
  ////////////////////////////////////////////////////////////////////

  sc_start();
  return EXIT_SUCCESS;
}

int sc_main (int argc, char *argv[])
{
  try {
    return _main(argc, argv);
  } catch (std::exception &e) {
    std::cout << e.what() << std::endl;
  } catch (...) {
    std::cout << "Unknown exception occured" << std::endl;
    throw;
  }
  return 1;
}
