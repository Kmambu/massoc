#ifndef _MUX21_5_H
#define _MUX21_5_H
#include "systemc.h"

SC_MODULE(mux21_5)
{
	sc_in<sc_uint<5> > IN0;
	sc_in<sc_uint<5> > IN1;
	sc_in<bool> COM;
	sc_out<sc_uint<5> > S;

	SC_CTOR(mux21_5)
	{
		SC_METHOD(mWrite);
		sensitive << IN0 << IN1 << COM ;
	}

	void mWrite()
	{
		int com=(int)COM.read();

		switch (com)
		{
			case 0:
				S.write(IN0.read()); break;
			case 1:
				S.write(IN1.read()); break;
		}
	}
};
#endif

