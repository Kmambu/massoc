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
      //                         Fichier : ./channel_AWGN.h
      //                                                                           
      // Language   : C++     Version : 1.0    Date : Mon Jun 9 14:35:12 CEST 2008
      //
      // Author     : Michel Vasilevski
      // 
      // Laboratory : LIP6/SOC, University Pierre and Marie Curie, Paris 6
      //                                                                           
      //***************************************************************************
#ifndef CHANNEL_AWGN_N_H
#define CHANNEL_AWGN_N_H

#include <iostream>

#include "systemc-ams.h"

#include "noise.h"

SCA_TDF_MODULE (channel_AWGN_n)
{
  sca_tdf::sca_in < BB >in[4];
  sca_tdf::sca_out < BB >out[4];

  double sigma;
  double fs;
  double Eb;

  void set_snr(double snr)
  {
    double N0=Eb/pow(10,snr/10);
  
    sigma = sqrt(N0*fs/2);
    srand (time(NULL)); //randomize
  }
    
  void processing ()
  {

// 	  BB x0(sigma*randn(),
// 		sigma*randn(),
// 		sigma*randn(),
// 		sigma*randn(),
// 		sigma*randn(),
// 		sigma*randn(),
// 		sigma*randn(),
// 		in[0].read().w);

// 	  BB v=in[0].read()+in[1].read()+in[2].read()+in[3].read()+x0;
	  BB v=in[0].read()+in[1].read()+in[2].read()+in[3].read();

	  out[0].write(v);
	  out[1].write(v);
	  out[2].write(v);
	  out[3].write(v);
  }

  void init(double snr, double fs, double Eb)
  {
    this->Eb=Eb;
    this->fs=fs;

    double N0=Eb/pow(10,snr/10);
    this->sigma = sqrt(N0*fs/2);
    srand (time(NULL)); //randomize
  }

  SCA_CTOR (channel_AWGN_n) {}

};
#endif
