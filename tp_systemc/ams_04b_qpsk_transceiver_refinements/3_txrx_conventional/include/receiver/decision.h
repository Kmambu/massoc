      //***************************************************************************
      //                         File : ./decision.h
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
#ifndef DECISION_H
#define DECISION_H

namespace Rx{
SCA_TDF_MODULE(decision)
{
  sca_tdf::sca_in< double >in;
  sca_tdf::sca_out< bool >out;

  void processing() {
    if(in.read()>0){
      out.write(1);
    }
    else{
      out.write(0);
    }
  }
  void end_of_elaboration(){
    cout << "Rx:Decision :" << endl;
  }
  SCA_CTOR (decision) {}
};
}
#endif
