#include <systemc-ams>
#include "harmonic_sine_wavelets_source.h"
#include "tdf_vibration_sensor.h"
#include "tdf_sensor_frontend.h"


int sc_main(int argc, char* argv[]) {
  using namespace sc_core;
  using namespace sca_util;
  using namespace heter;

  ////////////////////////////////////////////////////////////////////////
  // System components constants.
  ////////////////////////////////////////////////////////////////////////

  // vibration sensor transducer constant for transformation of speed into voltage.
  const double k_trans = 1.0; // V s / m

  // Resolution of the ADC.
  const int NBitsADC = 5;
  // ADC sampling period (equal to TDF time step).
  // const sc_time dt_adc(10.0, SC_US);
  const sc_time dt_adc(10.0, SC_US);

  // Absolute amplitude averager parameters.
  // const long avg_n_samples = 50;
  const long avg_n_samples = 50;

  // Gain controller parameters.
  const int amp_low_threshold = 0.2 * ((1 << (NBitsADC - 1)) - 1);
  const int amp_high_threshold = 0.66 * ((1 << (NBitsADC - 1)) - 1);
  const int k_0 = 8;
  const int k_min = 0;
  const int k_max = 16;


  ////////////////////////////////////////////////////////////////////////
  // Initial conditions and stimuli.
  ////////////////////////////////////////////////////////////////////////

  // Supply voltages for pre-amplifier and ADC.
  const double v_supply = 5.0; // V

  // Vibration test signal.
  const double x_offset = -8.0e-6; // m
  const double x_amplitude = 4.0e-6; // m
  const double x_f_0 = 2.0e3; // Hz
  const int x_n_period = 8;
  const int x_n_harmonic = 2;

  // Simulation stop time.
  const sc_time t_stop(25.0, SC_MS);


  ////////////////////////////////////////////////////////////////////////
  // Model instantiation.
  ////////////////////////////////////////////////////////////////////////

  // Signals
  sca_tdf::sca_signal<double> x_sig("x_sig"), v_sig("v_sig");
  sca_tdf::sca_signal<sc_dt::sc_int<NBitsADC> > adc_sig("adc_sig");
  sca_tdf::sca_signal<int> k_sig("k_sig");

  // Mechanical vibration source x(t).
  harmonic_sine_wavelets_source vib_src("vib_src", x_offset, x_amplitude, x_f_0,
                                        x_n_period, x_n_harmonic);
  vib_src.out(x_sig);

  // Vibration sensor with displacement input and velocity proportional voltage output.
  tdf_vibration_sensor vib_sensor("vib_sensor", k_trans);
  vib_sensor.x_in(x_sig);
  vib_sensor.v_out(v_sig);

  // Sensor frontend with PGA and gain controller to use fully the dynamic range of its ADC.
  tdf_sensor_frontend<NBitsADC> frontend("frontend", dt_adc,
                                         v_supply, avg_n_samples,
                                         amp_low_threshold, amp_high_threshold,
                                         k_0, k_min, k_max);
  frontend.v_in(v_sig);
  frontend.adc_out(adc_sig);
  frontend.k_out(k_sig);


  ////////////////////////////////////////////////////////////////////////
  // Tracing and simulation
  ////////////////////////////////////////////////////////////////////////

  // Tracing
  sca_trace_file *tfp = sca_create_tabular_trace_file("tdf_vibration_sensor_tb");
  sca_trace(tfp, x_sig, x_sig.name());
  sca_trace(tfp, v_sig, v_sig.name());
  frontend.trace(tfp);
  sca_trace(tfp, adc_sig, "adc_sig");
  sca_trace(tfp, k_sig, "k_sig");

  // Simulation
  try {
    sc_start(t_stop);
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  sca_close_tabular_trace_file(tfp);

  sc_stop();
  return sc_report_handler::get_count(SC_ERROR);
}
