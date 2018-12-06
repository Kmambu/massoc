#ifndef _IR_REG_32_H
#define _IR_REG_32_H
#include "systemc.h"

SC_MODULE(ir_reg_32)
{
	sc_in<sc_uint<32> > D;
	sc_in<bool> regWrite;
	sc_in<bool> clk;
	sc_out<sc_uint<32> > Q;
	sc_out<sc_uint<16> > Q15_0;
	sc_out<sc_uint<5> > Q15_11;
	sc_out<sc_uint<5> > Q20_16;
	sc_out<sc_uint<5> > Q25_21;

	SC_CTOR(ir_reg_32)
	{
		SC_METHOD(mWrite);
		sensitive << clk.pos();
	}

	void mWrite()
	{
		if (regWrite)
		{
			sc_uint<32> din=D.read();
			Q = D;
			Q15_0.write(din.range(15,0));
			Q15_11.write(din.range(15,11));
			Q20_16.write(din.range(20,16));
			Q25_21.write(din.range(25,21));
		}
	}
};
#endif

