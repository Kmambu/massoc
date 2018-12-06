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
    cout << this->name() << " @ " << this->get_time() << ": "
         << in.read() << endl;
  }

};

#endif // SINK_H

/*
 * Local Variables:
 * mode: C++
 * End:
 */
