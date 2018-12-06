#ifndef RECTIFIER_H
#define RECTIFIER_H

#include <cmath>
#include <systemc-ams>

SCA_TDF_MODULE(rectifier) {
  // TODO: Declare the input and output ports.

  SCA_CTOR(rectifier)
  // TODO: Name the ports in the constructor's initializer list.
  {}

  // processing() will be implemented in rectifier.cpp
  void processing();

};

#endif // RECTIFIER_H

/*
 * Local Variables:
 * mode: C++
 * End:
 */
