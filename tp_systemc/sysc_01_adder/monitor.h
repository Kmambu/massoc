// File : monitor.h
#ifndef MONITOR_H
#define MONITOR_H

#include "systemc.h"

SC_MODULE(monitor)
{
	sc_in<bool> m_a,m_b,m_cin,m_sum,m_cout;

	void prc_monitor();

	SC_CTOR(monitor) : m_a("m_a"),m_b("m_b"),m_cin("m_cin"),
		m_sum("m_sum"),m_cout("m_cout")
	{
		SC_METHOD(prc_monitor);
		sensitive << m_a << m_b << m_cin << m_sum << m_cout;
	}
};

#endif // MONITOR_H
