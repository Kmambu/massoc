      //***************************************************************************
      //                         Fichier : ./dac_rz.h
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
#ifndef DAC_RZ_H
#define DAC_RZ_H
SCA_TDF_MODULE (dac_rz)
{
  sca_tdf::sca_in < double >in;
  sca_tdf::sca_out < double >out;

  double gain;
  unsigned int sim_rate;
  long int counter;

  void init (double gain,unsigned int sim_rate) {
    this->gain=gain;
    this->sim_rate=sim_rate;
  }
  void processing(){
    if (counter%sim_rate < sim_rate/2) {
      out.write(0);
    }
    else {
      out.write(gain*((in.read()==1)?1:-1));
    }
    counter++;
  }
  SCA_CTOR (dac_rz) {counter = 0;}
};
#endif
