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
      //                         Fichier : ./lna.h
      //                                                                           
      // Language   : C++     Version : 1.0    Date : Mon Jun 9 14:35:15 CEST 2008
      //
      // Author     : Michel Vasilevski
      // 
      // Laboratory : LIP6/SOC, University Pierre and Marie Curie, Paris 6
      //                                                                           
      //***************************************************************************
#ifndef LNA_H
#define LNA_H

#include "systemc-ams.h"

#include "spectrum.h"
#include "noise.h"
#include "constant.h"
#include "gain.h"
#include "baseband.h"

SCA_TDF_MODULE (lna)
{
  sca_tdf::sca_de::sca_in < bool > en;
  sca_tdf::sca_in < BB >in;
  sca_tdf::sca_out < BB >out;

  double gain_power;
  double a1;
  double a3;
  
  double AIP3;
  double sigma;

  double rin;
  double rout;
  double *rloadI;
  double *rloadQ;

  BB xx;

  void processing () {
    double rload=(*rloadI)*(*rloadQ)/((*rloadI)+(*rloadQ));
    this->a1 = sqrt(gain_power/(rload*rin))*(rout+rload);
    this->a3 = a1/(3.*pow(AIP3,2.)/4.);
    
    BB noise(sigma*randn(),sigma*randn(),sigma*randn(),sigma*randn(),sigma*randn(),sigma*randn(),sigma*randn(),in.read().w);
    BB input=in.read()+noise;
	if (en.read()==true)
    out.write (GAIN(input));
	else
    out.write (xx);
  }
  void init(double fs, 
            double gain_power_db, 
            double iip3, double nf,
            double rin, double rout, double *rloadI, double *rloadQ=&rinfinity){
    double f  = pow(10,nf/10);
    double N0 = 4*(f-1)*K*T*50;
    
    this->sigma=sqrt(N0*fs/2);
    srand (time(NULL)); //randomize
    
    this->rin=rin;
    this->rout=rout;
    this->rloadI=rloadI;
    this->rloadQ=rloadQ;
    this->gain_power=pow(10,gain_power_db/10);
    this->AIP3=undbm(iip3);
  }
  SCA_CTOR (lna) :
	xx() {}
};
#endif
