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
    // Laplace transfer function describing the 1st order low:
    //
    //                       1
    // H(s) = H0 * ---------------------
    //             1 + (1 / (2 pi fc)) s
    //
    // TODO: Initialise LTF numerator num_ and denominator den_ to
    //       implement the LTF.
  }

protected:
  // Apply LTF to the read input samples and write result to out port.
  void processing() {
    // TODO: Read input, pass it through the LTF and write to the output
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
