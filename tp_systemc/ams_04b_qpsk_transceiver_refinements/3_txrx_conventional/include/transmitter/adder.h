      //***************************************************************************
      //                         File : ./adder.h
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
#ifndef ADDER_H
#define ADDER_H

namespace Tx{
SCA_TDF_MODULE(adder)
{
  sca_tdf::sca_in< double >inI;
  sca_tdf::sca_in< double >inQ;
  sca_tdf::sca_out< double >out;

  void processing() {
    out.write (inI.read()+inQ.read());
  }
  void end_of_elaboration(){
    cout << "Tx:Adder :" << endl;
  }
  SCA_CTOR (adder) {}
};
}
#endif
