#include <iostream>
#include <systemc-ams>
#include "sin_source.h"
#include "rf_front_end.h"

/*
 * Entry point to the SystemC test bench.
 *
 * argc:    Command line argument counter.
 * argv:    Command line argument values.
 * Returns: Program exit value (0 on success).
 */
int sc_main(int argc, char* argv[]) {
  using namespace sc_core; using namespace sca_util; using sca_core::sca_time;

  ////////////////////////////////////////////////////////////////////////
  // Component and simulation parameters.
  ////////////////////////////////////////////////////////////////////////

  const double fosc = 10.0e6;          // Carrier signal frequency.
  const double fbb = 4.0e3;            // Base band signal frequency.

  const double fc = 20.0e3;            // Filter cut-off freqency.
  const double H0 = 5.0;               // Filter gain.

  const double inmin(-0.5*H0),
               inmax(0.5*H0);          // ADC input range.
  const int NBITS = 8;                 // ADC output word size.
  const double fs = 100.0e3;           // ADC output samples frequency.
  const sca_time Ts(1.0 / fs, SC_SEC); // ADC output samples frequency.
  const unsigned long Rrf = 2000;      // Oversampling rate of RF signal at ADC input.
  const unsigned long phirf = 0;       // RF signal sample to quantise.
  const unsigned long Dq = NBITS / 4;  // ADC quantisation delay.

  const sc_core::sc_time t_sim(5.0, SC_MS);  // Time to simulate.


  ////////////////////////////////////////////////////////////////////////
  // Instantiate model.
  ////////////////////////////////////////////////////////////////////////

  sca_tdf::sca_signal<double> rf_sig("rf_sig"), osc_sig("osc_sig");
  sc_core::sc_signal<sc_dt::sc_uint<NBITS> > dig_sig("dig_sig");

  // RF signal at 10.004 MHz.
  sin_source rf_src_1("rf_src_1", 0.0, 1.0 , fosc + fbb, 0.0);
  rf_src_1.out(rf_sig);
  // Carrier with 10 MHz.
  sin_source osc_src_1("osc_src_1", 0.0, 1.0 , fosc, 0.0);
  osc_src_1.out(osc_sig);


  rf_front_end<NBITS> rffe_1("rffe_1", Ts, inmin, inmax, Dq, Rrf, phirf, fc, H0);
  rffe_1.rf_in(rf_sig);
  rffe_1.osc_in(osc_sig);
  rffe_1.dig_out(dig_sig);


  ////////////////////////////////////////////////////////////////////////
  // Tracing and simulation.
  ////////////////////////////////////////////////////////////////////////

  // Tracing of RF signals to DAT file.
  sca_trace_file *tfp_rf = sca_create_tabular_trace_file("01_rf_front_end_rf_tb_rf");
  sca_trace(tfp_rf, rf_sig, "rf_sig");
  sca_trace(tfp_rf, osc_sig, "osc_sig");
  sca_trace(tfp_rf, rffe_1.mxr_sig, "rffe_1.mxr_sig");
  sca_trace(tfp_rf, rffe_1.lp_sig, "rffe_1.lp_sig");
  // Tracing of base band signals to DAT file.
  sca_trace_file *tfp_bb = sca_create_tabular_trace_file("01_rf_front_end_rf_tb_bb");
  sca_trace(tfp_bb, dig_sig, "dig_sig");

  // Tracing to VCD file.
  sca_util::sca_trace_file* tfp_vcd = sca_util::sca_create_vcd_trace_file("01_rf_front_end_rf_tb");
  sca_trace(tfp_vcd, rf_sig, "rf_sig");
  sca_trace(tfp_vcd, osc_sig, "osc_sig");
  sca_trace(tfp_vcd, rffe_1.mxr_sig, "rffe_1.mxr_sig");
  sca_trace(tfp_vcd, rffe_1.lp_sig, "rffe_1.lp_sig");
  sca_trace(tfp_vcd, dig_sig, "dig_sig");

  // Simulation
  try {
    sc_start(t_sim);
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl; 
  }

  // Close trace files and stop simulation.
  sca_close_tabular_trace_file(tfp_rf);
  sca_close_tabular_trace_file(tfp_bb);
  sca_close_tabular_trace_file(tfp_vcd);
  sc_stop();
  return sc_core::sc_report_handler::get_count(sc_core::SC_ERROR);
}
