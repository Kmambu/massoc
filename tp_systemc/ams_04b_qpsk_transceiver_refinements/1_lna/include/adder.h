#ifndef ADDER_H
#define ADDER_H

SCA_TDF_MODULE (adder)
{
  sca_tdf::sca_in < double >inI;
  sca_tdf::sca_in < double >inQ;
  sca_tdf::sca_out < double >out;

  void processing () {
    out.write (inI.read()+inQ.read());
  }
  SCA_CTOR (adder) {}
};
#endif
