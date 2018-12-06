#ifndef GEN_H
#define GEN_H

#include "systemc-ams.h"

SCA_TDF_MODULE (gen)
{
  	sca_tdf::sca_out < double  > out;

  	double i;
  	double cpt;
	double dt;

  	void processing () {
		out.write(i);
  	}

 	void setI(double cycle_current)
	{
		i=cycle_current;
	}

 	void initialize()
	{
		dt=out.get_timestep().to_seconds();
	}

  	SCA_CTOR (gen) {
		i=0.96;
  	}
};
#endif
