#ifndef DE_PROGRAMMABLE_GAIN_AMPLIFIER_H_
#define DE_PROGRAMMABLE_GAIN_AMPLIFIER_H_

#include <cmath>
#include <systemc-ams>

namespace heter {

class de_programmable_gain_amplifier : public sca_tdf::sca_module {
public:
  sca_tdf::sca_in<double> in;
  sca_tdf::sca_de::sca_in<int> k_in;
  sca_tdf::sca_out<double> out;

  de_programmable_gain_amplifier(sc_core::sc_module_name nm, double v_supply)
  : in("in"), k_in("k_in"), out("out"), v_supply_(v_supply)
  {
    sc_assert(v_supply > 0.0);
  }

  //! Virtual destructor
  virtual ~de_programmable_gain_amplifier() {}

protected:
  void set_attributes() {
    // Due to the reading of the gain control value through a DE->TDF
    // converter port, it is not anymore necessary to adapt the rates
    // between the input and control signal.
    in.set_rate(1);
    k_in.set_rate(1);
    out.set_rate(1);
  }

  void initialize() {}

  void processing() {
    // As the module is now single rate, we don't need anymore a loop
    // to process n samples per read gain control value.
    double k = k_in.read();
    // Amplify input value to output value.
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
}; // class de_programmable_gain_amplifier

} // namespace std;

#endif // DE_PROGRAMMABLE_GAIN_AMPLIFIER_H_

/*
 * Local Variables:
 * mode: C++
 * End:
 */
