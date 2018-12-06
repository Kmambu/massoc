      //***************************************************************************
      //                         File : ./lo.h
      //
      // Date       : Mon Jul 11 23:05:11 CEST 2011
      //
      // Language   : SystemC AMS 1.0Beta2
      //
      //
      // Laboratory : LIP6/SOC, University Pierre and Marie Curie, Paris 6
      //
      //***************************************************************************
#ifndef R_LO_H
#define R_LO_H
#include "baseband.h"

namespace Rx{
SCA_TDF_MODULE(lo)
{
  sca_tdf::sca_out< BB>outI;
  sca_tdf::sca_out< BB >outQ;

  double phase_mismatch;
  double pulsation_offset;
  double gain_mismatch;
  double wc;

  void processing() {
    //TODO Modifier le comportement ideal de l'oscillateur en introduisant des non-idealités:
    // "gain mismatch", "frequency offset", "phase mismatch".
    outI.write (BB(
        0,
        1,
        0,
        0,
        0,
        0,
        0,
        wc
        ));
    outQ.write (BB(
        0,
        0,
        0,
        0,
        1,
        0,
        0,
        wc
        ));
  }
  void set_carrier (double fc) {
    wc=2.0*M_PI*fc;
  }
  void set_phase_mismatch(double ph_m){
    phase_mismatch=ph_m;
  }
  void set_frequency_offset(double f_o){
    pulsation_offset=2.0*M_PI*f_o;
  }
  void set_gain_mismatch(double g_m){
    gain_mismatch=g_m;
  }
  void end_of_elaboration(){
    cout << "Rx:Lo :" << endl; 
    cout << "\tfrequency =" << endl;
    cout << "\t\t" << wc/(2.0*M_PI) << " Hz" << endl;
    cout << "\t\t" << wc << " rad/s" << endl;
    cout << "\tphase mismatch =" << endl;
    cout << "\t\t" << phase_mismatch << " rad" << endl;
    cout << "\tfrequency offset =" << endl;
    cout << "\t\t" << pulsation_offset/(2.0*M_PI) << " Hz" << endl;
    cout << "\t\t" << pulsation_offset << " rad/s" << endl;
    cout << "\tgain mismatch =" << endl;
    cout << "\t\t" << gain_mismatch << "" << endl;
  }
  SCA_CTOR (lo) {
    pulsation_offset=0;
    phase_mismatch=0;
    gain_mismatch=0;
  }
};
}
#endif
