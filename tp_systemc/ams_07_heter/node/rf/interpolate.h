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
      //                         Fichier : ./interpolate.h
      //
      // Language   : C++     Version : 1.0    Date : Mon Jun 9 14:35:14 CEST 2008
      //
      // Author     : Michel Vasilevski
      //
      // Laboratory : LIP6/SOC, University Pierre and Marie Curie, Paris 6
      //
      //***************************************************************************
#ifndef INTERPOLATE_H
#define INTERPOLATE_H
#include "systemc-ams.h"

#include "baseband.h"

SCA_TDF_MODULE (interpolate)
{
  sca_tdf::sca_in < double >in;
  sca_tdf::sca_out < BB >out;

  int ratio;

  void processing () {
    BB x=in.read();
    for (int i=0;i<ratio;i++)
      out.write(x,i);
  }

  void init (int ratio) {
    this->ratio = ratio;
  }

  void set_attributes() {
    out.set_rate(ratio);
  }

  SCA_CTOR (interpolate) {}
};
#endif
