// File : full_adder.h
#ifndef FULL_ADDER_H
#define FULL_ADDER_H

#include "half_adder.h"

SC_MODULE(full_adder)
{
	sc_in<bool> a,b,carry_in;
	sc_out<bool> sum,carry_out;

	sc_signal<bool> c1,s1,c2;
	void prc_or();

	half_adder *ha1_ptr,*ha2_ptr;

	SC_CTOR(full_adder) : a("a"),b("b"),carry_in("carry_in"),
	 			sum("sum"), carry_out("carry_out"),
				c1("c1"),s1("s1"),c2("c2")
	{
		ha1_ptr=new half_adder("ha1");
		// Named association:
		ha1_ptr->a(a);
		ha1_ptr->b(b);
		ha1_ptr->sum(s1);
		ha1_ptr->carry(c1);

		ha2_ptr=new half_adder("ha2");
		// Positional association:
		(*ha2_ptr)(s1,carry_in,sum,c2);

		SC_METHOD(prc_or);
		sensitive << c1 << c2;
	}

	// A destructor
	~full_adder()
	{
		delete ha1_ptr;
		delete ha2_ptr;
	}
};

#endif // FULL_ADDER_H
