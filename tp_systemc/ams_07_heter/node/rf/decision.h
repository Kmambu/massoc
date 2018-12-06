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
      //                         Fichier : ./decision.h
      //                                                                           
      // Language   : C++     Version : 1.0    Date : Mon Jun 9 14:35:15 CEST 2008
      //
      // Author     : Michel Vasilevski
      // 
      // Laboratory : LIP6/SOC, University Pierre and Marie Curie, Paris 6
      //                                                                           
      //***************************************************************************
#ifndef DECISION_H
#define DECISION_H

#include "systemc-ams.h"

SCA_TDF_MODULE (decision)
{
  sca_tdf::sca_in < double >in;
  sca_tdf::sca_out < sc_uint<1> >out;

  double rin;

  void processing () {
    if (in.read () > 0) {
      out.write (1);
    }
    else {
      out.write (0);
    }
  }
  void init(double rin){
    this->rin=rin;
  }
  SCA_CTOR (decision) {}

};
#endif
