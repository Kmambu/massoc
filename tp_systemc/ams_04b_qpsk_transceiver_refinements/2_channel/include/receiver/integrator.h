      //***************************************************************************
      //                         File : ./integrator.h
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
#ifndef INTEGRATOR_H
#define INTEGRATOR_H
#include "baseband.h"

namespace Rx{
SCA_TDF_MODULE(integrator)
{
  sca_tdf::sca_in< BB >in;
  sca_tdf::sca_out< double >out;

  double tsym;

  void processing() {
    out.write (in.read().DC*tsym);
  }
  void set_specifications(double fb) {
    tsym=2.0/fb;
  }
  void end_of_elaboration(){
    cout << "Rx:Integrator :" << endl;
  }
  SCA_CTOR (integrator) {}
};
}
#endif
