#ifndef DE_ABS_AMPLITUDE_AVERAGER_H_
#define DE_ABS_AMPLITUDE_AVERAGER_H_

#include <cmath>
#include <systemc-ams>

namespace heter {

// Absolute amplitude averager.
template<int NBits>
class de_abs_amplitude_averager : public sca_tdf::sca_module {
public:
  sca_tdf::sca_in<sc_dt::sc_int<NBits> > in;
  sca_tdf::sca_de::sca_out<bool> clk;
  sca_tdf::sca_de::sca_out<sc_dt::sc_int<NBits> > out;

  explicit de_abs_amplitude_averager(sc_core::sc_module_name nm, long n_samples)
  : in("in"), clk("clk"), out("out"), n_samples_(n_samples)
  {
    sc_assert((2 <= NBits) && (NBits <= 64));
    sc_assert(n_samples > 0);
  }

  void set_attributes() {
    // TODO 1: Specify the correct port rates to process n_samples_ from
    //         the input. Generate a clock pulse along with the
    //         calculated average of the absolute amplitudes. 

    // TODO 2: After an initial simulation try, establish causality by
    //         assigning appropriate delays.
  }

  void initialize() {
    // TODO 2: Initialize any delayed port samples.
  }

  void processing() {
    // TODO 1: Generate the clock signal.

    // Calculate and output average of absolute amplitudes.
    long sum = 0;
    for (long i = 0; i < n_samples_; ++i) {
      sum += std::labs(in.read(i));
    }
    long avg = sum / n_samples_;
    out.write(avg);
  }

private:
  const long n_samples_;  // Number of averaged samples.
}; // class de_abs_amplitude_averager

} // namespace heter

#endif // ABS_AMPLITUDE_AVERAGER_H_

/*
 * Local Variables:
 * mode: C++
 * End:
 */
