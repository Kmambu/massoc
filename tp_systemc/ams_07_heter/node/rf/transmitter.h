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
      //                         Fichier : ./transmitter.h
      //                                                                           
      // Language   : C++     Version : 1.0    Date : Mon Jun 9 14:35:13 CEST 2008
      //
      // Author     : Michel Vasilevski
      // 
      // Laboratory : LIP6/SOC, University Pierre and Marie Curie, Paris 6
      //                                                                           
      //***************************************************************************
#ifndef TRANSMITTER_H
#define TRANSMITTER_H

#include "systemc-ams.h"

#include <limits.h>
#include "encoder.h"
#include "demux.h"
#include "lo.h"
#include "interpolate.h"
#include "modulator.h"
#include "adder.h"
#include "constant.h"
#include "gain.h"
#include "baseband.h"

SC_MODULE (transmitter)
{
  sc_in < bool > en;
  sc_in < bool > in;
  sca_tdf::sca_out < BB >out;

  sca_tdf::sca_signal < double >sig_encoder;
  sca_tdf::sca_signal < double >sig_demuxI;
  sca_tdf::sca_signal < double >sig_demuxQ;
  sca_tdf::sca_signal < BB >sig_interpolateI;
  sca_tdf::sca_signal < BB >sig_interpolateQ;
  sca_tdf::sca_signal < BB >sig_loI;
  sca_tdf::sca_signal < BB >sig_loQ;
  sca_tdf::sca_signal < BB >sig_modulatorI;
  sca_tdf::sca_signal < BB >sig_modulatorQ;

  encoder *i_encoder;
  demux *i_demux;
  lo *i_lo;
  interpolate *i_interpolateI;
  modulator *i_modulatorI;
  interpolate *i_interpolateQ;
  modulator *i_modulatorQ;
  adder *i_adder;

  void init (double Eb,double fs,double fc,double fb) {
    i_encoder->init(ideal_gain(50,50,50),50,50,&i_demux->rin,fb);
    i_demux->init(ideal_gain(50,50,50),ideal_gain(50,50,50),50,50,50,&i_modulatorI->rin,&i_modulatorQ->rin);
    i_interpolateI->init(1);
    i_lo->init(fc,0,0,0);
    i_modulatorI->init(fs,ideal_gain(50,50,50)-10*log10(1/fb),2000,0,0,50,50,&i_adder->rinI);
    i_interpolateQ->init(1);
    i_modulatorQ->init(fs,ideal_gain(50,50,50)-10*log10(1/fb),2000,0,0,50,50,&i_adder->rinQ);
    i_adder->init(ideal_gain(50,rzero,rinfinity),ideal_gain(50,rzero,rinfinity),50,50,rzero,&rinfinity);
  }
  SC_CTOR (transmitter) {

    i_encoder=new encoder("i_encoder");
    i_encoder->in(in);
    i_encoder->out(sig_encoder);

    i_demux=new demux("i_demux");
    i_demux->in(sig_encoder);
    i_demux->outI(sig_demuxI);
    i_demux->outQ(sig_demuxQ);

    i_lo=new lo("i_lo");
    i_lo->outI(sig_loI);
    i_lo->outQ(sig_loQ);

    i_interpolateI=new interpolate("i_interpolateI");
    i_interpolateI->in(sig_demuxI);
    i_interpolateI->out(sig_interpolateI);

    i_modulatorI=new modulator("i_modulatorI");
    i_modulatorI->in(sig_interpolateI);
    i_modulatorI->in_lo(sig_loI);
    i_modulatorI->out(sig_modulatorI);

    i_interpolateQ=new interpolate("i_interpolateQ");
    i_interpolateQ->in(sig_demuxQ);
    i_interpolateQ->out(sig_interpolateQ);

    i_modulatorQ=new modulator("i_modulatorQ");
    i_modulatorQ->in(sig_interpolateQ);
    i_modulatorQ->in_lo(sig_loQ);
    i_modulatorQ->out(sig_modulatorQ);

    i_adder=new adder("i_adder");
    i_adder->en(en);
    i_adder->inI(sig_modulatorI);
    i_adder->inQ(sig_modulatorQ);
    i_adder->out(out);

  }
};
#endif
