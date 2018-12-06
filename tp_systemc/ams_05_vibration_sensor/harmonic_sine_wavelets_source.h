#ifndef HARMONIC_SINE_WAVELETS_SOURCE_H_
#define HARMONIC_SINE_WAVELETS_SOURCE_H_

#include <cmath>
#include <systemc-ams>

namespace heter {

//! Waveform generator module to output a sequence of sinusoidal wavelets.
class harmonic_sine_wavelets_source : public sca_tdf::sca_module {
public:
  sca_tdf::sca_out<double> out;

  //! Construct sinusoidal wavelets waveform generator.
  harmonic_sine_wavelets_source(sc_core::sc_module_name nm,
                                double offset, double amplitude, double f_0,
                                int n_period = 1, int n_harmonic = 3)
  : out("out"), offset_(offset), amplitude_(amplitude), f_0_(f_0),
    n_period_(n_period), n_harmonic_(n_harmonic),
    T_0_(1.0 / f_0_), T_period_(n_period * T_0_)
  {
    assert(f_0 > 0.0);
    assert(n_period >= 1);
    assert(n_harmonic >= 0);
  }

  //! Set TDF attributes.
  void set_attributes() {
    out.set_rate(1);
  }

  //! Calculate and output value of waveform at time t.
  void processing () {
    double t = this->get_time().to_seconds();
    double t_pos = fmod(t, T_period_);
    int harmonic = static_cast<int>(floor(t / T_period_)) % (n_harmonic_ + 1);
    double val = offset_;
    // val += pow(2.0, harmonic) * amplitude_ * sin(2.0 * M_PI * pow(2.0, harmonic) * f_0_ * t_pos);
    val += amplitude_ * sin(2.0 * M_PI * pow(2.0, harmonic) * f_0_ * t_pos);
    out.write(val);
  }

private:
  const double offset_;              //!< Offset of the sine wave.
  const double amplitude_;           //!< Amplitude of the sine wave.
  const double f_0_;    //!< Base frequency of the sine wave.
  const int n_period_;          //!< Number of periods for one wavelet with f_0_.
  const int n_harmonic_;        //!< Number of harmonics in the wavelet sequence.
  const double T_0_;         //!< Period of sine wave with f_0_.
  const double T_period_;    //!< Period of a sine wavelet.
};

} // namespace heter

#endif // HARMONIC_SINE_WAVELETS_SOURCE_H_

/*
 * Local Variables:
 * mode: C++
 * End:
 */
