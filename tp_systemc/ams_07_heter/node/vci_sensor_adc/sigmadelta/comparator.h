      //***************************************************************************
      //                         Fichier : ./comparator.h
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
#ifndef COMPARATOR_H
#define COMPARATOR_H

SCA_TDF_MODULE (comparator)
{
  sca_tdf::sca_in < double >in;
  sca_tdf::sca_de::sca_in < bool >ck;
  sca_tdf::sca_out < double >out;

  double vref;
  bool ck_previous;
  double vout;

  void init (double vref) {
    this->vref=vref;
    this->vout=0;
  }
  void set_attributes() {
    out.set_delay(1);
  }
  void processing () {
    if ((ck.read()^ck_previous)&&(ck.read()==1)){
      if (in.read() > vref) {
        vout = 1;
      }
      else {
        vout = -1;
      }
    }
    out.write(vout);
    ck_previous=ck.read();
  }
  SCA_CTOR (comparator) {}
};
#endif
