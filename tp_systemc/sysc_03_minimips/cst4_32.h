#ifndef _CST4_32_H
#define _CST4_32_H
#include "systemc.h"

SC_MODULE(cst4_32)
{
	sc_out<sc_uint<32> > S;

	SC_CTOR(cst4_32)
	{
		SC_METHOD(mWrite);
	}

	void mWrite()
	{
		S.write(4) ;
	}
};
#endif

