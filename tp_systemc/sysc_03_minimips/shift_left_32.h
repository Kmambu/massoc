#ifndef _SHIFT_LEFT_32_H
#define _SHIFT_LEFT_32_H
#include "systemc.h"

SC_MODULE(shift_left_32)
{
	sc_in<sc_uint<32> > I;
	sc_out<sc_uint<32> > O;

	SC_CTOR(shift_left_32)
	{
		SC_METHOD(mWrite);
		sensitive << I ;
	}

	void mWrite()
	{
		sc_uint<32> i_value=I.read();
		sc_uint<32> o_value=0;

		o_value.range(31,2)=i_value.range(29,0);

		O.write(o_value);
	}
};
#endif

