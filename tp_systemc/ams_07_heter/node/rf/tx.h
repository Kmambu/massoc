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
      //                         Fichier : ./tx.h
      //                                                                           
      // Language   : C++     Version : 1.0    Date : Mon Jun 9 14:35:09 CEST 2008
      //
      // Author     : Michel Vasilevski
      // 
      // Laboratory : LIP6/SOC, University Pierre and Marie Curie, Paris 6
      //                                                                           
      //***************************************************************************
#ifndef TX_H
#define TX_H

#include "systemc-ams.h"

//#include "interface_sc2sdf.h"
#include "transmitter.h"

SC_MODULE (tx)
{
  sc_in < bool > en;
  sc_in < bool > digital_in;
  sca_tdf::sca_out < BB > bb_out;

  //sca_sdf_signal < sc_uint<1> >sig_interface_sc2sdf;

  //interface_sc2sdf *i_interface_sc2sdf;
  transmitter *i_transmitter;

  void init (double fs, double fc, double fb, double ampl, double Eb){
    //i_interface_sc2sdf->init(fb/2);
    i_transmitter->init(Eb, fs, fc, fb/2);

  }
  SC_CTOR (tx) {

/*     i_interface_sc2sdf=new interface_sc2sdf("i_interface_sc2sdf"); */
/*     i_interface_sc2sdf->in(digital_in); */
/*     i_interface_sc2sdf->out(sig_interface_sc2sdf); */

    i_transmitter=new transmitter("i_transmitter");
    i_transmitter->en(en);
/*     i_transmitter->in(sig_interface_sc2sdf); */
    i_transmitter->in(digital_in);
    i_transmitter->out(bb_out);

  }
};
#endif
