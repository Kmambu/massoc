#ifndef _SIGN_EXTEND_32_H
#define _SIGN_EXTEND_32_H
#include "systemc.h"

SC_MODULE(sign_extend_32)
{
	sc_in<sc_uint<16> > I;
	sc_out<sc_uint<32> > O;

	SC_CTOR(sign_extend_32)
	{
		SC_METHOD(mWrite);
		sensitive << I ;
	}

	void mWrite()
	{
		sc_uint<16> i_value=I.read();

		if (i_value[15]==1)
			O.write(0xFFFF0000 | i_value);
		else
			O.write(i_value);
	}
};
#endif

