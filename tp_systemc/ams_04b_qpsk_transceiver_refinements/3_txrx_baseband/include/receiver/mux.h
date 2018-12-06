      //***************************************************************************
      //                         File : ./mux.h
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
#ifndef MUX_H
#define MUX_H

namespace Rx{
SCA_TDF_MODULE(mux)
{
  sca_tdf::sca_in< bool >inI;
  sca_tdf::sca_in< bool >inQ;
  sca_tdf::sca_out< bool >out;

  void processing() {
    out.write(inI.read(),0);
    out.write(inQ.read(),1);
  }
  void set_attributes(){
    out.set_rate(2);
  }
  void end_of_elaboration(){
    cout << "Rx:Mux :" << endl;
  }
  SCA_CTOR (mux){}
};
}
#endif
