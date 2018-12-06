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
      //                         Fichier : ./adder.h
      //                                                                           
      // Language   : C++     Version : 1.0    Date : Mon Jun 9 14:35:14 CEST 2008
      //
      // Author     : Michel Vasilevski
      // 
      // Laboratory : LIP6/SOC, University Pierre and Marie Curie, Paris 6
      //                                                                           
      //***************************************************************************
#ifndef ADDER_H
#define ADDER_H

#include "systemc-ams.h"
#include "baseband.h"
SCA_TDF_MODULE (adder)
{
  sca_tdf::sca_de::sca_in < bool > en;
  sca_tdf::sca_in < BB >inI;
  sca_tdf::sca_in < BB >inQ;
  sca_tdf::sca_out < BB >out;

  double gainI_power;
  double gainQ_power;
  double gainI_voltage;
  double gainQ_voltage;

  double rinI;
  double rinQ;
  double rout;
  double *rload;
  BB xx;

  void processing () {
    gainI_voltage=sqrt(gainI_power/(*rload*rinI))*(rout+*rload);
    gainQ_voltage=sqrt(gainQ_power/(*rload*rinQ))*(rout+*rload);
	if (en.read()==true)
    out.write (gainI_voltage*inI.read()+gainQ_voltage*inQ.read());
	else
    out.write (xx);
  }

  void init (double gainI_power_db, double gainQ_power_db, double rinI, double rinQ, double rout, double *rload) {
    this->rinI=rinI;
    this->rinQ=rinQ;
    this->rout=rout;
    this->rload=rload;
    this->gainI_power=pow(10,gainI_power_db/10);
    this->gainQ_power=pow(10,gainQ_power_db/10);
  }

  SCA_CTOR (adder) :
	xx() {}
};
#endif
