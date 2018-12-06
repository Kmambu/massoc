#ifndef LP_FILTER_ELN_H
#define LP_FILTER_ELN_H

#include <systemc-ams>

// Lowpass filter module using an embedded structural ELN description.
class lp_filter_eln : public sc_core::sc_module {
public:
  sca_tdf::sca_in<double> in;   // Input.
  sca_tdf::sca_out<double> out; // Output.

  sca_eln::sca_node n1;         // Electrical nodes (TODO: add as necessary).
  sca_eln::sca_node_ref gnd;    // Electrical reference node.

  // Internal ELN primitives.
  sca_eln::sca_tdf::sca_vsource tdf2v_1;
  // TODO: Add the R and C instances of the filter
  sca_eln::sca_tdf::sca_vsink v2tdf_1;

  // Construct filter having the specified cut-off frequency and gain.
  lp_filter_eln(sc_core::sc_module_name nm, double R1, double C1, double H0 = 1.0)
  : in("in"), out("out"), n1("n1"), /* TODO: initialize additional nodes */
    gnd("gnd"),
    tdf2v_1("tdf2v_1"),
    // TODO: Initialize the R an C instances.
    v2tdf_1("v2tdf_1", H0)
  {
    // Specify connectivity.
    tdf2v_1.inp(in);
    tdf2v_1.p(n1); tdf2v_1.n(gnd);

    // TODO: Interconnect the R and C to a 1st order low pass filter.

    v2tdf_1.p(n1); v2tdf_1.n(gnd);
    v2tdf_1.outp(out);
  }
}; // class lp_filter_eln

#endif // LP_FILTER_ELN_H

/*
 * Local Variables:
 * mode: C++
 * End:
 */
