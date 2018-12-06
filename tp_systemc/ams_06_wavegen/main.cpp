#include <iostream>
#include <cstdlib>

#include "systemc-ams.h"
#include "coords.h"
#include "wavegen.h"
#include "sink.h"

int sc_main(int argc, char *argv[])
{
  sca_tdf::sca_signal < double > s_sensor[4];

  wavegen *i_wavegen;
  sink *i_sink[4];

  i_wavegen=new wavegen("i_wavegen");
  i_wavegen->init(40, 30, sc_core::sc_time(1/(10*2.0*50e3*32),SC_SEC));
  i_wavegen->set_sensor_pos(0,(int)SENSOR0_X,(int)SENSOR0_Y);
  i_wavegen->set_sensor_pos(1,(int)SENSOR1_X,(int)SENSOR1_Y);
  i_wavegen->set_sensor_pos(2,(int)SENSOR2_X,(int)SENSOR2_Y);
  i_wavegen->set_sensor_pos(3,(int)SENSOR3_X,(int)SENSOR3_Y);

  i_wavegen->out_sensor[0](s_sensor[0]);
  i_wavegen->out_sensor[1](s_sensor[1]);
  i_wavegen->out_sensor[2](s_sensor[2]);
  i_wavegen->out_sensor[3](s_sensor[3]);

  i_sink[0]=new sink("i_sink0");
  i_sink[0]->in(s_sensor[0]);

  i_sink[1]=new sink("i_sink1");
  i_sink[1]->in(s_sensor[1]);

  i_sink[2]=new sink("i_sink2");
  i_sink[2]->in(s_sensor[2]);

  i_sink[3]=new sink("i_sink3");
  i_sink[3]->in(s_sensor[3]);

  sca_util::sca_trace_file* atf = sca_util::sca_create_vcd_trace_file( "tr.vcd" );
  sca_util::sca_trace( atf, s_sensor[0], "s_0" );
  sca_util::sca_trace( atf, s_sensor[1], "s_1" );
  sca_util::sca_trace( atf, s_sensor[2], "s_2" );
  sca_util::sca_trace( atf, s_sensor[3], "s_3" );

  sc_start(15.0,SC_MS);

  sca_util::sca_close_vcd_trace_file( atf );

  return 0;
}
