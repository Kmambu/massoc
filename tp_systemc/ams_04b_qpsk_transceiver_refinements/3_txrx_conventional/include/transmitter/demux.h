      //***************************************************************************
      //                         File : ./demux.h
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
#ifndef DEMUX_H
#define DEMUX_H

namespace Tx{
SCA_TDF_MODULE(demux)
{
  sca_tdf::sca_in< double >in;
  sca_tdf::sca_out< double >outI;
  sca_tdf::sca_out< double >outQ;
  
  void processing() {
    outI.write(in.read(0));
    outQ.write(in.read(1));
  }
  void set_attributes(){
    in.set_rate(2);
  }
  void end_of_elaboration(){
    cout << "Tx:Demux :" << endl;
  }
  SCA_CTOR (demux){}
};
}
#endif
