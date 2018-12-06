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
      //                         Fichier : ./receiver.h
      //                                                                           
      // Language   : C++     Version : 1.0    Date : Mon Jun 9 14:35:16 CEST 2008
      //
      // Author     : Michel Vasilevski
      // 
      // Laboratory : LIP6/SOC, University Pierre and Marie Curie, Paris 6
      //                                                                           
      //***************************************************************************
#ifndef RECEIVER_H
#define RECEIVER_H

#include "systemc-ams.h"

#include <limits.h>
#include "lna.h"
#include "lo.h"
#include "demodulator.h"
#include "integrator.h"
#include "sampler.h"
#include "decision.h"
#include "mux.h"
#include "constant.h"
#include "gain.h"
#include "baseband.h"
#include "plot.h"

SC_MODULE (receiver)
{
  sc_in < bool> en;
  sca_tdf::sca_in < BB >in;
  sca_tdf::sca_out < sc_uint<1> >out;

  sca_tdf::sca_signal < BB >sig_lna;
  sca_tdf::sca_signal < BB >sig_loI;
  sca_tdf::sca_signal < BB >sig_loQ;
  sca_tdf::sca_signal < BB >sig_demodulatorI;
  sca_tdf::sca_signal < BB >sig_demodulatorQ;
  sca_tdf::sca_signal < double >sig_integratorI;
  sca_tdf::sca_signal < double >sig_integratorQ;
  sca_tdf::sca_signal < double >sig_samplerI;
  sca_tdf::sca_signal < double >sig_samplerQ;
  sca_tdf::sca_signal < sc_uint<1> >sig_decisionI;
  sca_tdf::sca_signal < sc_uint<1> >sig_decisionQ;

  lna *i_lna;
  lo *i_lo;
  demodulator *i_demodulatorI;
  demodulator *i_demodulatorQ;
  integrator *i_integratorI;
  integrator *i_integratorQ;
  sampler *i_samplerI;
  sampler *i_samplerQ;
  decision *i_decisionI;
  decision *i_decisionQ;
  mux *i_mux;
  plot *i_plot;

  void init (double fs, double fc, double fb) {
    i_lna->init(fs,ideal_gain(rzero,50,25),2009.6,0,rzero,50,&i_demodulatorI->rin,&i_demodulatorQ->rin);
    i_lo->init(fc,0,0,0);
    i_demodulatorI->init(fs,ideal_gain(50,50,50)-10*log10(1/fb),2009.6,0,0,50,50,&i_integratorI->rin);
    i_integratorI->init(fb,ideal_gain(50,50,50),50,50,&i_samplerI->rin);
    i_samplerI->init(1,ideal_gain(50,50,50),50,50,&i_decisionI->rin);
    i_decisionI->init(50);
    i_demodulatorQ->init(fs,ideal_gain(50,50,50)-10*log10(1/fb),2009.6,0,0,50,50,&i_integratorQ->rin);
    i_integratorQ->init(fb,ideal_gain(50,50,50),50,50,&i_samplerQ->rin);
    i_samplerQ->init(1,ideal_gain(50,50,50),50,50,&i_decisionQ->rin);
    i_decisionQ->init(50);
    i_plot->init("./trace/trace.dat",1);
  }

  SC_CTOR (receiver) {
    i_lna=new lna("lna");
    i_lna->en(en);
    i_lna->in(in);
    i_lna->out(sig_lna);

    i_lo=new lo("i_lo");
    i_lo->outI(sig_loI);
    i_lo->outQ(sig_loQ);

    i_demodulatorI=new demodulator("demodulatorI_cos");
    i_demodulatorI->in(sig_lna);
    i_demodulatorI->in_lo(sig_loI);
    i_demodulatorI->out(sig_demodulatorI);

    i_integratorI=new integrator("i_integratorI");
    i_integratorI->in(sig_demodulatorI);
    i_integratorI->out(sig_integratorI);

    i_samplerI=new sampler("samplerI");
    i_samplerI->in(sig_integratorI);
    i_samplerI->out(sig_samplerI);

    i_decisionI=new decision("i_decisionI");
    i_decisionI->in(sig_samplerI);
    i_decisionI->out(sig_decisionI);

    i_demodulatorQ=new demodulator("demodulatorQ_sin");
    i_demodulatorQ->in(sig_lna);
    i_demodulatorQ->in_lo(sig_loQ);
    i_demodulatorQ->out(sig_demodulatorQ);

    i_integratorQ=new integrator("i_integratorQ");
    i_integratorQ->in(sig_demodulatorQ);
    i_integratorQ->out(sig_integratorQ);

    i_samplerQ=new sampler("samplerQ");
    i_samplerQ->in(sig_integratorQ);
    i_samplerQ->out(sig_samplerQ);

    i_decisionQ=new decision("i_decisionQ");
    i_decisionQ->in(sig_samplerQ);
    i_decisionQ->out(sig_decisionQ);

    i_mux=new mux("i_mux");
    i_mux->inI(sig_decisionI);
    i_mux->inQ(sig_decisionQ);
    i_mux->out(out);

    i_plot=new plot("i_plot");
    i_plot->inI(sig_samplerI);
    i_plot->inQ(sig_samplerQ);

  }
};
#endif
