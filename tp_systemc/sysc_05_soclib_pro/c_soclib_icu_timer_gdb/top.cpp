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
 *         Francois Pecheux <francois.pecheux@lip6.fr>
 *         Antoine LEVEQUE <antoine.leveque@lip6.fr>
 *
 */

#include <iostream>
#include <cstdlib>

#include "systemc.h"
#include "node.h"

//small function for binding name
const char * tostring(const char * name, int val){
  std::ostringstream temp;
  temp << name << val;
  return (temp.str()).c_str();
}

int _main(int argc, char *argv[])
{
  ////////////////////////////////////////////////////////////////////
  // Signals                                                        //
  ////////////////////////////////////////////////////////////////////

  sc_clock	s_clk("s_clk", sc_core::sc_time(10, sc_core::SC_NS));
  sc_signal<bool> s_resetn("s_resetn");

  ////////////////////////////////////////////////////////////////////
  // Instances                                                      //
  ////////////////////////////////////////////////////////////////////

  node *n[1];

  ////////////////////////////////////////////////////////////////////
  // Netlist                                                        //
  ////////////////////////////////////////////////////////////////////

  n[0] = new node (tostring("n",0),0,"soft/bin.soft");
  n[0]->p_clk(s_clk);
  n[0]->p_resetn(s_resetn);

  ////////////////////////////////////////////////////////////////////
  // Simulation                                                     //
  ////////////////////////////////////////////////////////////////////

  sc_start(sc_core::sc_time(0, sc_core::SC_NS));
  s_resetn = false;
  sc_start(sc_core::sc_time(1, sc_core::SC_NS));
  s_resetn = true;

  sc_core::sc_start();
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
