#ifndef _NOSIGN_EXTEND_32_H
#define _NOSIGN_EXTEND_32_H
#include "systemc.h"

SC_MODULE(nosign_extend_32)
{
	sc_in<sc_uint<16> > I;
	sc_out<sc_uint<32> > O;

	SC_CTOR(nosign_extend_32)
	{
		SC_METHOD(mWrite);
		sensitive << I ;
	}

	void mWrite()
	{
		sc_uint<16> i_value=I.read();

		O.write(0x00000000 | i_value.range(15,0));
	}
};
#endif

