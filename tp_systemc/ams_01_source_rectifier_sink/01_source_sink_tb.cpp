#include <systemc-ams>
#include "sin_source.h"
#include "sink.h"

// Simulation entry point.
int sc_main(int argc, char *argv[]) {
  // Declare signals to interconnect source and sink.
  sca_tdf::sca_signal<double> sig_1("sig_1");

  // Instantiate source and sink as well as bind their ports to the signal.
  sin_source src_1("src_1");
  src_1.out(sig_1);

  sink sink_1("sink_1");
  sink_1.in(sig_1);

  // Configure signal tracing.
  sca_util::sca_trace_file* tfp = sca_util::sca_create_tabular_trace_file("01_source_sink_tb");
  sca_util::sca_trace(tfp, sig_1, "sig_1");

  // Start simulation.
  sc_core::sc_start(100.0, sc_core::SC_MS);

  // Close trace file and stop simulation to enable clean-up by
  // asking SystemC to execute all end_of_simulation() callbacks.
  sca_util::sca_close_tabular_trace_file(tfp);
  sc_core::sc_stop();
  return 0;
}
