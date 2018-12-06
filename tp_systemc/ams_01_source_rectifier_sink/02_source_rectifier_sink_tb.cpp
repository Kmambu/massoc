#include <systemc-ams>
#include "sin_source.h"
// TODO: Include your rectifier module.
#include "sink.h"

// Simulation entry point.
int sc_main(int argc, char *argv[]) {
  // Make the SystemC core functions and the SystemC AMS utility
  // functions visible in the current context.
  using namespace sc_core;
  using namespace sca_util;

  // Declare signals to interconnect source, rectifier, and sink.
  sca_tdf::sca_signal<double> sig_1("sig_1");
  // TODO: Declare signal between rectifier and sink.

  // Instantiate source, rectifier and sink as well as bind their ports
  // to the signal.
  sin_source src_1("src_1");
  src_1.out(sig_1);

  // TODO: Add here the rectifier instance.

  sink sink_1("sink_1");
  sink_1.in(sig_1);  // TODO: Connect the rectifier to the sink.

  // Open trace file. This time, we can avoid the specification of the
  // namespace sca_util:: due to the earlier using namespace
  // statement.
  sca_trace_file* tfp = sca_create_tabular_trace_file("02_source_rectifier_sink_tb");

  // You can also pass directly the signal's hierarchical name to the
  // 3rd argument of sca_trace:
  sca_trace(tfp, sig_1, sig_1.name());
  // TODO: Trace the additional signal.

  // Start simulation.
  sc_start(100.0, SC_MS);

  // Close trace file and stop simulation.
  sca_close_tabular_trace_file(tfp);
  sc_stop();
  return 0;
}
