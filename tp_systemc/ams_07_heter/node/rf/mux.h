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
      //                         Fichier : ./mux.h
      //                                                                           
      // Language   : C++     Version : 1.0    Date : Mon Jun 9 14:35:16 CEST 2008
      //
      // Author     : Michel Vasilevski
      // 
      // Laboratory : LIP6/SOC, University Pierre and Marie Curie, Paris 6
      //                                                                           
      //***************************************************************************
#ifndef MUX_H
#define MUX_H

#include "systemc-ams.h"

SCA_TDF_MODULE (mux)
{
  sca_tdf::sca_in < sc_uint<1> >inI;
  sca_tdf::sca_in < sc_uint<1> >inQ;
  sca_tdf::sca_out < sc_uint<1> >out;

  void set_attributes() {
	out.set_rate(2);
  }

  void processing () {
    out.write(inI.read(),0);
    out.write(inQ.read(),1);
  }

  SCA_CTOR (mux) {}
};
#endif
