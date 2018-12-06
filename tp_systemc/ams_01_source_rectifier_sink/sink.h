#ifndef SINK_H
#define SINK_H

#include <iostream>
#include <systemc-ams>

SCA_TDF_MODULE(sink) {

  sca_tdf::sca_in<double> in;

  SCA_CTOR(sink)
  : in("in")
  {}

  void processing() {
    using namespace std;
    // TODO: Read from "in" the current value and output it to std::cout
    //       together with the module name (use "this->name()") and
    //       current time (use "this->get_time()").
  }

};

#endif // SINK_H

/*
 * Local Variables:
 * mode: C++
 * End:
 */
