#ifndef _FSM_H
#define _FSM_H
#include "systemc.h"
#include "constants.h"

#include <iostream>
#include <fstream>
#include <string>
using namespace std;

#define MEMNOP		0
#define MEMREAD		1
#define MEMWRITE	2

#define R_RS		0
#define R_RT		1

#define W_RT		0
#define W_RD		1

#define MUX_ADDR_AD	0
#define MUX_ADDR_PC	1

#define MUX_X_PC	0
#define MUX_X_RF	1
#define MUX_X_AD	2
#define MUX_X_DT	3
#define MUX_X_CST0	4

#define MUX_Y_CST0	0
#define MUX_Y_CST4	1
#define MUX_Y_AD	2
#define MUX_Y_SE32	3
#define MUX_Y_SHF2	4
#define MUX_Y_NSE32	5

#define ALU_OP_ADD	2
#define ALU_OP_SUB	6
#define ALU_OP_OR	7

#define S_RESET 0
#define S_PCPLUS4 1
#define S_LW1 2
#define S_LW2 3
#define S_LW3 4
#define S_SW1 5
#define S_SW2 6
#define S_SW3 7
#define S_ADDI1 8
#define S_ADDI2 9 
#define S_ADD1 10
#define S_ADD2 11 

SC_MODULE(fsm)
{
	sc_in<bool> clk;
	sc_in<bool> reset;
	sc_in<sc_uint<32> > ir;
	sc_in<bool> zero;

	sc_out<bool> write_pc;
	sc_out<bool> mux_rf_w;
	sc_out<bool> write_rf;
	sc_out<bool> mux_rf_r;
	sc_out<bool> write_ad;
	sc_out<bool> write_dt;
	sc_out<bool> write_ir;
	sc_out<sc_uint<3> > mux_x;
	sc_out<sc_uint<3> > mux_y;
	sc_out<bool> mux_addr;
	sc_out<sc_uint<3> > alu_op;
	sc_out<sc_uint<2> > memrw;

	sc_signal<int> state,next_state;

	SC_CTOR(fsm)
	{

		SC_METHOD(mReg);
		sensitive << clk.pos();
		SC_METHOD(mNextState);
		sensitive << state << ir << zero;
		SC_METHOD(mMooreOutputs);
		sensitive << state;
	}

	void mReg()
	{
		if (reset==0)
			state=S_RESET;
		else
			state=next_state;
		printf("state =%d\n", state.read());
	}

	void mNextState()
	{
		sc_uint<32> ir_value=ir.read();
		switch (state)
		{
			case S_RESET: // 0
				next_state=S_PCPLUS4;
				break;
			case S_PCPLUS4: // 1
				if ((int)ir_value.range(31,26)==OP_LW)
					next_state=S_LW1;
				else if ((int)ir_value.range(31,26)==OP_SW)
					next_state=S_SW1;
				else if ((int)ir_value.range(31,26)==OP_ADDI)
					next_state=S_ADDI1;
				else if ((int)ir_value.range(31,26)==OP_SPECIAL)
				{
					if ((int)ir_value.range(5,0)==FUNC_ADD)
					next_state=S_ADD1;
				}
				break;
			case S_LW1: //2
				next_state=S_LW2;
				break;
			case S_LW2: //3
				next_state=S_LW3;
				break;
			case S_LW3: //4
				next_state=S_PCPLUS4;
				break;
			case S_SW1:
				next_state=S_SW2;
				break;
			case S_SW2:
				next_state=S_SW3;
				break; 
			case S_SW3:
				next_state=S_PCPLUS4;
				break;
			case S_ADDI1:
				next_state=S_ADDI2;
				break;
			case S_ADDI2:
				next_state=S_PCPLUS4;
				break;
			case S_ADD1:
				next_state=S_ADD2;
				break;
			case S_ADD2:
				next_state=S_PCPLUS4;
				break;
			default:
				cout << "Erreur dans la fonction mNextState de fsm" << endl ;
				exit(1);
				break;
		}
	}

	void mMooreOutputs()
	{
		switch(state)
		{
			case S_RESET:
				write_pc=0;
				mux_rf_w=W_RT;
				write_rf=0;
				mux_rf_r=R_RS;
				write_ad=0;
				write_dt=0;
				write_ir=1;
				mux_x=MUX_X_CST0;
				mux_y=MUX_Y_CST0;
				mux_addr=MUX_ADDR_PC;
				alu_op=ALU_OP_ADD;
				memrw=MEMREAD;
				break;

			case S_PCPLUS4:
				write_pc=1;
				mux_rf_w=W_RT;
				write_rf=0;
				mux_rf_r=R_RS;
				write_ad=0;
				write_dt=0;
				write_ir=0;
				mux_x=MUX_X_PC;
				mux_y=MUX_Y_CST4;
				mux_addr=MUX_ADDR_PC;
				alu_op=ALU_OP_ADD;
				memrw=MEMNOP;
				break;

			case S_LW1: // RS+IMD16 -> AD
				write_pc=0;
				mux_rf_w=W_RT;
				write_rf=0;
				mux_rf_r=R_RS;
				write_ad=1;
				write_dt=0;
				write_ir=0;
				mux_x=MUX_X_RF;
				mux_y=MUX_Y_SE32;
				mux_addr=MUX_ADDR_PC;
				alu_op=ALU_OP_ADD;
				memrw=MEMNOP;
				break;

			case S_LW2:
				write_pc=0;
				mux_rf_w=W_RT;
				write_rf=0;
				mux_rf_r=R_RS;
				write_ad=0;
				write_dt=1;
				write_ir=0;
				mux_x=MUX_X_CST0;
				mux_y=MUX_Y_CST0;
				mux_addr=MUX_ADDR_AD;
				alu_op=ALU_OP_ADD;
				memrw=MEMREAD;
				break;

			case S_LW3:  // DT -> R[20:16], MEM[PC]->IR
				write_pc=0;
				mux_rf_w=W_RT;
				write_rf=1;
				mux_rf_r=R_RS;
				write_ad=0;
				write_dt=0;
				write_ir=1;
				mux_x=MUX_X_DT; // DT
				mux_y=MUX_Y_CST0; //CST0
				mux_addr=MUX_ADDR_PC;
				alu_op=ALU_OP_ADD;
				memrw=MEMREAD;
				break;

			case S_SW1: // RS+IMD16 -> AD
				write_pc=0;
				mux_rf_w=W_RT;
				write_rf=0;
				mux_rf_r=R_RS;
				write_ad=1;
				write_dt=0;
				write_ir=0;
				mux_x=MUX_X_RF;
				mux_y=MUX_Y_SE32;
				mux_addr=MUX_ADDR_PC;
				alu_op=ALU_OP_ADD;
				memrw=MEMNOP;
				break;

			case S_SW2: // R[20->16] -> MEM[AD]
				write_pc=0;
				mux_rf_w=W_RT;
				write_rf=0;
				mux_rf_r=R_RT;
				write_ad=0;
				write_dt=0;
				write_ir=0;
				mux_x=MUX_X_RF; // RF
				mux_y=MUX_Y_CST0; // CST0
				mux_addr=MUX_ADDR_AD;
				alu_op=ALU_OP_ADD;
				memrw=MEMWRITE;
				break;

			case S_SW3: // MEM[PC] -> IR
				write_pc=0;
				mux_rf_w=W_RT;
				write_rf=0;
				mux_rf_r=R_RS;
				write_ad=0;
				write_dt=0;
				write_ir=1;
				mux_x=MUX_X_CST0; // CST0
				mux_y=MUX_Y_CST0; // CST0
				mux_addr=MUX_ADDR_PC;
				alu_op=ALU_OP_ADD;
				memrw=MEMREAD;
				break;

			case S_ADDI1: // AD <- RS + SX[IR]
				write_pc=0;
				mux_rf_w=W_RT;
				write_rf=0;
				mux_rf_r=R_RS;
				write_ad=1;
				write_dt=0;
				write_ir=0;
				mux_x=MUX_X_RF;
				mux_y=MUX_Y_SE32;
				mux_addr=MUX_ADDR_PC;
				alu_op=ALU_OP_ADD;
				memrw=MEMNOP;
				break;

			case S_ADDI2: // RT <- AD
				            // IR <- MEM[PC]
				write_pc=0;
				mux_rf_w=W_RT;
				write_rf=1;
				mux_rf_r=R_RS;
				write_ad=0;
				write_dt=0;
				write_ir=1;
				mux_x=MUX_X_AD;
				mux_y=MUX_Y_CST0;
				mux_addr=MUX_ADDR_PC;
				alu_op=ALU_OP_ADD;
				memrw=MEMREAD;
				break;

			case S_ADD1: // AD <- RS
				write_pc=0;
				mux_rf_w=W_RT;
				write_rf=0;
				mux_rf_r=R_RS;
				write_ad=1;
				write_dt=0;
				write_ir=0;
				mux_x=MUX_X_RF;
				mux_y=MUX_Y_CST0;
				mux_addr=MUX_ADDR_PC;
				alu_op=ALU_OP_ADD;
				memrw=MEMNOP;
				break;

			case S_ADD2: // RD <- AD + RT
				           // IR <- MEM[PC]
				write_pc=0;
				mux_rf_w=W_RD;
				write_rf=1;
				mux_rf_r=R_RT;
				write_ad=0;
				write_dt=0;
				write_ir=1;
				mux_x=MUX_X_RF;
				mux_y=MUX_Y_AD;
				mux_addr=MUX_ADDR_PC;
				alu_op=ALU_OP_ADD;
				memrw=MEMREAD;
				break;
			default:
				cout << "Erreur dans la fonction mMooreOutputs de fsm" << endl ;
				exit(1);
				break;
		}
	}
};
#endif
