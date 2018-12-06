#ifndef SIN_SOURCE_H
#define SIN_SOURCE_H

#include <cmath>
#include <systemc-ams>

// Simple sinusoidal source module.
class sin_source : public sca_tdf::sca_module {
public:
  sca_tdf::sca_out<double> out;     // Output.

  // Construct sinusoidal source and name its ports.
  sin_source(sc_core::sc_module_name nm,
             double off = 0.0, double amp  = 1.0,
             double f = 50.0, double phi = 0.0)
  : out("out"), off_(off), amp_(amp), f_(f), phi_(phi)
  {}

protected:
  // Generate sine wave and write it to the out port.
  void processing() {
    using std::sin;
    double val = 0.0;
    double t = out.get_time().to_seconds();
    val = off_ + amp_ * sin(2.0 * M_PI * f_ * t + (M_PI / 180.0) * phi_);
    out.write(val);
  }
private:
  double off_, amp_, f_, phi_; // Offset, amplitude, frequency / Hz, phase / DEG.
}; // class sin_source

#endif // SIN_SOURCE_H

/*
 * Local Variables:
 * mode: C++
 * End:
 */
