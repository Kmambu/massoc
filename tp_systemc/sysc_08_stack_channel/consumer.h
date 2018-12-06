#ifndef CONSUMER_H
#define CONSUMER_H

#include "systemc.h"
#include "stack_if.h"

class consumer : public sc_module
{
public:

  sc_port<stack_read_if> in;
  sc_in<bool> Clock;

  void do_reads()
  {
    char c;

    while (true)
    {
      wait();             // for clock edge
      if (in->nb_read(c))
        cout << "R " << c << " at "
             << sc_time_stamp() << endl;
    }
  }

  SC_CTOR(consumer)
  {
    SC_THREAD(do_reads);
      sensitive << Clock.pos();
  }
};

#endif
