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
      //                         Fichier : ./gain.h
      //                                                                           
      // Language   : C++     Version : 1.0    Date : Mon Jun 9 14:35:08 CEST 2008
      //
      // Author     : Michel Vasilevski
      // 
      // Laboratory : LIP6/SOC, University Pierre and Marie Curie, Paris 6
      //                                                                           
      //***************************************************************************
#ifndef IDEAL_GAIN
#define IDEAL_GAIN
#define GAIN(input) (a1*input-a3*pow(input,3))
#include "systemc-ams.h"
#include <limits>

// FPX double rinfinity=(double)numeric_limits<float>::max();
double rinfinity=(double)1.0e100;
double rzero=1./rinfinity;

double ideal_gain(double rin, double rout, double rload){
  return 10.*log10(rload*rin/pow(rout+rload,2.));
}
#endif
