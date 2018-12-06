// Always encapsulate the header content by #include guards!
#ifndef SIN_SOURCE_H
#define SIN_SOURCE_H

#include <cmath>
#include <systemc-ams>

// Declare a TDF module.
SCA_TDF_MODULE(sin_source) {

  sca_tdf::sca_out<double> out;  // TDF output port.

  // Construct the TDF module.
  SCA_CTOR(sin_source)
  : out("out") // Name the ports.
  {}

  // Set TDF attributes.
  void set_attributes() {
    // TODO: Set the time step of the output port to 1 ms.
  }

  // Describe module behavior over time.
  void processing() {
    // Get current time of the sample to be written to the out port.
    double t = out.get_time().to_seconds();
    // TODO: Calculate current value of the sine wave (amp = 1.5 V, f = 50 Hz)
    double x = 0.0;
    // Write sample to the output
    out.write(x);
  }

}; // Never forget ";" at end of class definition!

#endif // SIN_SOURCE_H

/*
 * Local Variables:
 * mode: C++
 * End:
 */
