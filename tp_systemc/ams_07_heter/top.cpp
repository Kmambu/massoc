/*
 *
 * SOCLIB_LGPL_HEADER_BEGIN
 * 
 * This file is part of SoCLib, GNU LGPLv2.1.
 * 
 * SoCLib is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; version 2.1 of the License.
 * 
 * SoCLib is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with SoCLib; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 * 
 * SOCLIB_LGPL_HEADER_END
 *
 * Copyright (c) UPMC, Lip6, SoC
 */

#include <iostream>
#include <cstdlib>
#include <cstring>

#include "node.h"
#include "wavegen.h"
#include "tx.h"
#include "rx.h"
#include "channel_AWGN_n.h"

#include "gen.h"
#include "kibam.h"
#include "sink.h"

using namespace sc_core;
using soclib::common::IntTab;
using soclib::common::Segment;

#define NB_NODE 4

//small function for binding name
const char * tostring(const char * name, int val){
  std::ostringstream temp;
  temp << name << val;
  return (temp.str()).c_str();
}

void recursive_descent( sc_core::sc_object* obj, unsigned int level )
{
        if (std::strcmp(obj->kind(), "sc_module") == 0)
        {
                // Insert some specific action here
                for ( unsigned k = 0; k < level; k++ )
                        std::cout << " " ;
                //std::cout << typeid(*obj).name() << std::endl ;
		sc_module *m;
		m = dynamic_cast<sc_module*>(obj);
                std::cout << m->name() << std::endl ;
        }
        //std::cout << obj->kind() << " " << typeid(*obj).name() << std::endl ;
        std::vector<sc_core::sc_object*> children = obj->get_child_objects();
        for ( unsigned i = 0; i < children.size(); i++ )
                recursive_descent( children[i],level+1 );
}

int _main(int argc, char *argv[])
{
  // Define our VCI parameters
  typedef soclib::caba::VciParams<4,6,32,1,1,1,8,1,1,1> vci_param;

  sc_clock        s_clk("signal_clk",sc_time(10,SC_NS));
  sc_signal<bool> s_resetn("signal_resetn");

  sc_signal<bool> *s_uart_tx_emitted[NB_NODE];
  sc_signal<bool> *s_uart_rx_received[NB_NODE];
  sc_signal<bool> *s_uart_tx_en[NB_NODE];
  sc_signal<bool> *s_uart_rx_en[NB_NODE];
  sca_tdf::sca_signal < BB > *s_channel_rf_in[NB_NODE];
  sca_tdf::sca_signal < BB > *s_channel_rf_out[NB_NODE];
  sca_tdf::sca_signal < double > *s_sensor[NB_NODE];

  node<vci_param,8> *i_node[NB_NODE];
  wavegen *i_wavegen;
  channel_AWGN_n *i_channel;
  tx *i_tx[NB_NODE];
  rx *i_rx[NB_NODE];
  
  double ampl = 0.56;
  double fs = 24e9;
  double fc = 2.4e9;
  double fb = 2.4e6;
  double Eb = 1.0;
  double SNR = 100;
  
  i_channel=new channel_AWGN_n("channel");
  i_channel->init(SNR, fs, Eb);
  i_wavegen=new wavegen("wavegen");
  i_wavegen->init(EPICENTRE_X, EPICENTRE_Y, sc_core::sc_time(1/(10*2.0*50e3*32),SC_SEC));
  i_wavegen->set_sensor_pos(0,(int)SENSOR0_X,(int)SENSOR0_Y);
  i_wavegen->set_sensor_pos(1,(int)SENSOR1_X,(int)SENSOR1_Y);
  i_wavegen->set_sensor_pos(2,(int)SENSOR2_X,(int)SENSOR2_Y);
  i_wavegen->set_sensor_pos(3,(int)SENSOR3_X,(int)SENSOR3_Y);

  for (int i=0; i<NB_NODE; i++) {
    s_uart_rx_received[i] = new sc_signal <bool> (tostring("s_uart_rx_received",i));
    s_uart_tx_emitted[i]  = new sc_signal <bool> (tostring("s_uart_tx_emitted",i));
    s_uart_tx_en[i]       = new sc_signal <bool> (tostring("s_uart_tx_en",i));
    s_uart_rx_en[i]       = new sc_signal <bool> (tostring("s_uart_rx_en",i));
    s_channel_rf_in[i]  = new sca_tdf::sca_signal < BB >   (tostring("s_channel_rf_in",i)); 
    s_channel_rf_out[i] = new sca_tdf::sca_signal < BB >   (tostring("s_channel_rf_out",i));
    s_sensor[i]         = new sca_tdf::sca_signal <double> (tostring("s_sensor",i));
	  
    i_node[i] = new node<vci_param,8> (tostring("node",i),i,"soft/bin.soft");
    i_node[i]->p_clk(s_clk);
    i_node[i]->p_resetn(s_resetn);
    i_node[i]->p_tx(*s_uart_tx_emitted[i]);
    i_node[i]->p_rx(*s_uart_rx_received[i]);
    i_node[i]->p_tx_en(*s_uart_tx_en[i]);
    i_node[i]->p_rx_en(*s_uart_rx_en[i]);
    i_node[i]->p_sensor(*s_sensor[i]);

    i_tx[i] = new tx (tostring("tx",i));
    i_tx[i]->en (*s_uart_tx_en[i]);
    i_tx[i]->digital_in (*s_uart_tx_emitted[i]);
    i_tx[i]->bb_out (*s_channel_rf_out[i]);
    i_tx[i]->init(fs, fc, fb, ampl, Eb);

    i_rx[i] = new rx (tostring("rx",i));
    i_rx[i]->en (*s_uart_rx_en[i]);
    i_rx[i]->bb_in (*s_channel_rf_in[i]);
    i_rx[i]->digital_out (*s_uart_rx_received[i]);
    i_rx[i]->init(fs, fc, fb, ampl, Eb);

    i_channel->in[i](*s_channel_rf_out[i]);
    i_channel->out[i](*s_channel_rf_in[i]);

    i_wavegen->out_sensor[i](*s_sensor[i]);
  }

  	sca_tdf::sca_signal < double > current[4];
        sca_tdf::sca_signal < double > y1[4], y2[4];

        sink *i_sink;
        gen *i_gen[4];
        kibam *i_kibam[4];

        i_gen[0] = new gen("i_gen0");
        i_gen[0]->out(current[0]);

        i_gen[1] = new gen("i_gen1");
        i_gen[1]->out(current[1]);

        i_gen[2] = new gen("i_gen2");
        i_gen[2]->out(current[2]);

        i_gen[3] = new gen("i_gen3");
        i_gen[3]->out(current[3]);

        i_kibam[0] = new kibam ("i_kibam0");
        i_kibam[0]->in_i(current[0]);
        i_kibam[0]->py1(y1[0]);
        i_kibam[0]->py2(y2[0]);

        i_kibam[1] = new kibam ("i_kibam1");
        i_kibam[1]->in_i(current[1]);
        i_kibam[1]->py1(y1[1]);
        i_kibam[1]->py2(y2[1]);

        i_kibam[2] = new kibam ("i_kibam2");
        i_kibam[2]->in_i(current[2]);
        i_kibam[2]->py1(y1[2]);
        i_kibam[2]->py2(y2[2]);

        i_kibam[3] = new kibam ("i_kibam3");
        i_kibam[3]->in_i(current[3]);
        i_kibam[3]->py1(y1[3]);
        i_kibam[3]->py2(y2[3]);

        i_sink = new sink ("i_sink");
        i_sink->in0(y1[0]);
        i_sink->in1(y2[0]);
        i_sink->in2(y1[1]);
        i_sink->in3(y2[1]);
        i_sink->in4(y1[2]);
        i_sink->in5(y2[2]);
        i_sink->in6(y1[3]);
        i_sink->in7(y2[3]);

        i_gen[0]->set_timestep(sc_core::sc_time(100.0,SC_NS));
        i_gen[1]->set_timestep(sc_core::sc_time(100.0,SC_NS));
        i_gen[2]->set_timestep(sc_core::sc_time(100.0,SC_NS));
        i_gen[3]->set_timestep(sc_core::sc_time(100.0,SC_NS));

        std::vector<sc_object*> tops = sc_get_top_level_objects();
        for ( unsigned i = 0; i < tops.size(); i++ )
                recursive_descent( tops[i],0 );

//   sca_trace_file *tf1 = sca_create_tabular_trace_file ("./wavegen.dat");
//   sca_trace (tf1, i_node[3]->sensor->i_sigmadelta.in, "wavegen");

//   sca_trace_file *tf2 = sca_create_tabular_trace_file ("./decimator.dat");
//   sca_trace (tf2, i_node[3]->sensor->i_decimator.out, "decimator");

  sc_start(sc_core::sc_time(0, SC_NS));
  s_resetn = false;
  sc_start(sc_core::sc_time(1, SC_NS));
  s_resetn = true;

#ifdef SOCVIEW
  debug();
#else
  while (1)
	{
  		sc_start(sc_core::sc_time(1, SC_NS));
		for (int i=0;i<4;i++)
			if (i_node[i]->uart->r_control==2)
				i_gen[i]->setI(0.96);
			else
				i_gen[i]->setI(0.48);
	}
#endif
  return EXIT_SUCCESS;
}

int sc_main (int argc, char *argv[])
{
  try {
    return _main(argc, argv);
  } catch (std::exception &e) {
    std::cout << e.what() << std::endl;
  } catch (...) {
    std::cout << "Unknown exception occured" << std::endl;
    throw;
  }
  return 1;
}
