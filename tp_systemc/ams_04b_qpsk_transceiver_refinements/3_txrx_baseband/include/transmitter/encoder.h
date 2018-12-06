      //***************************************************************************
      //                         File : ./encoder.h
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
#ifndef ENCODER_H
#define ENCODER_H

namespace Tx{
SCA_TDF_MODULE(encoder)
{
  sca_tdf::sca_in<bool >in;
  sca_tdf::sca_out<double >out;

  void processing() {
    out.write((in.read()*2)-1);
  }
  void end_of_elaboration(){
    cout << "Tx:Encoder :" << endl;
  }
  SCA_CTOR (encoder) {}
};
}
#endif
