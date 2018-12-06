#ifndef SINK_H
#define SINK_H
template<
class C
  >
SCA_TDF_MODULE (sink)
{
   sca_tdf::sca_in< C >in;

  void sig_proc () {}
  SCA_CTOR (sink) {}
};
#endif
