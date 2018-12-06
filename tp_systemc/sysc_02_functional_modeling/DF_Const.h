#ifndef DF_CONST_H
#define DF_CONST_H

#include <systemc.h>

template <class T> SC_MODULE(DF_Const)
{
  sc_fifo_out<T> output;
  T constant_;

  void process()
  {
    while(1)
      output.write(constant_);
  }

  SC_HAS_PROCESS(DF_Const);

  DF_Const(sc_module_name N, const T& C)
  : sc_module(N), output("output"), constant_(C)
  {
    SC_THREAD(process);
  }
};

#endif // DF_CONST_H
