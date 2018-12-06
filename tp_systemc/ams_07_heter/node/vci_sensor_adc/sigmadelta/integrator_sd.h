      //***************************************************************************
      //                         Fichier : ./integrator_sd.h
      //                                                                           
      // Language   : SystemC-AMS  Date : Tue Sep 23 04:32:04 CEST 2008
      //
      // Authors    : Michel Vasilevski     : michel.vasilevski@lip6.fr
      //            : Francois Pecheux      : francois.pecheux@lip6.fr
      //            : Hassan Aboushady      : hassan.aboushady@lip6.fr
      // 
      // Laboratory : LIP6/SOC, University Pierre and Marie Curie, Paris 6
      //                                                                           
      //***************************************************************************
#ifndef INTEGRATOR_SD_H
#define INTEGRATOR_SD_H
//#include <systemc-ams.h>

SCA_TDF_MODULE (integrator_sd)
{
  sca_tdf::sca_in < double >in1;
  sca_tdf::sca_in < double >in2;
  sca_tdf::sca_out < double >out;

  sca_vector < double >NUM, DEN, S;
  sca_tdf::sca_ltf_nd ltf1;

  void init (double fs) {
    DEN (0) = 0.0;
    DEN (1) = 1.0;
    NUM (0) = fs/3.0;
  }
  
  void processing () {
    out.write (ltf1(NUM, DEN, S, in1.read () - in2.read ()));
  }
  SCA_CTOR (integrator_sd){}
};
#endif
