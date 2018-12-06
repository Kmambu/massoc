      //***************************************************************************
      //                         Fichier : ./sigmadelta.h
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
#ifndef SIGMADELTA_H
#define SIGMADELTA_H

#include "sigmadelta/integrator_sd.h"
#include "sigmadelta/comparator.h"
#include "sigmadelta/dac_rz.h"

SC_MODULE (sigmadelta)
{
  sca_tdf::sca_in < double >in;
  sc_in < bool >ck;
  sca_tdf::sca_out < double >out;

  sca_tdf::sca_signal < double >sig_integrator_2;
  sca_tdf::sca_signal < double >sig_integrator_1;
  sca_tdf::sca_signal < double >sig_dac_1;
  sca_tdf::sca_signal < double >sig_dac_2;

  comparator *i_comparator_1;
  integrator_sd *i_integrator_1, *i_integrator_2;
  dac_rz *i_dac_1, *i_dac_2;

  void init (double fs, int sim_rate) {
    i_integrator_1->init(fs);
    i_integrator_2->init(fs);
    i_comparator_1->init(0);
    i_dac_1->init(2.0,sim_rate);
    i_dac_2->init(7.0/6.0,sim_rate);
  }

  SC_CTOR (sigmadelta) {

    i_integrator_1 = new integrator_sd ("i_integrator_1");
    i_integrator_1->in1 (in);
    i_integrator_1->in2 (sig_dac_1);
    i_integrator_1->out (sig_integrator_1);

    i_integrator_2 = new integrator_sd ("i_integrator_2");
    i_integrator_2->in1 (sig_integrator_1);
    i_integrator_2->in2 (sig_dac_2);
    i_integrator_2->out (sig_integrator_2);

    i_comparator_1 = new comparator ("i_comparator_1");
    i_comparator_1->in (sig_integrator_2);
    i_comparator_1->ck (ck);
    i_comparator_1->out (out);

    i_dac_1 = new dac_rz ("i_dac_1");
    i_dac_1->in (out);
    i_dac_1->out (sig_dac_1);

    i_dac_2 = new dac_rz ("i_dac_2");
    i_dac_2->in (out);
    i_dac_2->out (sig_dac_2);
  }
};
#endif
