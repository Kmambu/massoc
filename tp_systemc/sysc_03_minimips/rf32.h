#ifndef _RF32_H
#define _RF32_H
#include "systemc.h"

SC_MODULE(rf32)
{
	sc_in<bool> clk;
	sc_in<bool> RegWrite;
	sc_in<sc_uint<5> > Addwrite;
	sc_in<sc_uint<5> > Addrd1;
	sc_in<sc_uint<32> > Writedata;
	sc_out<sc_uint<32> > data1;

	sc_signal<sc_uint<32> > regFile[32];

	SC_CTOR(rf32)
	{
		SC_METHOD(mReadRegs);
		sensitive << Addrd1 ;
		sensitive << clk.neg() ;
		SC_METHOD(mWriteReg);
		sensitive << clk.pos() ;
	}

	void mReadRegs()
	{
		if (Addrd1.read()==0)
			data1.write(0);
		else
			data1.write(regFile[Addrd1.read()].read());
	}

	void mWriteReg()
	{
		if (RegWrite)
			if (Addwrite.read()!=0)
			{
				regFile[Addwrite.read()].write(Writedata.read());
		//		printf("rf[%d]=%d\n",(int)Addwrite.read(),(int)Writedata.read());
			}
	}
};
#endif
