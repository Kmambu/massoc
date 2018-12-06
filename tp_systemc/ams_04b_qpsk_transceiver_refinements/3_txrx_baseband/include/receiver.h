      //***************************************************************************
      //                         File : ./receiver.h
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
#ifndef RECEIVER_H
#define RECEIVER_H

#include <limits.h>
#include "receiver/antenna.h"
#include "receiver/lna.h"
#include "receiver/lo.h"
#include "receiver/mixer.h"
#include "receiver/integrator.h"
#include "receiver/sampler.h"
#include "receiver/decision.h"
#include "receiver/mux.h"

SC_MODULE (receiver)
{
  sca_tdf::sca_in< BB >in;
  sca_tdf::sca_out< bool >out;

  sca_tdf::sca_signal< BB >sig_antenna;
  sca_tdf::sca_signal< BB >sig_lna;
  sca_tdf::sca_signal< BB >sig_loI;
  sca_tdf::sca_signal< BB >sig_loQ;
  sca_tdf::sca_signal< BB >sig_mixerI;
  sca_tdf::sca_signal< BB >sig_mixerQ;
  sca_tdf::sca_signal< double >sig_integratorI;
  sca_tdf::sca_signal< double >sig_integratorQ;
  sca_tdf::sca_signal< double >sig_samplerI;
  sca_tdf::sca_signal< double >sig_samplerQ;
  sca_tdf::sca_signal< bool >sig_decisionI;
  sca_tdf::sca_signal< bool >sig_decisionQ;

  Rx::antenna *i_antenna;
  Rx::lna *i_lna;
  Rx::lo *i_lo;
  Rx::mixer *i_mixerI;
  Rx::mixer *i_mixerQ;
  Rx::integrator *i_integratorI;
  Rx::integrator *i_integratorQ;
  Rx::sampler *i_samplerI;
  Rx::sampler *i_samplerQ;
  Rx::decision *i_decisionI;
  Rx::decision *i_decisionQ;
  Rx::mux *i_mux;

  void initialize(double fc, double fb) {
    i_lna->set_gain(0);
    i_lo->set_carrier(fc);
    i_mixerI->set_gain(20*log10(sqrt(fb)));
    i_integratorI->set_specifications(fb);
    i_samplerI->set_rate(1);
    i_mixerQ->set_gain(20*log10(sqrt(fb)));
    i_integratorQ->set_specifications(fb);
    i_samplerQ->set_rate(1);
  }

  SC_CTOR (receiver) {
    i_antenna=new Rx::antenna("i_antenna");
    i_antenna->in(in);
    i_antenna->out(sig_antenna);

    i_lna=new Rx::lna("lna");
    i_lna->in(sig_antenna);
    i_lna->out(sig_lna);

    i_lo=new Rx::lo("i_lo");
    i_lo->outI(sig_loI);
    i_lo->outQ(sig_loQ);

    i_mixerI=new Rx::mixer("mixerI");
    i_mixerI->in(sig_lna);
    i_mixerI->in_lo(sig_loI);
    i_mixerI->out(sig_mixerI);

    i_mixerQ=new Rx::mixer("mixerQ");
    i_mixerQ->in(sig_lna);
    i_mixerQ->in_lo(sig_loQ);
    i_mixerQ->out(sig_mixerQ);

    i_integratorI=new Rx::integrator("i_integratorI");
    i_integratorI->in(sig_mixerI);
    i_integratorI->out(sig_integratorI);

    i_integratorQ=new Rx::integrator("i_integratorQ");
    i_integratorQ->in(sig_mixerQ);
    i_integratorQ->out(sig_integratorQ);

    i_samplerI=new Rx::sampler("samplerI");
    i_samplerI->in(sig_integratorI);
    i_samplerI->out(sig_samplerI);

    i_samplerQ=new Rx::sampler("samplerQ");
    i_samplerQ->in(sig_integratorQ);
    i_samplerQ->out(sig_samplerQ);

    i_decisionI=new Rx::decision("i_decisionI");
    i_decisionI->in(sig_samplerI);
    i_decisionI->out(sig_decisionI);

    i_decisionQ=new Rx::decision("i_decisionQ");
    i_decisionQ->in(sig_samplerQ);
    i_decisionQ->out(sig_decisionQ);

    i_mux=new Rx::mux("i_mux");
    i_mux->inI(sig_decisionI);
    i_mux->inQ(sig_decisionQ);
    i_mux->out(out);
  }
};
#endif
