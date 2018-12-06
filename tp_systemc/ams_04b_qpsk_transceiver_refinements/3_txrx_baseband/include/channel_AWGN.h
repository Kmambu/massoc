      //***************************************************************************
      //                         File : ./channel_AWGN.h
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
#ifndef CHANNEL_AWGN_H
#define CHANNEL_AWGN_H

#include "common/noise.h"
#include "baseband.h"

SCA_TDF_MODULE(channel_AWGN)
{
  sca_tdf::sca_in< BB >in;
  sca_tdf::sca_out< BB >out;

  double sigma;

  void processing() {
    BB x(sigma*randn(),sigma*randn(),sigma*randn(),sigma*randn(),sigma*randn(),sigma*randn(),sigma*randn(),in.read().w);
    out.write(in.read()+x);
  }
  void initialize(double N0dB, double fs)
  {
    double N0=pow(10.0,N0dB/10.0);
    sigma=sqrt(N0*fs/2.0);
    srand (time(NULL)); //randomize
  }
  SCA_CTOR (channel_AWGN) {}
};
#endif
