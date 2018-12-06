#ifndef _MUX21_32_H
#define _MUX21_32_H
#include "systemc.h"

SC_MODULE(mux21_32)
{
	sc_in<sc_uint<32> > IN0;
	sc_in<sc_uint<32> > IN1;
	sc_in<bool> COM;
	sc_out<sc_uint<32> > S;

	SC_CTOR(mux21_32)
	{
		SC_METHOD(mWrite);
		sensitive << IN0 << IN1 << COM ;
	}

	void mWrite()
	{
		S.write(COM ? IN1.read() : IN0.read()) ;
	}
};
#endif

