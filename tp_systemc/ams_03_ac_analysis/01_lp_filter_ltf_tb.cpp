#include <iostream>
#include <cmath>
#include <systemc-ams>
#include "sin_source.h"
#include "lp_filter_ltf.h"
#include "sink.h"

// Simulation entry point.
int sc_main(int argc, char *argv[]) {
  // Import definitions from selected SystemC AMS namespaces.
  using namespace sc_core;
  using namespace sca_util;
  using namespace sca_ac_analysis;

  // Source parameters
  const double amp_src = 1.0;    // Source amplitude
  const double f_src = 100.0e5;  // Source frequency / Hz
  const double phi_src = 0.0;    // Phase / DEG
  const double ac_amp = 1.0;     // AC amplitude
  const double ac_phi = 0.0;     // AC phase / DEG
  // LPF parameters
  const double fc = 1.0e3;       // LPF cut-off frequency / Hz
  const double H0 = 1.0;         // LPF gain

  // Simulation parameters
  const sc_time t_sim(20.0 * (1.0 / f_src), SC_SEC);  // Time to simulate.
  const sc_time t_step(1.0 / (10.0 * f_src), SC_SEC); // Simulation time step.
  const double f_start = 1.0;    // Start frequency / Hz
  const double f_stop = 1.0e6;   // End frequency / Hz
  const double n_step = 61;      // Number of steps over the frequency range.

  // Declare signals to interconnect source and sink.
  sca_tdf::sca_signal<double> sig_1("sig_1"), sig_2("sig_2");

  // Instantiate source, filter, and sink as well as bind their ports
  // to the signals.
  sin_source src_1("src_1", 0.0, amp_src, f_src, phi_src, ac_amp, ac_phi);
  src_1.out(sig_1);
  src_1.set_timestep(t_step);

  lp_filter_ltf lpf_1("lpf_1", fc, H0);
  lpf_1.in(sig_1);
  lpf_1.out(sig_2);

  sink sink_1("sink_1");
  sink_1.in(sig_1);

  // Open trace file.
  sca_trace_file* tfp = sca_create_tabular_trace_file("01_lp_filter_ltf_tb");

  // Trace the signals.
  sca_trace(tfp, sig_1, sig_1.name());
  sca_trace(tfp, sig_2, sig_2.name());

  try {
    // Start transient simulation.
    sc_start(t_sim);

    // Reopen different trace file for AC analysis.
    // The registered traces are conserved.
    tfp->reopen("01_lp_filter_ltf_tb_ac");
    tfp->set_mode(sca_ac_format(SCA_AC_DB_DEG));

    sca_ac_start(f_start, f_stop, n_step, SCA_LOG);
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  sca_util::sca_close_tabular_trace_file(tfp);
  sc_stop();
  return sc_report_handler::get_count(SC_ERROR);
}
