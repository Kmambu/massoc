#ifndef _CPU_H
#define _CPU_H
#include "systemc.h"

#include "constants.h"
#include "fsm.h"
#include "alu_32.h"
#include "pc_reg_32.h"
#include "shift_left_32.h"
#include "sign_extend_32.h"
#include "nosign_extend_32.h"
#include "mux21_5.h"
#include "mux21_32.h"
#include "mux81_32.h"
#include "rf32.h"
#include "plain_reg_32.h"
#include "ir_reg_32.h"
#include "cst4_32.h"
#include "cst0_32.h"

SC_MODULE(cpu)
{
	sc_in<bool> clk;
	sc_in<bool> reset;
	sc_out<sc_uint<32> > address;
	sc_out<sc_uint<2> > memrw;
	sc_out<sc_uint<32> > dataout;
	sc_in<sc_uint<32> > datain;

	sc_signal<bool> write_pc;
	sc_signal<bool> mux_rf_w;
	sc_signal<bool> mux_rf_r;
	sc_signal<bool> write_rf;
	sc_signal<sc_uint<5> > adr_rf_w;
	sc_signal<sc_uint<5> > adr_rf_r;
	sc_signal<bool> write_ad;
	sc_signal<bool> write_dt;
	sc_signal<bool> write_ir;
	sc_signal<sc_uint<3> > mux_x;
	sc_signal<sc_uint<3> > mux_y;
	sc_signal<bool> mux_addr;
	sc_signal<sc_uint<3> > alu_op;
	sc_signal<bool> zero;
	sc_signal<sc_uint<32> > ir;

	sc_signal<sc_uint<32> > cst0out;
	sc_signal<sc_uint<32> > cst4out;
	sc_signal<sc_uint<32> > pcout;
	sc_signal<sc_uint<32> > rfout;
	sc_signal<sc_uint<32> > adout;
	sc_signal<sc_uint<32> > dtout;
	sc_signal<sc_uint<32> > irsextout;
	sc_signal<sc_uint<32> > irextout;
	sc_signal<sc_uint<32> > irshiftout;
	sc_signal<sc_uint<32> > mux_x_out;
	sc_signal<sc_uint<32> > mux_y_out;
	sc_signal<sc_uint<32> > int_dataout;
	sc_signal<sc_uint<16> > irout15_0;
	sc_signal<sc_uint<5> > irout15_11;
	sc_signal<sc_uint<5> > irout20_16;
	sc_signal<sc_uint<5> > irout25_21;

	pc_reg_32 *pc_1;
	rf32 *rf32_1;
	plain_reg_32 *reg_ad_1;
	plain_reg_32 *reg_dt_1;
	ir_reg_32 *ir_reg_32_1;
	mux21_5 *mux_rf_r_1;
	mux21_5 *mux_rf_w_1;
	mux81_32 *mux_a_1;
	mux81_32 *mux_b_1;
	sign_extend_32 *sign_extend_32_1;
	nosign_extend_32 *nosign_extend_32_1;
	shift_left_32 *shift_left_32_1;
	mux21_32 *mux_addr_1;
	alu_32 *alu_32_1;
	cst4_32 *cst4_1;
	cst0_32 *cst0_1;
	fsm *fsm_1;

	SC_CTOR(cpu)
	{
		// PC_REG_32
		pc_1=new pc_reg_32("pc_reg_32_1");

		pc_1->clk(clk);
		pc_1->reset(reset);
		pc_1->D(dataout);
		pc_1->PCWrite(write_pc);
		pc_1->Q(pcout);

		// RF32

		rf32_1=new rf32("rf32_1");
		rf32_1->clk(clk);
		rf32_1->RegWrite(write_rf);
		rf32_1->Addwrite(adr_rf_w);
		rf32_1->Addrd1(adr_rf_r);
		rf32_1->Writedata(int_dataout);
		rf32_1->data1(rfout);

		// AD
		reg_ad_1=new plain_reg_32("reg_ad_1");
		reg_ad_1->clk(clk);
		reg_ad_1->regWrite(write_ad);
		reg_ad_1->D(int_dataout);
		reg_ad_1->Q(adout);

		// DT
		reg_dt_1=new plain_reg_32("reg_dt_1");
		reg_dt_1->clk(clk);
		reg_dt_1->regWrite(write_dt);
		reg_dt_1->D(datain);
		reg_dt_1->Q(dtout);

		// IR_REG_32
		ir_reg_32_1=new ir_reg_32("ir_reg_32_1");
		ir_reg_32_1->clk(clk);
		ir_reg_32_1->D(datain);
		ir_reg_32_1->regWrite(write_ir);
		ir_reg_32_1->Q(ir);
		ir_reg_32_1->Q15_0(irout15_0);
		ir_reg_32_1->Q25_21(irout25_21);
		ir_reg_32_1->Q20_16(irout20_16);
		ir_reg_32_1->Q15_11(irout15_11);

		// SIGN_EXTEND
		sign_extend_32_1=new sign_extend_32("sign_extend_32_1");
		sign_extend_32_1->I(irout15_0);
		sign_extend_32_1->O(irextout);

		// NOSIGN_EXTEND
		nosign_extend_32_1=new nosign_extend_32("nosign_extend_32_1");
		nosign_extend_32_1->I(irout15_0);
		nosign_extend_32_1->O(irsextout);

		// SHIFT LEFT 2
		shift_left_32_1=new shift_left_32("shift_left_32_1");
		shift_left_32_1->I(irextout);
		shift_left_32_1->O(irshiftout);

		// CST0
		cst0_1=new cst0_32("cst0_1");
		cst0_1->S(cst0out);

		// CST4
		cst4_1=new cst4_32("cst4_1");
		cst4_1->S(cst4out);

		// MUX21_5 pour RF lecture
		mux_rf_r_1=new mux21_5("mux_rf_r_1");
		mux_rf_r_1->IN0(irout25_21);
		mux_rf_r_1->IN1(irout20_16);
		mux_rf_r_1->COM(mux_rf_r);
		mux_rf_r_1->S(adr_rf_r);

		// MUX21_5 pour ADW_RF
		mux_rf_w_1=new mux21_5("mux_rf_w_1");
		mux_rf_w_1->IN0(irout20_16);
		mux_rf_w_1->IN1(irout15_11);
		mux_rf_w_1->COM(mux_rf_w);
		mux_rf_w_1->S(adr_rf_w);

		// MUX81_32 pour a
		mux_a_1=new mux81_32("mux_a_1");
		mux_a_1->IN0(pcout);
		mux_a_1->IN1(rfout);
		mux_a_1->IN2(adout);
		mux_a_1->IN3(dtout);
		mux_a_1->IN4(cst0out);
		mux_a_1->IN5(cst0out);
		mux_a_1->IN6(cst0out);
		mux_a_1->IN7(cst0out);
		mux_a_1->COM(mux_x);
		mux_a_1->S(mux_x_out);

                // MUX81_32 pour b
                mux_b_1=new mux81_32("mux_b_1");
                mux_b_1->IN0(cst0out);
                mux_b_1->IN1(cst4out);
                mux_b_1->IN2(adout);
                mux_b_1->IN3(irextout);
                mux_b_1->IN4(irshiftout);
                mux_b_1->IN5(irsextout);
                mux_b_1->IN6(cst0out);
                mux_b_1->IN7(cst0out);
                mux_b_1->COM(mux_y);
                mux_b_1->S(mux_y_out);

		// MUX21_32 pour le PC et ALUOUT
		mux_addr_1=new mux21_32("mux_addr_1");
		mux_addr_1->IN0(adout);
		mux_addr_1->IN1(pcout);
		mux_addr_1->COM(mux_addr);
		mux_addr_1->S(address);

		// ALU

		alu_32_1=new alu_32("alu_32_1");
		alu_32_1->A(mux_x_out);
		alu_32_1->B(mux_y_out);
		alu_32_1->Aluop(alu_op);
		alu_32_1->Aluout(int_dataout);
		alu_32_1->zero(zero);

		// FSM
		fsm_1=new fsm("fsm_1");
		fsm_1->clk(clk);
		fsm_1->reset(reset);
		fsm_1->ir(ir);
		fsm_1->zero(zero);

		fsm_1->write_pc(write_pc);
		fsm_1->mux_rf_w(mux_rf_w);
		fsm_1->write_rf(write_rf);
		fsm_1->mux_rf_r(mux_rf_r);
		fsm_1->write_ad(write_ad);
		fsm_1->write_dt(write_dt);
		fsm_1->write_ir(write_ir);
		fsm_1->mux_x(mux_x);
		fsm_1->mux_y(mux_y);
		fsm_1->mux_addr(mux_addr);
		fsm_1->alu_op(alu_op);
		fsm_1->memrw(memrw);

		SC_METHOD(dataoutext);
		sensitive << int_dataout;
	}

	void dataoutext()
	{
		dataout.write(int_dataout);
	}
};
#endif
