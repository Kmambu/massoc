#ifndef SINK_H
#define SINK_H

#include "systemc-ams.h"

SCA_TDF_MODULE (sink)
{
  sca_tdf::sca_in < double  > in;

  void processing () {
	double v=in.read();
	// std::cout << in.get_time().to_seconds() << " " << v << std::endl;
  }

  SCA_CTOR (sink) {
  }

};

#endif
