/////////////////////////////////////////////////////////////////////////////
// File       : lo.h
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

#ifndef T_LO_H
#define T_LO_H

#include <cmath>
#include <iostream>
#include <systemc-ams>

namespace Tx{

SCA_TDF_MODULE(lo)
{
  sca_tdf::sca_out<double> outI;
  sca_tdf::sca_out<double> outQ;

  double wc;

  void processing(){
    // TODO: Generate the I and Q carrier signals (cosinus and sinus)
    //       with an angular velocity of wc.
    outI.write(0);
    outQ.write(0);
  }

  void set_carrier(double fc){
    wc=2.0*M_PI*fc;
  }

  void end_of_elaboration(){
    using namespace std;
    cout << "Tx:Lo :" << endl; 
    cout << "\tfrequency =" << endl;
    cout << "\t\t" << wc/(2.0*M_PI) << " Hz" << endl;
    cout << "\t\t" << wc << " rad/s" << endl;
  }

  SCA_CTOR (lo){}
};

}

#endif

/*
 * Local Variables:
 * mode: C++
 * End:
 */
