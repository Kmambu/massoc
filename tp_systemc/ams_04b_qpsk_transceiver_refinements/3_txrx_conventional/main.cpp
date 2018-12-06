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

int sc_main(int argc, char *argv[])
{
  sc_set_time_resolution(1,SC_FS);

  double Eb = 0.0;
  double N0 = -800;
  double fb=2.4e6;
  double fc=2.4e9;
  double fs=24e9;

  sc_time Tb = sc_time(1/fb,SC_SEC);

  int nb_bits = 8;

  sca_tdf::sca_signal< bool >sig_pulse_src;
  sca_tdf::sca_signal< double >sig_transmitter;
  sca_tdf::sca_signal< double >sig_channel;
  sca_tdf::sca_signal< bool >sig_receiver;

  pulse_src *i_pulse_src=new pulse_src("i_pulse_src");
  i_pulse_src->out(sig_pulse_src);
  i_pulse_src->initialize(sc_time(1.0/fb,SC_SEC));

  transmitter *i_transmitter=new transmitter("i_transmitter");
  i_transmitter->in(sig_pulse_src);
  i_transmitter->out(sig_transmitter);
  i_transmitter->initialize(Eb, fs, fc, fb);

  channel_AWGN *i_channel=new channel_AWGN("i_channel");
  i_channel->in(sig_transmitter);
  i_channel->out(sig_channel);
  i_channel->initialize(N0, fs);

  receiver *i_receiver=new receiver("i_receiver");
  i_receiver->in(sig_channel);
  i_receiver->out(sig_receiver);
  i_receiver->initialize(fs, fc, fb);

  // TRACE TRACE TRACE TRACE TRACE TRACE TRACE TRACE TRACE TRACE TRACE TRACE 

  sca_util::sca_trace_file* i_tf=sca_create_tabular_trace_file("./trace/receiver2.dat");
  sca_util::sca_trace(i_tf,i_receiver->sig_samplerI,"samplerI");
  sca_util::sca_trace(i_tf,i_receiver->sig_samplerQ,"samplerQ");

  // END TRACE END TRACE END TRACE END TRACE END TRACE END TRACE END TRACE

  // "sc_start(temps)" commande la simulation pendant un certain "temps".
  // Il est possible de commander plusieurs simulations successives.
  // Dans ce cas toutes les "traces" seront fournit dans le meme fichier.
  // C'est particulierement pratique si l'ont veut analyser l'effet d'un default sur la tranmission.
  
  //TODO Modifier la commande de simulation a l'aide d'une boucle "for" (par exemple) 
  // pour permettre une analyse de :
  // "gain mismatch", "frequency offset" puis de "phase mismatch".
  // Il n'est pas necessaire de commander les trois analyses a la suite car l'affichage du fichier de
  // trace superposerait les trois resultats.
  sc_start (nb_bits*Tb);

  sca_util::sca_close_tabular_trace_file(i_tf);
  cout << endl << sc_time_stamp() << "  sc_main finished" << endl;
  return 0;
}
