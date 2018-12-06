#ifndef DF_FORK_H
#define DF_FORK_H

#include <systemc.h>

template <class T>
SC_MODULE(DF_Fork)
{
  sc_fifo_in<T> input;
  sc_fifo_out<T> output1;
  sc_fifo_out<T> output2;

  void process()
  {
    while(1)
    {
      T value = input.read();
      output1.write(value);
      output2.write(value);
    }
  }

  SC_CTOR(DF_Fork)
  : input("input"), output1("output1"), output2("output2")
  {
    SC_THREAD(process);
  }
};

#endif // DF_FORK_H
