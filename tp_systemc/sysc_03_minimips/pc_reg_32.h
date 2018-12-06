#ifndef _PC_REG_32_H
#define _PC_REG_32_H
#include "systemc.h"

SC_MODULE(pc_reg_32)
{
	sc_in<sc_uint<32> > D;
	sc_in<bool> clk;
	sc_in<bool> PCWrite;
	sc_in<bool> reset;
	sc_out<sc_uint<32> > Q;

	SC_CTOR(pc_reg_32)
	{
		SC_METHOD(mWrite);
		sensitive << clk.pos();
	}

	void mWrite()
	{
		sc_uint<32> d_input=D.read();

		if (reset==0)
		{
			Q = 0x0;
		}
		else if (PCWrite)
		{
			Q = d_input;
		}
	}
};
#endif

