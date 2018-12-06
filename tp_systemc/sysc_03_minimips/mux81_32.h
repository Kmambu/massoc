#ifndef _MUX81_32_H
#define _MUX81_32_H
#include "systemc.h"

SC_MODULE(mux81_32)
{
	sc_in<sc_uint<32> > IN0;
	sc_in<sc_uint<32> > IN1;
	sc_in<sc_uint<32> > IN2;
	sc_in<sc_uint<32> > IN3;
	sc_in<sc_uint<32> > IN4;
	sc_in<sc_uint<32> > IN5;
	sc_in<sc_uint<32> > IN6;
	sc_in<sc_uint<32> > IN7;
	sc_in<sc_uint<3> > COM;
	sc_out<sc_uint<32> > S;

	SC_CTOR(mux81_32)
	{
		SC_METHOD(mWrite);
		sensitive << IN0 << IN1 << IN2 << IN3 << IN4 << IN5 << IN6 << IN7 << COM ;
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
			case 2:
				S.write(IN2.read()); break;
			case 3:
				S.write(IN3.read()); break;
			case 4:
				S.write(IN4.read()); break;
			case 5:
				S.write(IN5.read()); break;
			case 6:
				S.write(IN6.read()); break;
			case 7:
				S.write(IN7.read()); break;
		}
	}
};
#endif

