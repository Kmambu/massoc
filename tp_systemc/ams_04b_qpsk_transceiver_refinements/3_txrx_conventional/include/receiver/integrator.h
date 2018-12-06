      //***************************************************************************
      //                         File : ./integrator.h
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
#ifndef INTEGRATOR_H
#define INTEGRATOR_H

namespace Rx{
SCA_TDF_MODULE(integrator)
{
  sca_tdf::sca_in< double >in;
  sca_tdf::sca_out< double >out;

  int symbole_rate;

  sca_vector<double> DEN, NUM,S;
  sca_tdf::sca_ltf_nd ltf1;

  void processing() {
    if (!(int(round(in.get_time().to_seconds()/in.get_timestep().to_seconds()))%symbole_rate)){
      S(0)=0;
    }
    out.write(ltf1(NUM,DEN,S,in.read()));
  }
  void set_specifications(int _symbole_rate) {
    DEN(0)=0.0;
    DEN(1)=1.0;
    NUM(0)=1.0;
    symbole_rate = _symbole_rate;
  }
  void end_of_elaboration(){
    cout << "Rx:Integrator :" << endl;
  }
  SCA_CTOR (integrator) {}
};
}
#endif
