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
      //                         Fichier : ./integrator.h
      //                                                                           
      // Language   : C++     Version : 1.0    Date : Mon Jun 9 14:35:16 CEST 2008
      //
      // Author     : Michel Vasilevski
      // 
      // Laboratory : LIP6/SOC, University Pierre and Marie Curie, Paris 6
      //                                                                           
      //***************************************************************************
#ifndef INTEGRATOR_H
#define INTEGRATOR_H

#include "systemc-ams.h"

SCA_TDF_MODULE (integrator)
{
  sca_tdf::sca_in < BB >in;
  sca_tdf::sca_out < double >out;

  double fsym;
  
  double gain_power;
  double gain_voltage;
  
  double rin;
  double rout;
  double *rload;

  void processing () {
    gain_voltage=sqrt(gain_power/(*rload*rin))*(rout+*rload);
    out.write (gain_voltage*(in.read().DC/fsym));
  }
  void init (double fb, double gain_power_db,
             double rin, double rout, double *rload) {
    fsym=fb/2;
    this->rin=rin;
    this->rout=rout;
    this->rload=rload;
    this->gain_power=pow(10,gain_power_db/10);
  }
  SCA_CTOR (integrator) {}
};
#endif
