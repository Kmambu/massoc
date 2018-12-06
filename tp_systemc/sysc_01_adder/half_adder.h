#ifndef HALF_ADDER_H
#define HALF_ADDER_H

// File : half_adder.h
#include "systemc.h"

SC_MODULE(half_adder)
{
	sc_in<bool> a,b;
	sc_out<bool> sum,carry;

	void prc_half_adder();

	SC_CTOR(half_adder) : a("a"),b("b"),sum("sum"),carry("carry")
	{
		SC_METHOD(prc_half_adder);
		sensitive << a << b;
	}
};

#endif // HALF_ADDER_H
