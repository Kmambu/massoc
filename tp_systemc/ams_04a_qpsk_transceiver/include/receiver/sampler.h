/////////////////////////////////////////////////////////////////////////////
// File       : sampler.h
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

#ifndef SAMPLER_H
#define SAMPLER_H

#include <iostream>
#include <systemc-ams>

namespace Rx{

SCA_TDF_MODULE(sampler)
{
  sca_tdf::sca_in< double >in;
  sca_tdf::sca_out< double >out;

  int rate;

  void processing() {
    out.write(in.read(rate-1));
  }

  void set_rate (int rate_) {
    rate = rate_;
  }

  void set_attributes(){
    in.set_rate(rate);
  }

  void end_of_elaboration() {
    using namespace std;
    cout << "Rx:Sampler :" << endl;
    cout << "\trate =" << endl;
    cout << "\t\t" << rate << endl;
  }

  SCA_CTOR (sampler) {}
};

}

#endif

/*
 * Local Variables:
 * mode: C++
 * End:
 */
