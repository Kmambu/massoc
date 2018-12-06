#include <iostream>
#include <fstream>
#include "systemc-ams.h"
#include "lna.h"
#include "cos_src.h"
#include "adder.h"
#include "analyser/analyser_iip3.h"
int sc_main(int argc, char *argv[])
{
  sc_set_time_resolution(1,SC_FS);
  sca_tdf::sca_signal < double >sig_cos_src_1;
  sca_tdf::sca_signal < double >sig_cos_src_2;
  sca_tdf::sca_signal < double >sig_adder;
  sca_tdf::sca_signal < double >sig_lna;

  cos_src *i_cos_src_1;
  cos_src *i_cos_src_2;
  adder *i_adder;
  lna *i_lna;
  analyser_iip3 *analyser_i;

  int N =1000000;

  double fc=2.4e9;
  double f1=fc+2.4e6;
  double f2=fc+4.8e6;
  double cos_ampl=-20;            //dB Volts
  double lna_gain=10;
  double lna_iip3=10;
  double lna_nf=30;

  double fs=24e9;

  i_cos_src_1=new cos_src("cos_src_1");
  i_cos_src_1->out(sig_cos_src_1);
  i_cos_src_1->init(cos_ampl, f1);

  i_cos_src_2=new cos_src("cos_src_2");
  i_cos_src_2->out(sig_cos_src_2);
  i_cos_src_2->init(cos_ampl, f2);

  i_adder=new adder("adder");
  i_adder->inI(sig_cos_src_1);
  i_adder->inQ(sig_cos_src_2);
  i_adder->out(sig_adder);

  i_lna=new lna("lna");
  i_lna->in(sig_adder);
  i_lna->out(sig_lna);
  i_lna->init(lna_gain,lna_iip3,lna_nf);

  analyser_i=new analyser_iip3("analyser_iip3");
  analyser_i->in(sig_lna);
  analyser_i->init(fs,
      N,
      f1,
      f2,
      cos_ampl,
      &i_lna->a1,
      &i_lna->a3);

  sc_start (N*sc_time(1/24e9,SC_SEC));
//  cout << endl << sc_time_stamp() << " : " << int(i_cos_src_1->out.get_time().to_seconds()/i_cos_src_1->out.get_timestep().to_seconds())+1 << "  sc_main finished" << endl;
//move to /includ/cos_src.h
  sc_stop();
  return 0;
}
