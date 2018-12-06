      //***************************************************************************
      //                         File : ./antenna.h
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
#ifndef T_ANTENNA_H
#define T_ANTENNA_H
#include "baseband.h"

namespace Tx{
SCA_TDF_MODULE(antenna)
{
  sca_tdf::sca_in< BB >in;
  sca_tdf::sca_out< BB >out;

  void processing() {
    out.write(in.read());
  }
  void end_of_elaboration(){
    cout << "Tx:Antenna :" << endl;
  }
  SCA_CTOR (antenna) {}
};
}
#endif
