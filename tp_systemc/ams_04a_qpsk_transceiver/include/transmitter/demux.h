/////////////////////////////////////////////////////////////////////////////
// File       : demux.h
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

#ifndef DEMUX_H
#define DEMUX_H

#include <iostream>
#include <systemc-ams>

namespace Tx {

SCA_TDF_MODULE(demux)
{
  sca_tdf::sca_in<double> in;
  sca_tdf::sca_out<double> outI;
  sca_tdf::sca_out<double> outQ;
  
  void processing() {
    // TODO: Distribute the two read input samples on the
    //       I and Q channel, respectively.
    outI.write(0);
    outQ.write(0);
  }

  void set_attributes(){
    // TODO: Specify the correct sample rates on the TDF ports.
  }

  void end_of_elaboration(){
    using namespace std;
    cout << "Tx:Demux :" << endl;
  }

  SCA_CTOR (demux){}
};

}

#endif

/*
 * Local Variables:
 * mode: C++
 * End:
 */
