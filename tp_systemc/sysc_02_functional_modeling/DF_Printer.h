#ifndef DF_PRINTER_H
#define DF_PRINTER_H

#include <systemc.h>

template <class T>
SC_MODULE(DF_Printer)
{
  sc_fifo_in<T> input;

  unsigned n_iterations_;
  bool done_;

  void process()
  {
    for (unsigned i = 0; i < n_iterations_; i++)
    {
      T value = input.read();
      cout << name() << " " << value << endl;
    }
    done_ = true;
  }

  SC_HAS_PROCESS(DF_Printer);

  DF_Printer(sc_module_name NAME, unsigned N_ITER)
  : sc_module(NAME), input("input"), n_iterations_(N_ITER), done_(false)
  {
    SC_THREAD(process);
  }

  ~DF_Printer()
  {
    if (!done_)
      cout << name() << " not done yet " << endl;
  }
};

#endif
