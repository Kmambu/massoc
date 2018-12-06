#include "systemc.h"
#include "ram.h"
#include "cpu.h"

int sc_main(int argc,char **argv)
{
	sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", SC_DO_NOTHING);

	cpu *cpu_1;
	ram *ram_1;

	sc_clock clk("clk");
	sc_signal<bool> reset("reset");
	sc_signal<sc_uint<32> > address("address");
	sc_signal<sc_uint<2> > memrw("memrw");
	sc_signal<sc_uint<32> > datain("datain");
	sc_signal<sc_uint<32> > dataout("dataout");

	// CPU 1 
	cpu_1=new cpu("cpu_1");
	cpu_1->clk(clk);
	cpu_1->reset(reset);
	cpu_1->address(address);
	cpu_1->memrw(memrw);
	cpu_1->dataout(dataout);
	cpu_1->datain(datain);

	// RAM 1 
	ram_1=new ram("ram_1");
	ram_1->clk(clk);
	ram_1->addr(address);
	ram_1->dout(datain);
	ram_1->din(dataout);
	ram_1->memrw(memrw);

	sc_trace_file *tfile=sc_create_vcd_trace_file("results");

	sc_trace(tfile,clk,"clk") ;
	sc_trace(tfile,reset,"reset") ;
	sc_trace(tfile,address,"address") ;
	sc_trace(tfile,datain,"datain") ;
	sc_trace(tfile,dataout,"dataout") ;
	sc_trace(tfile,cpu_1->pcout,"cpu_1.pcout") ;
	sc_trace(tfile,cpu_1->fsm_1->state,"cpu_1.fsm_1.state") ;

	sc_trace(tfile,cpu_1->rf32_1->regFile[00],"cpu_1.rf32_1.reg(0)") ;
	sc_trace(tfile,cpu_1->rf32_1->regFile[01],"cpu_1.rf32_1.reg(1)") ;
	sc_trace(tfile,cpu_1->rf32_1->regFile[02],"cpu_1.rf32_1.reg(2)") ;
	sc_trace(tfile,cpu_1->rf32_1->regFile[03],"cpu_1.rf32_1.reg(3)") ;
	sc_trace(tfile,cpu_1->rf32_1->regFile[04],"cpu_1.rf32_1.reg(4)") ;
	sc_trace(tfile,cpu_1->rf32_1->regFile[05],"cpu_1.rf32_1.reg(5)") ;
	sc_trace(tfile,cpu_1->rf32_1->regFile[06],"cpu_1.rf32_1.reg(6)") ;
	sc_trace(tfile,cpu_1->rf32_1->regFile[07],"cpu_1.rf32_1.reg(7)") ;

	sc_trace(tfile,cpu_1->cst0out,"cst0out");
	sc_trace(tfile,cpu_1->cst4out,"cst4out");
	sc_trace(tfile,cpu_1->pcout,"pcout");
	sc_trace(tfile,cpu_1->rfout,"rfout");
	sc_trace(tfile,cpu_1->adout,"adout");
	sc_trace(tfile,cpu_1->dtout,"dtout");
	sc_trace(tfile,cpu_1->irsextout,"irsextout");
	sc_trace(tfile,cpu_1->irextout,"irextout");
	sc_trace(tfile,cpu_1->irshiftout,"irshiftout");
	sc_trace(tfile,cpu_1->mux_x_out,"mux_x_out");
	sc_trace(tfile,cpu_1->mux_y_out,"mux_y_out");
	sc_trace(tfile,cpu_1->int_dataout,"int_dataout");
	sc_trace(tfile,cpu_1->irout15_0,"irout15_0");
	sc_trace(tfile,cpu_1->irout15_11,"irout15_11");
	sc_trace(tfile,cpu_1->irout20_16,"irout20_16");
	sc_trace(tfile,cpu_1->irout25_21,"irout25_21");

	sc_trace(tfile,cpu_1->write_pc,"write_pc");;
	sc_trace(tfile,cpu_1->mux_rf_w,"mux_rf_w");;
	sc_trace(tfile,cpu_1->mux_rf_r,"mux_rf_r");;
	sc_trace(tfile,cpu_1->write_rf,"write_rf");;
	sc_trace(tfile,cpu_1->adr_rf_w,"adr_rf_w");;
	sc_trace(tfile,cpu_1->adr_rf_r,"adr_rf_r");;
	sc_trace(tfile,cpu_1->write_ad,"write_ad");;
	sc_trace(tfile,cpu_1->write_dt,"write_dt");;
	sc_trace(tfile,cpu_1->write_ir,"write_ir");;
	sc_trace(tfile,cpu_1->mux_x,"mux_x");;
	sc_trace(tfile,cpu_1->mux_y,"mux_y");;
	sc_trace(tfile,cpu_1->mux_addr,"mux_addr");;
	sc_trace(tfile,cpu_1->alu_op,"amu_op");;
	sc_trace(tfile,cpu_1->zero,"zero");;
	sc_trace(tfile,cpu_1->ir,"ir");;

	sc_start(0, SC_NS);
	reset=0;
	sc_start(1, SC_NS);
	reset=1;
	for (int i=0;i<100;i++)
	{
		sc_start(1, SC_NS);
	}

	sc_close_vcd_trace_file(tfile);

	return 0;
}
