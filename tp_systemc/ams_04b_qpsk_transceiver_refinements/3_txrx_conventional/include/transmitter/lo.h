      //***************************************************************************
      //                         File : ./lo.h
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
#ifndef T_LO_H
#define T_LO_H

namespace Tx{
SCA_TDF_MODULE(lo)
{
  sca_tdf::sca_out< double >outI;
  sca_tdf::sca_out< double >outQ;

  double wc;

  void processing(){
    outI.write(cos(wc*get_time().to_seconds()));
    outQ.write(sin(wc*get_time().to_seconds()));
  }
  void set_carrier(double fc){
    wc=2.0*M_PI*fc;
  }
  void end_of_elaboration(){
    cout << "Tx:Lo :" << endl; 
    cout << "\tfrequency =" << endl;
    cout << "\t\t" << wc/(2.0*M_PI) << " Hz" << endl;
    cout << "\t\t" << wc << " rad/s" << endl;
  }
  SCA_CTOR (lo){}
};
}
#endif
