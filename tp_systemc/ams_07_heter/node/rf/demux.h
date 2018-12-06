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
      //                         Fichier : ./demux.h
      //
      // Language   : C++     Version : 1.0    Date : Mon Jun 9 14:35:15 CEST 2008
      //
      // Author     : Michel Vasilevski
      //
      // Laboratory : LIP6/SOC, University Pierre and Marie Curie, Paris 6
      //
      //***************************************************************************
#ifndef DEMUX_H
#define DEMUX_H

#include "systemc-ams.h"

SCA_TDF_MODULE (demux)
{
  sca_tdf::sca_in < double >in;
  sca_tdf::sca_out < double >outI;
  sca_tdf::sca_out < double >outQ;

  double gainI_power;
  double gainQ_power;
  double gainI_voltage;
  double gainQ_voltage;

  double rin;
  double routI;
  double routQ;
  double *rloadI;
  double *rloadQ;

  void set_attributes() {
    in.set_rate(2);
  }
  void processing () {
    gainI_voltage=sqrt(gainI_power/(*rloadI*rin))*(routI+*rloadI);
    gainQ_voltage=sqrt(gainQ_power/(*rloadQ*rin))*(routQ+*rloadQ);
    outI.write(gainI_voltage*in.read(0));
    outQ.write(gainQ_voltage*in.read(1));
  }
  void init(double gainI_power_db, double gainQ_power_db, double rin, double routI, double routQ, double *rloadI, double *rloadQ){
    this->rin=rin;
    this->routI=routI;
    this->routQ=routQ;
    this->rloadI=rloadI;
    this->rloadQ=rloadQ;
    this->gainI_power=pow(10,gainI_power_db/10);
    this->gainQ_power=pow(10,gainQ_power_db/10);
  }
  SCA_CTOR (demux) {}
};
#endif
