#ifndef _ALU_32_H
#define _ALU_32_H
#include "systemc.h"

SC_MODULE(alu_32)
{
	sc_in<sc_uint<32> > A;
	sc_in<sc_uint<32> > B;
	sc_in<sc_uint<3> > Aluop;
	sc_out<sc_uint<32> > Aluout;
	sc_out<bool> zero;

	SC_CTOR(alu_32)
	{
		SC_METHOD(mWrite);
		sensitive << A << B << Aluop ;
	}

	void mWrite()
	{
		sc_uint<32> result = 0 ;
		sc_uint<32> opA ;
		sc_uint<32> opB ;
	
		opA = A.read() ;
		opB = B.read() ;
	
		switch(Aluop.read())
		{
			case 0:
				result = opA & opB ;
				break;
			case 1:
				result = opA | opB ;
				break;
			case 2:
				result = opA + opB ;
				break;
			case 6:
				result = opA - opB ;
				break;
			case 7:
				result = opA | opB ;
				break;
			default:
				cout << "aluop illegal" << endl ;
				break;
	
		}
		Aluout=result;
		zero=(result==0)?1:0;
	}
};
#endif
