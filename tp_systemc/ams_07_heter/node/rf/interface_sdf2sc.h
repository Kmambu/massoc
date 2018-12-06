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
 */
      //***************************************************************************
      //                         Fichier : ./interface_sdf2sc.h
      //
      // Language   : C++     Version : 1.0    Date : Mon Jun 9 14:35:09 CEST 2008
      //
      // Author     : Michel Vasilevski
      //
      // Laboratory : LIP6/SOC, University Pierre and Marie Curie, Paris 6
      //
      //***************************************************************************
#ifndef INTERFACE_SDF2SC_H
#define INTERFACE_SDF2SC_H

#include "systemc-ams.h"

template <
  int nb_bits >
SCA_TDF_MODULE (interface_sdf2sc)
{
  sca_tdf::sca_in < sc_uint<nb_bits> >in;
  sca_tdf::sc_out < bool >out[nb_bits];

  void set_attributes() {
    // Attention: Added to restore causality when using TDF converter outports in multirate systems
    for (int i=0;i<nb_bits;i++)
      out[i].set_delay(1);
  }

  void processing () {
    sc_uint<nb_bits> val = in.read();
    for (int i=0;i<nb_bits;i++)
      out[i].write(val[i]);
  }

  SCA_CTOR (interface_sdf2sc) {
  }
};
#endif
