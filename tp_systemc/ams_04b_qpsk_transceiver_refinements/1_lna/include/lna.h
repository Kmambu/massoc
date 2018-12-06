#ifndef LNA_H
#define LNA_H

#include "noise.h"

SCA_TDF_MODULE (lna)
{
  sca_tdf::sca_in < double >in;
  sca_tdf::sca_out < double >out;

  double a1;
  double a3;
  
  double sigma;
  double nf;

  void processing () {
    double input = in.read()+sigma*randn();
    out.write (a1*input+a3*pow(input,3));
  }
  void post_processing(){
    printf("lna :\n\ta1=%e\n\ta3=%e\n",a1,a3);
  }
  void init(double gain_db, double iip3, double _nf){
    nf = _nf;
    //TODO Modifier l'initialisation de "a1" pour qu'il soit determiné a partir de "gain_db".
    // Et modifier l'initialisation de "a3" pour qu'il soit déterminé a partir de a1 et iip3.
    // gain_db est une puissance en dB, iip3 en dBm, a1 et a3 en volt.
    a1 = 1;
    a3 = 0;
  }
//  void end_of_elaboration(){
  void initialize(){
    double fsimu = 1/out.get_timestep().to_seconds();
    srand(time(NULL)); //randomize
    //TODO Modifier l'initialisation de "sigma" (l'ecart type) pour qu'il soit déterminé
    // a partir de "nf" et "fs".
  }
  SCA_CTOR (lna) {}
};
#endif
