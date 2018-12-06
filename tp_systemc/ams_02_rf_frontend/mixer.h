#ifndef MIXER_H
#define MIXER_H

#include <systemc-ams>

// Ideal mixer module.
class mixer : public sca_tdf::sca_module {
public:
  sca_tdf::sca_in<double> in1, in2; // Inputs.
  sca_tdf::sca_out<double> out;     // Output.

  // Construct mixer and name its ports.
  mixer(sc_core::sc_module_name nm)
  : in1("in1"), in2("in2"), out("out")
  {}

protected:
  // Multiply input samples and write result to the out port.
  void processing() {
    // TODO: Implement mixer behaviour.
  }
}; // class mixer

#endif // MIXER_H

/*
 * Local Variables:
 * mode: C++
 * End:
 */
