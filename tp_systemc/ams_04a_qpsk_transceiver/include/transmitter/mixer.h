/////////////////////////////////////////////////////////////////////////////
// File       : ./mixer.h
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

#ifndef T_MIXER_H
#define T_MIXER_H

#include <iostream>
#include <systemc-ams>

namespace Tx{

SCA_TDF_MODULE(mixer)
{
  sca_tdf::sca_in<double> in;
  sca_tdf::sca_in<double> in_lo;
  sca_tdf::sca_out<double> out;

  double a1;

  void processing() {
    // TODO: Modify the behavior so that it output the product of the
    //       inputs and the gain a1.
    out.write(0);
  }

  void set_gain(double _vdB){
    a1=pow(10,_vdB/20.0);
  }

  void end_of_elaboration(){
    using namespace std;
    cout << "Tx:Mixer :" << endl; 
    cout << "\tgain =" << endl;
    cout << "\t\t" << 20.0*log10(a1) << " dB" << endl;
    cout << "\t\t" << a1 << "" << endl;
  }

  SCA_CTOR (mixer) {}
};

}

#endif

/*
 * Local Variables:
 * mode: C++
 * End:
 */
