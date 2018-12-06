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
      //                         Fichier : ./lo.h
      //                                                                           
      // Language   : C++     Version : 1.0    Date : Mon Jun 9 14:35:15 CEST 2008
      //
      // Author     : Michel Vasilevski
      // 
      // Laboratory : LIP6/SOC, University Pierre and Marie Curie, Paris 6
      //                                                                           
      //***************************************************************************
#ifndef LO_H
#define LO_H

#include "systemc-ams.h"

#include "constant.h"
#include "baseband.h"

SCA_TDF_MODULE (lo)
{
  sca_tdf::sca_out < BB >outI;
  sca_tdf::sca_out < BB >outQ;

  double phase_mismatch;
  double pulsation_offset;
  double gain_mismatch;
  double wc;

  void processing () {
    BB I(
        0,
        (1+(gain_mismatch/2))*cos(pulsation_offset*get_time().to_seconds()+phase_mismatch/2),
        0,
        0,
        -(1+(gain_mismatch/2))*sin(pulsation_offset*get_time().to_seconds()+phase_mismatch/2),
        0,
        0,
        wc
        );
    BB Q(
        0,
        (1-(gain_mismatch/2))*sin(pulsation_offset*get_time().to_seconds()-phase_mismatch/2),
        0,
        0,
        (1-(gain_mismatch/2))*cos(pulsation_offset*get_time().to_seconds()-phase_mismatch/2),
        0,
        0,
        wc
        );
    outI.write (I);
    outQ.write (Q);
  }
  void init (double fc, double f_o, double ph_m, double g_m) {
    this->wc=2*M_PI*fc;
    this->pulsation_offset=2*M_PI*f_o;
    this->phase_mismatch=ph_m;
    this->gain_mismatch=g_m;
  }
  void set_phase_mismatch(double ph_m){
    phase_mismatch=ph_m;
  }
  void set_frequency_offset(double f_o){
    pulsation_offset=2*M_PI*f_o;
  }
  void set_gain_mismatch(double g_m){
    gain_mismatch=g_m;
  }
  SCA_CTOR (lo) {}
};
#endif
