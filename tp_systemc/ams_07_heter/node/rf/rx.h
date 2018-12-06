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
      //                         Fichier : ./mote.h
      //                                                                           
      // Language   : C++     Version : 1.0    Date : Mon Jun 9 14:35:09 CEST 2008
      //
      // Author     : Michel Vasilevski
      // 
      // Laboratory : LIP6/SOC, University Pierre and Marie Curie, Paris 6
      //                                                                           
      //***************************************************************************
#ifndef RX_H
#define RX_H

#include "systemc-ams.h"

#include "interface_sdf2sc.h"
#include "receiver.h"
  
SC_MODULE (rx)
{
  sc_in < bool> en;
  sca_tdf::sca_in < BB > bb_in;
  sc_out < bool > digital_out;

  sc_signal < bool > sig_interface_rf;
  sca_tdf::sca_signal < sc_uint<1> >sig_receiver;

  interface_sdf2sc <1>*i_interface_rf;
  receiver *i_receiver;

  void init (double fs, double fc, double fb, double ampl, double Eb){

    i_receiver->init(fs, fc, fb/2);

  }
  SC_CTOR (rx) {

    i_receiver=new receiver("i_receiver");
    i_receiver->en(en);
    i_receiver->in(bb_in);
    i_receiver->out(sig_receiver);

    i_interface_rf=new interface_sdf2sc <1>("i_interface_rf");
    i_interface_rf->in(sig_receiver);
    i_interface_rf->out[0](digital_out);

  }
};
#endif
