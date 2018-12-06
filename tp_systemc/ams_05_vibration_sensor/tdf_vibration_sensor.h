#ifndef TDF_VIBRATION_SENSOR_H_
#define TDF_VIBRATION_SENSOR_H_

#include <systemc-ams>

namespace heter {

// Vibration sensor, which output is proportional to the vibration velocity.
class tdf_vibration_sensor : public sca_tdf::sca_module {
public:

  sca_tdf::sca_in<double> x_in;    // Displacement [m]
  sca_tdf::sca_out<double> v_out;  // Sensor output voltage [V]

  // Construct TDF vibration sensor from its name and conversion
  // factor from vibration speed to output voltage [V s / m].
  tdf_vibration_sensor(sc_core::sc_module_name nm, double k)
  : x_in("x_in"), v_out("v_out"), k_(k)
  , start_up_(true), x_in_last_(0.0), x_dot_(0.0)
  {}

  void set_attributes() {
    x_in.set_rate(1);
    v_out.set_rate(1);
  }

  void processing() {
    // Calculate velocity as 1st time derivative of displacement by
    // evaluating Newton's difference quotient for the current and
    // last sample.
    if (!start_up_) {
      x_dot_ = (x_in.read() - x_in_last_) / x_in.get_timestep().to_seconds();
    } else {
      start_up_ = false;
    }
    x_in_last_ = x_in.read();
    // Convert vibration velocity to output voltage via conversion factor k.
    v_out.write(k_ * x_dot_);
  }

private:
  double k_;          // Conversion factor from vibration velocity to output voltage [V s / m].
  bool start_up_;     // Flag to mark first execution of processing().
  double x_in_last_;  // Last displacement read in previous processing() execution.
  double x_dot_;      // 1st time derivative of displacement, a.k.a. velocity.
};

} // namespace heter

#endif // TDF_VIBRATION_SENSOR_H_

/*
 * Local Variables:
 * mode: C++
 * End:
 */
