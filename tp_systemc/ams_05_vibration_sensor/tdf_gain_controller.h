#ifndef TDF_GAIN_CONTROLLER_H_
#define TDF_GAIN_CONTROLLER_H_

#include <systemc-ams>

namespace heter {

// TDF gain controller
template<int NBits>
class tdf_gain_controller : public sca_tdf::sca_module {
public:
  sca_tdf::sca_in<sc_dt::sc_int<NBits> > amp_in;
  sca_tdf::sca_out<int> k_out;

  explicit tdf_gain_controller(sc_core::sc_module_name nm,
                               int low_threshold = 0.2 * ((1 << (NBits - 1)) - 1),
                               int high_threshold = 0.8 * ((1 << (NBits - 1)) - 1),
                               int k_0 = 0, int k_min = 0, int k_max = 16)
  : amp_in("amp_in"), k_out("k_out"),
    low_threshold_(low_threshold), high_threshold_(high_threshold),
    k_min_(k_min), k_max_(k_max), state_(keep_gain), k_(k_0)
  {
    sc_assert(low_threshold > 0);
    sc_assert(low_threshold < high_threshold);
    sc_assert(high_threshold < ((1 << (NBits - 1)) - 1));
    sc_assert(k_min_ < k_max_);
    sc_assert(k_min_ <= k_0 && k_0 <= k_max_);
  }

  void set_attributes() {
    amp_in.set_rate(1);
    k_out.set_rate(1);
  }

  void processing() {
    switch (state_) {
    case keep_gain:
      if (amp_in.read() < low_threshold_) {
        state_ = increase_gain;
        ++k_;
      } else if (amp_in.read() >= high_threshold_) {
        state_ = decrease_gain;
        --k_;
      }
      break;
    case increase_gain:
      if (amp_in.read() < high_threshold_) {
        ++k_;
      } else {
        state_ = decrease_gain;
        --k_;
      }
      break;
    case decrease_gain:
      if (amp_in.read() < high_threshold_) {
        state_ = keep_gain;
      } else {
        --k_;
      }
      break;
    default:
      SC_REPORT_ERROR("/vibration_sensor/tdf_gain_controller", "Unexpected state.");
    }
    // Limit and set new gain.
    if (k_ < k_min_) { k_ = k_min_; }
    if (k_ > k_max_) { k_ = k_max_; }
    k_out.write(k_);
  }

private:
  const int low_threshold_;     // Low threshold for amplitude to increase gain.
  const int high_threshold_;    // High threshold for amplitude to lower gain.
  const double k_min_;          // Minimum gain power.
  const double k_max_;          // Maximum gain power.

  enum state_type {keep_gain, increase_gain, decrease_gain};
  state_type state_;            // Current state.
  int k_;                       // Current gain power.
}; // class tdf_gain_controller

} // namespace heter

#endif // TDF_GAIN_CONTROLLER_H_

/*
 * Local Variables:
 * mode: C++
 * End:
 */
