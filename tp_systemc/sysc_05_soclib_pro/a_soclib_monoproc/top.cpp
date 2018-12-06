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
// TME exercice: component includes.

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

  // TME exercise: map missing component segment(s).

  ////////////////////////////////////////////////////////////////////
  // Part 3 : signals                                               //
  ////////////////////////////////////////////////////////////////////

  sc_clock	signal_clk("signal_clk");
  sc_signal<bool> signal_resetn("signal_resetn");

  sc_signal<bool> signal_mips0_it0("signal_mips0_it0");
  sc_signal<bool> signal_mips0_it1("signal_mips0_it1");
  sc_signal<bool> signal_mips0_it2("signal_mips0_it2");
  sc_signal<bool> signal_mips0_it3("signal_mips0_it3");
  sc_signal<bool> signal_mips0_it4("signal_mips0_it4");
  sc_signal<bool> signal_mips0_it5("signal_mips0_it5");

  sc_signal<bool> signal_tty_irq0("signal_tty_irq0");

  soclib::caba::VciSignals<vci_param> signal_vci_m0("signal_vci_m0");

  soclib::caba::VciSignals<vci_param> signal_vci_vcimultiram0("signal_vci_vcimultiram0");
  soclib::caba::VciSignals<vci_param> signal_vci_tty("signal_vci_tty");


  ////////////////////////////////////////////////////////////////////
  // Part 4 : instances                                             //
  ////////////////////////////////////////////////////////////////////

  soclib::caba::VciXcacheWrapper<
    vci_param,
    soclib::common::Mips32ElIss
    > cache0("cache0", 0, maptab,IntTab(0), 4,1,8, 4,1,8);

  soclib::common::Loader loader("soft/bin.soft");
  soclib::caba::VciRam<vci_param> vcimultiram0("vcimultiram0", IntTab(0), maptab, loader);
  soclib::caba::VciMultiTty<vci_param> vcitty("vcitty", IntTab(1), maptab, "vcitty0", NULL);

  soclib::caba::VciVgmn<vci_param> vgmn("vgmn", maptab, 1, 2, 2, 8);

  ////////////////////////////////////////////////////////////////////
  // Part 5 : netlist                                               //
  ////////////////////////////////////////////////////////////////////

  cache0.p_clk(signal_clk);
  cache0.p_resetn(signal_resetn);
  cache0.p_irq[0](signal_mips0_it0);
  cache0.p_irq[1](signal_mips0_it1);
  cache0.p_irq[2](signal_mips0_it2);
  cache0.p_irq[3](signal_mips0_it3);
  cache0.p_irq[4](signal_mips0_it4);
  cache0.p_irq[5](signal_mips0_it5);
  cache0.p_vci(signal_vci_m0);

  // TME exercice: fill-in the netlist.

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
