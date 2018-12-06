      //***************************************************************************
      //                         File : ./interpolator.h
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
#ifndef INTERPOLATOR_H
#define INTERPOLATOR_H

namespace Tx{
SCA_TDF_MODULE(interpolator)
{
  sca_tdf::sca_in<double> in;
  sca_tdf::sca_out<double> out;

  int rate;

  void processing(){
    for (int i=0;i<rate;i++)
      out.write(in.read(),i);
  }
  void set_rate(int _rate){
    rate = _rate;
  }
  void set_attributes(){
    out.set_rate(rate);
  }
  void end_of_elaboration(){
    cout << "Tx:Interpolator :" << endl;
    cout << "\trate =" << endl;
    cout << "\t\t" << rate << endl;
  }
  SCA_CTOR(interpolator) {}
};
}
#endif
