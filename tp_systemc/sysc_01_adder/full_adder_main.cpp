// File : full_adder_main.cpp
#include "driver.h"
#include "monitor.h"
#include "full_adder.h"

int sc_main(int argc,char *argv[])
{
	sc_signal<bool> t_a("a"), t_b("b"), t_cin("cin"), t_sum("sum"), t_cout("cout");

	full_adder f1("FullAdderWithHalfAdder");
	// Connect using positional association:
	f1 << t_a << t_b << t_cin << t_sum << t_cout;

	driver d1("GenerateWaveforms");
	// Connect using named association:
	d1.d_a(t_a);
	d1.d_b(t_b);
	d1.d_cin(t_cin);

	monitor mo1("MonitorWaveforms");
	mo1 << t_a << t_b << t_cin << t_sum << t_cout;

	sc_trace_file* tfp = sc_create_vcd_trace_file("full_adder_main");
	sc_trace(tfp, t_a, t_a.name());
        sc_trace(tfp, t_b, t_b.name());
        sc_trace(tfp, t_cin, t_cin.name());
        sc_trace(tfp, t_sum, t_sum.name());
        sc_trace(tfp, t_cout, t_cout.name());
	sc_start(100,SC_NS);
	sc_close_vcd_trace_file(tfp);

	return 0;
}
