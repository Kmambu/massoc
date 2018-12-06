#ifndef TDF_ABS_AMPLITUDE_AVERAGER_H_
#define TDF_ABS_AMPLITUDE_AVERAGER_H_

#include <cmath>
#include <systemc-ams>

namespace heter {

// Absolute amplitude averager.
template<int NBits>
class tdf_abs_amplitude_averager : public sca_tdf::sca_module {
public:
  sca_tdf::sca_in<sc_dt::sc_int<NBits> > in;
  sca_tdf::sca_out<sc_dt::sc_int<NBits> > out;

  explicit tdf_abs_amplitude_averager(sc_core::sc_module_name nm, long n_samples)
  : in("in"), out("out"), n_samples_(n_samples)
  {
    sc_assert((2 <= NBits) && (NBits <= 64));
    sc_assert(n_samples > 0);
  }

  void set_attributes() {
    // TODO: Specify the correct port rates to process
    //       n_samples_. Also specify an appropriate delay to restore
    //       causality in the feedback loop.
  }

  void initialize() {
    // TODO: Initialize the delayed output sample(s).
  }

  void processing() {
    long sum = 0;
    // TODO: Calculate and output the average of n_samples_ absolute amplitudes.
    // Note: Use std::labs() to calculate the absolute value of the read
    //       input value.
    long avg = sum / n_samples_;
    out.write(avg);
  }

private:
  const long n_samples_;  // Number of averaged samples.
};  // class tdf_abs_amplitude_averager

} // namespace heter

#endif // TDF_ABS_AMPLITUDE_AVERAGER_H_

/*
 * Local Variables:
 * mode: C++
 * End:
 */
