#ifndef _RAM_H
#define _RAM_H
#include "systemc.h"

using namespace std;

#include <iostream>
#include <fstream>
#include <string>

SC_MODULE(ram)
{
	sc_in<sc_uint<32> > addr;
	sc_out<sc_uint<32> > dout;
	sc_in<sc_uint<32> > din;
	sc_in<sc_uint<2> > memrw;
	sc_in<bool> clk;

	sc_uint<32> ramContents[100];

	SC_CTOR(ram)
	{
		SC_METHOD(mRead);
		sensitive << addr << memrw;
		SC_METHOD(mWrite);
		sensitive << clk.pos();

		// AND test
		ramContents[0]=0x2001FFFF; // ADDI r1, r0, 0xFFFF
		ramContents[1]=0x20020FF0; // ADDI r2, r0, 0xFF0
		ramContents[2]=0x00221824; // AND  r3, r1, r2
		// BEQ test
		//ramContents[0]=0x10000001; // BEQ  r0, r0, 0x8
		//ramContents[1]=0x20010000; // ADDI r1, r0, 0x0
		//ramContents[2]=0x2001FFFF; // ADDI r1, r0, 0xFFFF
		// ADD test
		//ramContents[0]=0x20010001; // ADDI r1, r0, 0x1
		//ramContents[1]=0x20020002; // ADDI r2, r0, 0x2
		//ramContents[2]=0x00221820; // ADD  r3, r1, r2
		// Vanilla test
		//ramContents[0]=0x20010080;
		//ramContents[1]=0x8C220000;
		//ramContents[2]=0x8C230004;
		//ramContents[32]=0x00000001;
		//ramContents[33]=0x00000002;
	}

	void mRead()
	{
		if ((int)memrw.read()==1)
		{
			dout.write(ramContents[addr.read()>>2]) ;
		}
	}

	void mWrite()
	{
		if ((int)memrw.read()==2)
			ramContents[addr.read()>>2]=din.read() ;
	}
};
#endif

