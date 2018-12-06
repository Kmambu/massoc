#ifndef TDF_SENSOR_FRONTEND_H_
#define TDF_SENSOR_FRONTEND_H_

#include <systemc-ams>
#include "tdf_programmable_gain_amplifier.h"
#include "ad_converter.h"
#include "tdf_abs_amplitude_averager.h"
#include "tdf_gain_controller.h"


namespace heter {

template<int NBitsADC>
class tdf_sensor_frontend : public sc_core::sc_module {
public:
  sca_tdf::sca_in<double> v_in;
  sca_tdf::sca_out<sc_dt::sc_int<NBitsADC> > adc_out;
  sca_tdf::sca_out<int> k_out;


  tdf_sensor_frontend(sc_core::sc_module_name nm, sca_core::sca_time dt_adc,
                      double v_supply, double avg_n_samples,
                      double amp_low_threshold, double amp_high_threshold,
                      int k_0, int k_min, int k_max)
  : v_in("v_in"), adc_out("adc_out"), k_out("k_out"),
    v_amp_sig("v_amp_sig"), amp_sig("amp_sig"),
    pga_1("pga_1", v_supply, avg_n_samples),
    adc_1("adc_1", v_supply),
    avg_1("avg_1", avg_n_samples),
    gain_ctrl_1("gain_ctrl_1",
                amp_low_threshold, amp_high_threshold,
                k_0, k_min, k_max)
  {
    // Connectivity
    pga_1.in(v_in);
    pga_1.k_in(k_out);
    pga_1.out(v_amp_sig);

    adc_1.set_timestep(dt_adc);
    adc_1.in(v_amp_sig);
    adc_1.out(adc_out);

    avg_1.in(adc_out);
    avg_1.out(amp_sig);

    gain_ctrl_1.amp_in(amp_sig);
    gain_ctrl_1.k_out(k_out);
  }

  // Trace internal signal of the module into the passed trace file.
  void trace(sca_util::sca_trace_file* tfp) {
    sca_trace(tfp, v_amp_sig, v_amp_sig.name());
    sca_trace(tfp, amp_sig, amp_sig.name());
  }

private:
  sca_tdf::sca_signal<double> v_amp_sig;
  sca_tdf::sca_signal<sc_dt::sc_int<NBitsADC> > amp_sig;

  tdf_programmable_gain_amplifier pga_1;
  ad_converter<NBitsADC> adc_1;
  tdf_abs_amplitude_averager<NBitsADC> avg_1;
  tdf_gain_controller<NBitsADC> gain_ctrl_1;
}; // class tdf_sensor_frontend

} // namespace heter

#endif // TDF_SENSOR_FRONTEND_H_

/*
 * Local Variables:
 * mode: C++
 * End:
 */
