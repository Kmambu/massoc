#include <systemc.h>
#include "DF_Adder.h"
#include "DF_Const.h"
#include "DF_Fork.h"
#include "DF_Printer.h"

int sc_main(int argc, char** argv)
{
  sc_fifo<int> const_out("const_out", 5);
  sc_fifo<int> adder_out("adder_out",1);
  sc_fifo<int> feedback("feedback", 1);
  sc_fifo<int> to_printer("to_printer", 1);

  DF_Const<int> constant("constant",1);
  constant.output(const_out);

  DF_Adder<int> adder("adder");
  adder.input1(feedback);
  adder.input2(const_out);
  adder.output(adder_out);

  DF_Fork<int> fork("fork");
  fork.input(adder_out);
  fork.output1(feedback);
  fork.output2(to_printer);

  DF_Printer<int> printer("printer", 10);
  printer.input(to_printer);

  //initial values otherwise the system deadlocks
  feedback.write(42);

  sc_start();

  return 0;
}
