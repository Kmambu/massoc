      //***************************************************************************
      //                         File : ./transmitter.h
      //
      // Date       : Mon Jul 11 23:05:11 CEST 2011
      //
      // Language   : SystemC AMS 1.0Beta2
      //
      // Authors    : Michel Vasilevski     : michel.vasilevski@lip6.fr
      //            : Hassan Aboushady      : hassan.aboushady@lip6.fr
      //
      // Laboratory : LIP6/SOC, University Pierre and Marie Curie, Paris 6
      //
      //***************************************************************************
#ifndef TRANSMITTER_H
#define TRANSMITTER_H

#include <limits.h>
#include "transmitter/encoder.h"
#include "transmitter/demux.h"
#include "transmitter/lo.h"
#include "transmitter/interpolator.h"
#include "transmitter/mixer.h"
#include "transmitter/adder.h"
#include "transmitter/pa.h"
#include "transmitter/antenna.h"

SC_MODULE (transmitter)
{
  sca_tdf::sca_in< bool >in;
  sca_tdf::sca_out< BB >out;

  sca_tdf::sca_signal< double >sig_encoder;
  sca_tdf::sca_signal< double >sig_demuxI;
  sca_tdf::sca_signal< double >sig_demuxQ;
  sca_tdf::sca_signal< BB >sig_interpolatorI;
  sca_tdf::sca_signal< BB >sig_interpolatorQ;
  sca_tdf::sca_signal< BB >sig_loI;
  sca_tdf::sca_signal< BB >sig_loQ;
  sca_tdf::sca_signal< BB >sig_mixerI;
  sca_tdf::sca_signal< BB >sig_mixerQ;
  sca_tdf::sca_signal< BB >sig_adder;
  sca_tdf::sca_signal< BB >sig_pa;

  Tx::encoder *i_encoder;
  Tx::demux *i_demux;
  Tx::lo *i_lo;
  Tx::interpolator *i_interpolatorI;
  Tx::mixer *i_mixerI;
  Tx::interpolator *i_interpolatorQ;
  Tx::mixer *i_mixerQ;
  Tx::adder *i_adder;
  Tx::pa *i_pa;
  Tx::antenna *i_antenna;

  void initialize(double Eb,double fc,double fb){
    i_interpolatorI->set_rate(1);
    i_lo->set_carrier(fc);
    i_mixerI->set_gain(20.0*log10(sqrt(fb)));
    i_interpolatorQ->set_rate(1);
    i_mixerQ->set_gain(20.0*log10(sqrt(fb)));
    i_pa->set_gain(0.0);
  }
  SC_CTOR (transmitter) {
    i_encoder=new Tx::encoder("i_encoder");
    i_encoder->in(in);
    i_encoder->out(sig_encoder);

    i_demux=new Tx::demux("i_demux");
    i_demux->in(sig_encoder);
    i_demux->outI(sig_demuxI);
    i_demux->outQ(sig_demuxQ);

    i_lo=new Tx::lo("i_lo");
    i_lo->outI(sig_loI);
    i_lo->outQ(sig_loQ);

    i_interpolatorI=new Tx::interpolator("i_interpolatorI");
    i_interpolatorI->in(sig_demuxI);
    i_interpolatorI->out(sig_interpolatorI);

    i_mixerI=new Tx::mixer("i_mixerI");
    i_mixerI->in(sig_interpolatorI);
    i_mixerI->in_lo(sig_loI);
    i_mixerI->out(sig_mixerI);

    i_interpolatorQ=new Tx::interpolator("i_interpolatorQ");
    i_interpolatorQ->in(sig_demuxQ);
    i_interpolatorQ->out(sig_interpolatorQ);

    i_mixerQ=new Tx::mixer("i_mixerQ");
    i_mixerQ->in(sig_interpolatorQ);
    i_mixerQ->in_lo(sig_loQ);
    i_mixerQ->out(sig_mixerQ);

    i_adder=new Tx::adder("i_adder");
    i_adder->inI(sig_mixerI);
    i_adder->inQ(sig_mixerQ);
    i_adder->out(sig_adder);

    i_pa=new Tx::pa("i_pa");
    i_pa->in(sig_adder);
    i_pa->out(sig_pa);

    i_antenna=new Tx::antenna("i_antenna");
    i_antenna->in(sig_pa);
    i_antenna->out(out);
  }
};
#endif
