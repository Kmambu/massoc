      //***************************************************************************
      //                         File : ./noise.h
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
#ifndef NOISE_H
#define NOISE_H

#define K 1.3806504e-23
#define T 293
double randu()
{
  return (double)rand()/RAND_MAX;// RAND_MAX = 32767
}
double randn()
{
  double res=randu();
  if(res)
    return sqrt(-2*log(res))*cos(2*M_PI*randu()) ;
  return randn();
}
#endif
