#ifndef TDF_PROGRAMMABLE_GAIN_AMPLIFIER_H_
#define TDF_PROGRAMMABLE_GAIN_AMPLIFIER_H_

#include <cmath>
#include <systemc-ams>

namespace heter {

class tdf_programmable_gain_amplifier : public sca_tdf::sca_module {
public:
  sca_tdf::sca_in<double> in;
  sca_tdf::sca_in<int> k_in;
  sca_tdf::sca_out<double> out;

  tdf_programmable_gain_amplifier(sc_core::sc_module_name nm, double v_supply, long n_samples)
  : in("in"), k_in("k_in"), out("out"), v_supply_(v_supply), n_samples_(n_samples)
  {
    sc_assert(v_supply > 0.0);
    sc_assert(n_samples > 0);
  }

  // Virtual destructor
  virtual ~tdf_programmable_gain_amplifier() {}

protected:
  void set_attributes() {
    // TODO: Set the right sample rates on the TDF ports. 
    // Note: The gain controller updates only every n_samples_ its output.
  }

  void initialize() {}

  void processing() {
    // Read gain control input.
    double k = k_in.read();
    // TODO: Modify the following amplifier description to process
    // n_samples_ per processing() activation.
    double val = std::pow(2.0, k) * in.read();
    // Test if output saturates.
    if (val > v_supply_) {
      out.write(v_supply_);
    } else if (val < -v_supply_) {
      out.write(-v_supply_);
    } else {
      out.write(val);
    }
  }

private:
  const double v_supply_; // Supply voltage limiting output.
  const long n_samples_;  // Number of samples constituting a control period.
}; // class tdf_programmable_gain_amplifier

} // namespace std;

#endif // TDF_PROGRAMMABLE_GAIN_AMPLIFIER_H_

/*
 * Local Variables:
 * mode: C++
 * End:
 */
