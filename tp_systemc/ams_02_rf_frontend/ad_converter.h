#ifndef AD_CONVERTER_H
#define AD_CONVERTER_H

#include <systemc-ams>


// A/D converter module using TDF's multirate capabilities.
template<int NBITS>
class ad_converter : public sca_tdf::sca_module {
public:
  sca_tdf::sca_in<double> in;
  sca_tdf::sca_de::sca_out<sc_dt::sc_uint<NBITS> > out;

  ad_converter(sc_core::sc_module_name nm, sca_core::sca_time Ts,
               double inmin, double inmax,
               unsigned long Dq, unsigned long Rrf, unsigned long phirf)
  : in("in"), out("out"), Ts_(Ts), inmin_(inmin), inmax_(inmax),
    Dq_(Dq), Rrf_(Rrf), phirf_(phirf)
  {
    sc_assert(phirf < Rrf);  // Check parameters for consistency.
    sc_assert(inmin < inmax);
  }

protected:
  // Specify the module and port attributes.
  void set_attributes() {
    // TODO: Set the module time step to Ts_.

    // TODO: Set the input rate Rrf_ to oversample the RF signal.

    // TODO: Set the output delay Dq_ to model the delay induced by the
    //       quantisation process.
  }

  // Initialize delayed output samples.
  void initialize() {
    // TODO: Initialize output samples.
  }

  void processing() {
    // TODO: Read RF sample at index phirf_.
    double val = 0.0;
    // Quantize the read sample and write it to the delayed output port.
    if (val < inmin_) {
      out.write(0);
    } else if (val > inmax_) {
      out.write((1 << NBITS) - 1);
    } else {
      sc_dt::sc_uint<NBITS>
        q_val = ((val - inmin_) / (inmax_ - inmin_) * ((1 << NBITS) - 1));
      out.write(q_val);
    }
  }

private:
  sca_core::sca_time Ts_;       // Output sample period of ADC.
  double inmin_, inmax_;        // Input value range to be quantized.
  unsigned long Dq_;            // Output delay due to quantisation process.
  unsigned long Rrf_;           // Input rate to oversample RF signal.
  unsigned long phirf_;         // Index to input sample to be quantized.
}; // class ad_converter

#endif // AD_CONVERTER_H

/*
 * Local Variables:
 * mode: C++
 * End:
 */
