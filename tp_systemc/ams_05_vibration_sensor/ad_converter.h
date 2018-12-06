#ifndef AD_CONVERTER_H_
#define AD_CONVERTER_H_

#include <cmath>
#include <systemc-ams>

namespace heter {

//! \brief Analog to digital converter with NBits resolution for the
//!        signed integer output.
template<int NBits>
class ad_converter : public sca_tdf::sca_module {
public:
  sca_tdf::sca_in<double> in;
  sca_tdf::sca_out<sc_dt::sc_int<NBits> > out;

  ad_converter(sc_core::sc_module_name nm, double v_max)
  : in("in"), out("out"), v_max_(v_max)
  {
    sc_assert((2 <= NBits) && (NBits <= 64));
    sc_assert(v_max > 0.0);
  }

  void set_attributes() {
    in.set_rate(1);
    out.set_rate(1);
  }

  void initialize() {}

  void processing() {
    using namespace std;
    double v_in = in.read();
    if (v_in < -v_max_) {
      out.write(-((1 << (NBits - 1)) - 1));
    } else if (v_in > v_max_) {
      out.write((1 << (NBits - 1)) - 1);
    } else {
      sc_dt::sc_int<NBits>
        q_v_in = lround((v_in / v_max_) * ((1 << (NBits - 1)) - 1));
      out.write(q_v_in);
    }
  }

private:
  const double v_max_;
};  // class ad_converter

} // namespace heter

#endif // AD_CONVERTER_H_

/*
 * Local Variables:
 * mode: C++
 * End:
 */
