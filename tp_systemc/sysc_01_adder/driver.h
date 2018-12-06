// File : driver.h
#ifndef DRIVER_H
#define DRIVER_H

#include <systemc.h>

SC_MODULE(driver)
{
	sc_out<bool> d_a,d_b,d_cin;

	void prc_driver();

	SC_CTOR(driver) : d_a("d_a"),d_b("d_b"),d_cin("d_cin")
	{
		SC_THREAD(prc_driver);
	}
};

#endif // DRIVER_H
