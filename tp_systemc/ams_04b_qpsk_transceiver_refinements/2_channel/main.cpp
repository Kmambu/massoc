//***************************************************************************
//                         File : ./main.cpp
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
//***************************************************************************
#include <iostream>
#include <fstream>
#include "systemc-ams.h"
#include "pulse_src.h"
#include "receiver.h"
#include "transmitter.h"
#include "channel_AWGN.h"
#include "analyser/analyser_ber.h"
#include "baseband.h"

int sc_main(int argc, char *argv[])
{
  sc_set_time_resolution(1,SC_FS);

  double Eb = 0.0;
  double N0 = -8;
  double fb=2.4e6;
  double fc=2.4e9;

  // sc_time Tb = sc_time(1/fb,SC_SEC);

  int nb_bits = (int)1e5;

  sca_tdf::sca_signal< bool >sig_pulse_src;
  sca_tdf::sca_signal< BB >sig_transmitter;
  sca_tdf::sca_signal< BB >sig_channel;
  sca_tdf::sca_signal< bool >sig_receiver;
  sca_tdf::sca_signal< bool >sig_analyser_ber;

  pulse_src *i_pulse_src=new pulse_src("i_pulse_src");
  i_pulse_src->out(sig_pulse_src);
  i_pulse_src->initialize(sc_time(1.0/fb,SC_SEC));

  transmitter *i_transmitter=new transmitter("i_transmitter");
  i_transmitter->in(sig_pulse_src);
  i_transmitter->out(sig_transmitter);
  i_transmitter->initialize(Eb,fc, fb);

  channel_AWGN *i_channel=new channel_AWGN("i_channel");
  i_channel->in(sig_transmitter);
  i_channel->out(sig_channel);
  i_channel->initialize(N0, fb);

  receiver *i_receiver=new receiver("i_receiver");
  i_receiver->in(sig_channel);
  i_receiver->out(sig_receiver);
  i_receiver->initialize(fc, fb);

 analyser_ber *i_analyser_ber=new analyser_ber("i_analyser_ber");
  i_analyser_ber->in1(sig_receiver);
  i_analyser_ber->in2(sig_pulse_src);
  i_analyser_ber->out(sig_analyser_ber);
  i_analyser_ber->init(nb_bits);

  // TRACE TRACE TRACE TRACE TRACE TRACE TRACE TRACE TRACE TRACE TRACE TRACE 

  sca_util::sca_trace_file* i_tf=sca_create_tabular_trace_file("./trace/receiver2.dat");
  sca_util::sca_trace(i_tf,i_receiver->sig_samplerI,"samplerI");
  sca_util::sca_trace(i_tf,i_receiver->sig_samplerQ,"samplerQ");

  // END TRACE END TRACE END TRACE END TRACE END TRACE END TRACE END TRACE


  //TODO Modifier l'appel a la simulation lorsqu'il vous est demandé d'afficher le BER.
  sc_start (nb_bits/fb,SC_SEC);

  sca_util::sca_close_tabular_trace_file(i_tf);
  cout << endl << sc_time_stamp() << "  sc_main finished" << endl;
  return 0;
}
