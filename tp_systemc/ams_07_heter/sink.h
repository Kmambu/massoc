#ifndef SINK_H
#define SINK_H

#include "systemc-ams.h"

SCA_TDF_MODULE (sink)
{
  sca_tdf::sca_in < double  > in0;
  sca_tdf::sca_in < double  > in1;
  sca_tdf::sca_in < double  > in2;
  sca_tdf::sca_in < double  > in3;
  sca_tdf::sca_in < double  > in4;
  sca_tdf::sca_in < double  > in5;
  sca_tdf::sca_in < double  > in6;
  sca_tdf::sca_in < double  > in7;

  void processing () {
	double v0=in0.read();
	double v1=in1.read();
	double v2=in2.read();
	double v3=in3.read();
	double v4=in4.read();
	double v5=in5.read();
	double v6=in6.read();
	double v7=in7.read();
/*
	std::cout << get_timestep().to_seconds() 
		<< " " << v0 
		<< " " << v1 
		<< " " << v2 
		<< " " << v3 
		<< " " << v4 
		<< " " << v5 
		<< " " << v6 
		<< " " << v7 
		<< std::endl;
*/
  }

  SCA_CTOR (sink) {
  }

};
#endif
