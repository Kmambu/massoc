#ifndef RF_FRONT_END_H
#define RF_FRONT_END_H

#include <systemc-ams>

#include "mixer.h"
#include "lp_filter_ltf.h"
#include "ad_converter.h"

// RF front end module with structural description.
template<int NBITS = 8>
class rf_front_end : public sc_core::sc_module {
public:
  // Ports.
  sca_tdf::sca_in<double> rf_in, osc_in;
  sc_core::sc_out<sc_dt::sc_uint<NBITS> > dig_out;

  // Internal components.
  mixer mxr_1;
  lp_filter_ltf lpf_1;
  ad_converter<NBITS> adc_1;

  // Internal signals.
  sca_tdf::sca_signal<double> mxr_sig, lp_sig;

  // Construct RF front end using the passed parameters for the components.
  rf_front_end(sc_core::sc_module_name nm, 
               sca_core::sca_time Ts,
               double inmin, double inmax, 
               unsigned long Dq = NBITS/2, unsigned long Rrf = 1000000,
               unsigned long phirf = 0,
               double fc = 200.0e3, double H0 = 1.0)
  : mxr_1("mxr_1"), lpf_1("lpf_1", fc, H0),
    adc_1("adc_1", Ts, inmin, inmax, Dq, Rrf, phirf)
  {
    // Specify connectivity using port-to-port and port-to-signal binding.

    // TODO: Connect the mixer to the inputs.

    // TODO: Connect the LPF to the mixer output.

    // TODO: Connect the ADC to the LPF output.
  }
};  // class rf_front_end

#endif // RF_FRONT_END_H

/*
 * Local Variables:
 * mode: C++
 * End:
 */
