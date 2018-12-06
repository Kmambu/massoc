#ifndef LP_FILTER_LSF_H
#define LP_FILTER_LSF_H

#include <cmath>
#include <systemc-ams>

// Lowpass filter module using an embedded structural LSF description.
class lp_filter_lsf : public sc_core::sc_module {
public:
  sca_tdf::sca_in<double> in;   // Input.
  sca_tdf::sca_out<double> out; // Output.

  sca_lsf::sca_signal x_sig, y_sig, z_sig; // Internal LSF signals.

  // Internal LSF primitives.
  sca_lsf::sca_tdf::sca_source tdf2lsf_1;
  sca_lsf::sca_sub sub_1;
  sca_lsf::sca_dot dot_1;
  sca_lsf::sca_tdf::sca_sink lsf2tdf_1;

  // Construct filter having the specified cut-off frequency and gain.
  lp_filter_lsf(sc_core::sc_module_name nm, double fc, double H0 = 1.0)
  : in("in"), out("out"), x_sig("x_sig"), y_sig("y_sig"), z_sig("z_sig"),
    tdf2lsf_1("tdf2lsf_1"), sub_1("sub_1", H0),
    dot_1("dot_1", 1.0 / (2.0 * M_PI * fc)), lsf2tdf_1("lsf2tdf_1")
  {
    // Specify connectivity.
    tdf2lsf_1.inp(in); tdf2lsf_1.y(x_sig);

    sub_1.x1(x_sig); sub_1.x2(z_sig);
    sub_1.y(y_sig);

    dot_1.x(y_sig); dot_1.y(z_sig);

    lsf2tdf_1.x(y_sig); lsf2tdf_1.outp(out);
  }
}; // class lp_filter_lsf

#endif // LP_FILTER_LSF_H

/*
 * Local Variables:
 * mode: C++
 * End:
 */
