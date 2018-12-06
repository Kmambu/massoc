#ifndef _PLAIN_REG_32_H
#define _PLAIN_REG_32_H
#include "systemc.h"

SC_MODULE(plain_reg_32)
{
	sc_in<sc_uint<32> > D;
	sc_in<bool> regWrite;
	sc_in<bool> clk;
	sc_out<sc_uint<32> > Q;

	SC_CTOR(plain_reg_32)
	{
		SC_METHOD(mWrite);
		sensitive << clk.pos();
	}

	void mWrite()
	{
		if (regWrite)
		{
			Q = D;
		}
	}
};
#endif

