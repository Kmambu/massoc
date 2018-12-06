/////////////////////////////////////////////////////////////////////////////
// File       : ./main.cpp
//
// Date       : Mon Jul 11 23:05:16 CEST 2011
//
// Language   : SystemC AMS 1.0Beta2
//
// Authors    : Michel Vasilevski     : michel.vasilevski@lip6.fr
//            : Hassan Aboushady      : hassan.aboushady@lip6.fr
//
// Laboratory : LIP6/SOC, University Pierre and Marie Curie, Paris 6
//
/////////////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <systemc-ams>
#include "pulse_src.h"
#include "receiver.h"
#include "transmitter.h"
#include "channel_AWGN.h"


int sc_main(int argc, char *argv[])
{
  using namespace std;
  using namespace sc_core;

  sc_set_time_resolution(1, SC_FS);
  srand (time(NULL)); // initialize random generator

  double Eb = 0.0;
  double N0 = -800;
  double fb = 2.4e6;  // Hz
  double fc = 2.4e9;  // Hz
  double fs = 24.0e9; // Hz

  sc_time Tb = sc_time(1/fb, SC_SEC);

  int nb_bits = 8;

  sca_tdf::sca_signal< bool >sig_pulse_src;
  sca_tdf::sca_signal< double >sig_transmitter;
  sca_tdf::sca_signal< double >sig_channel;
  sca_tdf::sca_signal< bool >sig_receiver;

  pulse_src *i_pulse_src = new pulse_src("i_pulse_src");
  i_pulse_src->out(sig_pulse_src);
  i_pulse_src->set_parameters(sc_time(1.0/fb,SC_SEC));

  transmitter *i_transmitter = new transmitter("i_transmitter");
  i_transmitter->in(sig_pulse_src);
  i_transmitter->out(sig_transmitter);
  i_transmitter->set_parameters(Eb, fs, fc, fb);

  channel_AWGN *i_channel = new channel_AWGN("i_channel");
  i_channel->in(sig_transmitter);
  i_channel->out(sig_channel);
  i_channel->set_parameters(N0, fs);

  receiver *i_receiver = new receiver("i_receiver");
  i_receiver->in(sig_channel);
  i_receiver->out(sig_receiver);
  i_receiver->set_parameters(fs, fc, fb);

  // TRACE TRACE TRACE TRACE TRACE TRACE TRACE TRACE TRACE TRACE TRACE TRACE 
  sca_util::sca_trace_file* i_tf1=sca_util::sca_create_tabular_trace_file("./trace/transmitter1.dat");
  sca_util::sca_trace(i_tf1,sig_pulse_src,"pulse_src");
  sca_util::sca_trace(i_tf1,i_transmitter->sig_encoder,"encoder");

  sca_util::sca_trace_file* i_tf2=sca_util::sca_create_tabular_trace_file("./trace/transmitter2.dat");
  sca_util::sca_trace(i_tf2,i_transmitter->sig_demuxI,"demuxI");
  sca_util::sca_trace(i_tf2,i_transmitter->sig_demuxQ,"demuxQ");

  sca_util::sca_trace_file* i_tf3=sca_util::sca_create_tabular_trace_file("./trace/transmitter3.dat");
  sca_util::sca_trace(i_tf3,i_transmitter->sig_mixerI,"mixerI");
  sca_util::sca_trace(i_tf3,i_transmitter->sig_mixerQ,"mixerQ");
  sca_util::sca_trace(i_tf3,i_transmitter->sig_adder,"adder");
  sca_util::sca_trace(i_tf3,sig_transmitter,"transmitter");

  sca_util::sca_trace_file* i_tf4=sca_util::sca_create_tabular_trace_file("./trace/receiver1.dat");
  sca_util::sca_trace(i_tf4,sig_channel,"channel");
  sca_util::sca_trace(i_tf4,i_receiver->sig_lna,"lna");
  sca_util::sca_trace(i_tf4,i_receiver->sig_mixerI,"mixerI");
  sca_util::sca_trace(i_tf4,i_receiver->sig_mixerQ,"mixerQ");
  sca_util::sca_trace(i_tf4,i_receiver->sig_integratorI,"integratorI");
  sca_util::sca_trace(i_tf4,i_receiver->sig_integratorQ,"integratorQ");

  sca_util::sca_trace_file* i_tf5=sca_util::sca_create_tabular_trace_file("./trace/receiver2.dat");
  sca_util::sca_trace(i_tf5,i_receiver->sig_samplerI,"samplerI");
  sca_util::sca_trace(i_tf5,i_receiver->sig_samplerQ,"samplerQ");
  sca_util::sca_trace(i_tf5,i_receiver->sig_decisionI,"decisionI");
  sca_util::sca_trace(i_tf5,i_receiver->sig_decisionQ,"decisionQ");

  sca_util::sca_trace_file* i_tf6=sca_util::sca_create_tabular_trace_file("./trace/receiver3.dat");
  sca_util::sca_trace(i_tf6,sig_receiver,"receiver");
  // END TRACE END TRACE END TRACE END TRACE END TRACE END TRACE END TRACE

  // TRACE TRACE TRACE TRACE TRACE TRACE TRACE TRACE TRACE TRACE TRACE TRACE 
//  sca_util::sca_trace_file *i_tf1=sca_create_tabular_trace_file("./trace/receiver1.dat");
//  sca_util::sca_trace(i_tf1,i_receiver->sig_integratorI,"integratorI");
//  sca_util::sca_trace(i_tf1,i_receiver->sig_integratorQ,"integratorQ");
//  sca_util::sca_trace_file *i_tf2=sca_create_tabular_trace_file("./trace/receiver2.dat");
//  sca_util::sca_trace(i_tf2,i_receiver->sig_samplerI,"samplerI");
//  sca_util::sca_trace(i_tf2,i_receiver->sig_samplerQ,"samplerQ");
  // END TRACE END TRACE END TRACE END TRACE END TRACE END TRACE END TRACE

  sc_start (Tb*nb_bits);
  sca_util::sca_close_tabular_trace_file(i_tf1);
  sca_util::sca_close_tabular_trace_file(i_tf2);
  sca_util::sca_close_tabular_trace_file(i_tf3);
  sca_util::sca_close_tabular_trace_file(i_tf4);
  sca_util::sca_close_tabular_trace_file(i_tf5);
  sca_util::sca_close_tabular_trace_file(i_tf6);
  cout << endl << sc_time_stamp() << "  sc_main finished" << endl;
  return 0;
}
