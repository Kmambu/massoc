/////////////////////////////////////////////////////////////////////////////
// File       : pulse_src.h
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
#ifndef PULSE_SRC_H
#define PULSE_SRC_H

#include <cstdlib>
#include <iostream>
#include <systemc-ams>

SCA_TDF_MODULE(pulse_src)
{
  sca_tdf::sca_out<bool> out;

  bool val[8];
  int i;
  sca_core::sca_time Ts;
  
  void processing(){
    out.write(val[i%8]);
    // out.write(rand()%2); // write random bit (0 or 1)
    i++;
  }

  void set_parameters(sca_core::sca_time _Ts) {
    Ts=_Ts;
    val[0]=1;
    val[1]=1;
    val[2]=1;
    val[3]=0;
    val[4]=0;
    val[5]=1;
    val[6]=0;
    val[7]=0;
  }

  void set_attributes() {
    out.set_timestep(Ts);
  }

//  void end_of_elaboration() {
  void initialize() {
    using namespace std;
    cout << "Pulse source :" << endl;
    cout << "\ttimestep =" << endl;
    cout << "\t\t" << out.get_timestep().to_seconds() << " s" << endl;
  }

  SCA_CTOR (pulse_src){}
};
#endif

/*
 * Local Variables:
 * mode: C++
 * End:
 */
