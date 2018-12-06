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

SCA_TDF_MODULE(channel_AWGN)
{
  sca_tdf::sca_in< double >in;
  sca_tdf::sca_out< double >out;

  double sigma;

  void processing() {
    out.write(in.read()+sigma*randn());
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
