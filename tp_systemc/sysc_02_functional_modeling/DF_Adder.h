#ifndef DF_ADDER_H
#define DF_ADDER_H

#include <systemc.h>

template <class T>
SC_MODULE(DF_Adder)
{
  sc_fifo_in<T> input1, input2;
  sc_fifo_out<T> output;

  void process()
  {
    while(1)
    {
      output.write(input1.read() + input2.read());
    }
  }

  SC_CTOR(DF_Adder)
  : input1("input1"), input2("input2"), output("output")
  {
    SC_THREAD(process);
  }
};

#endif // DF_ADDER_H
