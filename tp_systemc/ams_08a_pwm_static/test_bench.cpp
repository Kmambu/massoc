/*****************************************************************************

  The following code is derived, directly or indirectly, from the SystemC
  source code Copyright (c) 1996-2013 by all Contributors.
  All Rights reserved.

  The contents of this file are subject to the restrictions and limitations
  set forth in the SystemC Open Source License Version 3.0 (the "License");
  You may not use this file except in compliance with such restrictions and
  limitations. You may obtain instructions on how to receive a copy of the
  License at http://www.systemc.org/. Software distributed by Contributors
  under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF
  ANY KIND, either express or implied. See the License for the specific
  language governing rights and limitations under the License.

 *****************************************************************************/

/*****************************************************************************

  test_bench.h --

  Original Authors: Martin Barnasconi, NXP Semiconductors
                    Torsten Maehne, UPMC

  Created on: 2011-05-16

  SVN Version       :  $Revision$
  SVN last checkin  :  $Date$
  SVN checkin by    :  $Author$
  SVN Id            :  $Id$

 *****************************************************************************/

/*****************************************************************************

  MODIFICATION LOG - modifiers, enter your name, affiliation, date and
  changes you are making here.

      Name, Affiliation, Date:
  Description of Modification:

 *****************************************************************************/

// TODO: improve parametrization of pi_ctrl1 and drv_motor1 -- especially the omega parameters (2 pi f).

#include <systemc-ams>

#include "pwm.h"
#include "pi_ctrl.h"
#include "diff.h"
#include "ref_src.h"
#include "drv_motor.h"

int sc_main(int argc, char* argv[])
{
  using sca_core::sca_time;
  sc_core::sc_set_time_resolution(1.0, sc_core::SC_FS);
  const sca_time t_stop = sca_time(0.2, sc_core::SC_SEC);

  // ref_src
  const double ref_val = 7.5;
  const sca_time t_step = sca_core::sca_time(0.01, sc_core::SC_MS);

  // pi_ctrl1
  const double kp = 1.0 / 15.0;       // original value: 2.0
  const double ki = 2.0 * M_PI * 2.0; // original value: 5.0

  // pwm1
  const double v0 = 0.0;
  const double v1 = 1.0;
  const sca_time t_period = sca_time(5.0, sc_core::SC_MS); // original value: 255 * 0.05 ms
  const sca_time t_ramp = sca_time(0.05, sc_core::SC_MS);  // original value: 0.05 ms

  // drv_motor1
  const double h0 = 15.0;                  // original value: 255.0
  const double omega0 = 2.0 * M_PI * 10.0; // original value: 20.0

  sca_tdf::sca_signal<double> iref, ierr, imeas, ictrl, vdrv;

  ref_src ref_src1("ref_src1", ref_val, t_step);
   ref_src1.out(iref);

  diff diff1("diff1");
   diff1.ref(iref);
   diff1.meas(imeas);
   diff1.err(ierr);

  pi_ctrl pi_ctrl1("pi_ctrl1", kp, ki);
   pi_ctrl1.in(ierr);
   pi_ctrl1.out(ictrl);

  pwm pwm1("pwm1", v0, v1, t_period, t_ramp, t_step);
   pwm1.in(ictrl);
   pwm1.out(vdrv);

  drv_motor drv_motor1("drv_motor1", h0, omega0);
   drv_motor1.in(vdrv);
   drv_motor1.out(imeas);

  sca_util::sca_trace_file* tfp_tab = sca_util::sca_create_tabular_trace_file("pwm_static");
  sca_util::sca_trace(tfp_tab, iref, "iref");
  sca_util::sca_trace(tfp_tab, imeas, "imeas");
  sca_util::sca_trace(tfp_tab, ierr, "ierr");
  sca_util::sca_trace(tfp_tab, ictrl, "ictrl");
  sca_util::sca_trace(tfp_tab, vdrv, "vdrv");

  sca_util::sca_trace_file* tfp_vcd = sca_util::sca_create_vcd_trace_file("pwm_static");
  sca_util::sca_trace(tfp_vcd, iref, "iref");
  sca_util::sca_trace(tfp_vcd, imeas, "imeas");
  sca_util::sca_trace(tfp_vcd, ierr, "ierr");
  sca_util::sca_trace(tfp_vcd, ictrl, "ictrl");
  sca_util::sca_trace(tfp_vcd, vdrv, "vdrv");

  sc_start(t_stop);

  sca_util::sca_close_tabular_trace_file(tfp_tab);
  sca_util::sca_close_vcd_trace_file(tfp_vcd);

  sc_core::sc_stop();

  return 0;
}
