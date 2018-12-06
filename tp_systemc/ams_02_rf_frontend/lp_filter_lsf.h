#ifndef LP_FILTER_LSF_H
#define LP_FILTER_LSF_H

#include <cmath>
#include <systemc-ams>

// Lowpass filter module using an embedded structural LSF description.
class lp_filter_lsf : public sc_core::sc_module {
public:
  sca_tdf::sca_in<double> in;   // Input.
  sca_tdf::sca_out<double> out; // Output.

  sca_lsf::sca_signal x_sig; // Internal LSF signals (TODO: add as necessary).

  // Internal LSF primitives.
  sca_lsf::sca_tdf::sca_source tdf2lsf_1;
  // TODO: Add LSF primitives to implement 1st order LPF
  sca_lsf::sca_tdf::sca_sink lsf2tdf_1;

  // Construct filter having the specified cut-off frequency and gain.
  lp_filter_lsf(sc_core::sc_module_name nm, double fc, double H0 = 1.0)
  : in("in"), out("out"), x_sig("x_sig"), /* TODO: initialize added signals */
    tdf2lsf_1("tdf2lsf_1"), 
    // TODO: initialize added LSF instances.
    lsf2tdf_1("lsf2tdf_1")
  {
    // Specify connectivity.
    tdf2lsf_1.inp(in); tdf2lsf_1.y(x_sig);

    // TODO: Interconnect LSF primitives to 1st order LPF:

    lsf2tdf_1.x(x_sig); lsf2tdf_1.outp(out);
  }
}; // class lp_filter_lsf

#endif // LP_FILTER_LSF_H

/*
 * Local Variables:
 * mode: C++
 * End:
 */
