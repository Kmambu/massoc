#ifndef LP_FILTER_LTF_H
#define LP_FILTER_LTF_H

#include <cmath>
#include <systemc-ams>

// Lowpass filter module using a Laplace transfer function.
class lp_filter_ltf : public sca_tdf::sca_module {
public:
  sca_tdf::sca_in<double> in;   // Input.
  sca_tdf::sca_out<double> out; // Output.

  // Construct filter having the specified cut-off frequency fc and gain H0.
  lp_filter_ltf(sc_core::sc_module_name nm, double fc, double H0 = 1.0)
  : in("in"), out("out"), H0_(H0), ltf_(), num_(), den_()
  {
    // Initialise LTF numerator and denominator 
    num_(0) = 1.0;
    den_(0) = 1.0; den_(1) = 1.0/(2.0 * M_PI * fc);
  }

protected:

  // Apply LTF to read input samples and write result to out port.
  void processing() {
    out.write(ltf_(num_, den_, in.read(), H0_));
  }

  // Small-signal behavior
  void ac_processing() {
    using sca_ac_analysis::sca_ac;
    using sca_ac_analysis::sca_ac_ltf_nd;
    // TODO: Calculate the frequency-dependent complex output value by
    //       passing the input through the LTF.
    //
    // Attention: The AC result of an LTF is calculated with a
    //            different function than the transient solution!
  }

private:
  double H0_;                              // Filter gain.
  sca_tdf::sca_ltf_nd ltf_;                // Laplace transfer function object.
  sca_util::sca_vector<double> num_, den_; // Numerator and denominator of LTF.
}; // class lp_filter_lsf

#endif // LP_FILTER_LTF_H

/*
 * Local Variables:
 * mode: C++
 * End:
 */
