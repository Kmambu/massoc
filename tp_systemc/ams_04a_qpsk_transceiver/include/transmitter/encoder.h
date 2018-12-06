/////////////////////////////////////////////////////////////////////////////
// File       : encoder.h
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
/////////////////////////////////////////////////////////////////////////////

#ifndef ENCODER_H
#define ENCODER_H

#include <iostream>
#include <systemc-ams>

namespace Tx {

SCA_TDF_MODULE(encoder)
{
  sca_tdf::sca_in<bool> in;
  sca_tdf::sca_out<double> out;

  void processing() {
    // TODO: Modify the behavior so that the module transforms logic values 0 and 1 to the corresponding analog -1 and 1 for the modulation.
    out.write(0);
  }

  void end_of_elaboration(){
    using namespace std;
    cout << "Tx:Encoder :" << endl;
  }

  SCA_CTOR (encoder) {}
};

}

#endif

/*
 * Local Variables:
 * mode: C++
 * End:
 */
