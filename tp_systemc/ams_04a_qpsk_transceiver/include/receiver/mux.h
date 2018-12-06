/////////////////////////////////////////////////////////////////////////////
// File       : mux.h
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

#ifndef MUX_H
#define MUX_H

#include <iostream>
#include <systemc-ams>

namespace Rx{

SCA_TDF_MODULE(mux)
{
  sca_tdf::sca_in<bool> inI;
  sca_tdf::sca_in<bool> inQ;
  sca_tdf::sca_out<bool> out;

  void processing() {
    // TODO: Analog to the demux module, this module has to read one
    //       sample from inI and inQ, and write them one after another
    //       to the output.
    out.write(0);
  }

  void set_attributes(){
    // TODO: Set the correct TDF rates.
  }

  void end_of_elaboration(){
    using namespace std;
    cout << "Rx:Mux :" << endl;
  }

  SCA_CTOR (mux){}
};

}

#endif

/*
 * Local Variables:
 * mode: C++
 * End:
 */
