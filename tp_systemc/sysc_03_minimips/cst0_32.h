#ifndef _CST0_32_H
#define _CST0_32_H
#include "systemc.h"

SC_MODULE(cst0_32)
{
	sc_out<sc_uint<32> > S;

	SC_CTOR(cst0_32)
	{
		SC_METHOD(mWrite);
	}

	void mWrite()
	{
		S.write(0) ;
	}
};
#endif

