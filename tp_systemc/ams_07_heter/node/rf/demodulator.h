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
      //                         Fichier : ./demodulator.h
      //                                                                           
      // Language   : C++     Version : 1.0    Date : Mon Jun 9 14:35:16 CEST 2008
      //
      // Author     : Michel Vasilevski
      // 
      // Laboratory : LIP6/SOC, University Pierre and Marie Curie, Paris 6
      //                                                                           
      //***************************************************************************
#ifndef DEMODULATOR_H
#define DEMODULATOR_H

#include "systemc-ams.h"

#include "spectrum.h"
#include "noise.h"
#include "constant.h"
#include "baseband.h"
#include "gain.h"

SCA_TDF_MODULE (demodulator)
{
  sca_tdf::sca_in < BB >in;
  sca_tdf::sca_in < BB >in_lo;
  sca_tdf::sca_out < BB >out;

  double gain_power;
  double a1;
  double a3;
  
  double AIP3;
  double sigma;
  
  double dc_offset;

  double rin;
  double *rload;
  double rout;

  void init(double fs, 
            double gain_power_db, 
            double iip3, double nf, 
            double dc_o,
            double rin, double rout, double *rload){
    double f  = pow(10,nf/10);
    double N0 = 4*(f-1)*K*T*50;

    this->sigma=sqrt(N0*fs/2);
    srand (time(NULL)); //randomize

    this->dc_offset=dc_o;
    
    this->rin=rin;
    this->rout=rout;
    this->rload=rload;
    this->gain_power=pow(10,gain_power_db/10);
    this->AIP3=undbm(iip3);
  }
  
  void set_dc_offset(double dc_o){
    dc_offset=dc_o;
  }

  void processing () {
    this->a1 = sqrt(gain_power/(*rload*rin))*(rout+*rload);
    this->a3 = this->a1/(3.*pow(AIP3,2.)/4.);

    BB noise(sigma*randn(),sigma*randn(),sigma*randn(),sigma*randn(),sigma*randn(),sigma*randn(),sigma*randn(),in_lo.read().w);
    BB input=in.read()*in_lo.read()+noise;
    out.write(GAIN(input)+dc_offset);
  }
  SCA_CTOR (demodulator) {}
};
#endif
