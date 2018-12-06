#ifndef PLOT_H
#define PLOT_H

#include "systemc-ams.h"
#include <stdio.h>
#include <iostream>
#include <fstream>

SCA_TDF_MODULE (plot)
{
  sca_tdf::sca_in < double >inI;
  sca_tdf::sca_in < double >inQ;

  int rate;
  double rout;
  int cpt;
  bool trace;
  const char* trace_file;

  void plot_trace(double val0, double val1) {
	/*
    ofstream file(trace_file, ios_base::out | ios_base::app);
    file << val0 << " " << val1 << std::endl;      
    file.close();
*/
  }

  void processing () {
    /*
    double in0=inI.read();
    double in1=inQ.read();
    
    std::cout.setf(ios::fixed,ios::floatfield);            // floatfield not set
    std::cout.precision(5);
    std::cout << in0 << " " << in1 << std::endl;
    */

    /*
    if (trace) {
      if (cpt>=16) {
	trace = false;
	cpt=0;
      }
      plot_trace(inI.read(),inQ.read());
      cpt++;
    }
    else if (inI.read()!=0 || inQ.read()!=0)
      trace=true;
    */
  }

  void init (const char* trace_file, int rate) {
    trace=false;
    cpt=0;
    this->trace_file=trace_file;
    this->rate = rate;
  }

  void set_attributes() {
	inI.set_rate(rate);
  }

  SCA_CTOR (plot) {}

};
#endif
