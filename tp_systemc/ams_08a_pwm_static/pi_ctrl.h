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

  pi_ctrl.h --

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

#ifndef PI_CTRL_H
#define PI_CTRL_H

#include <systemc-ams>

#ifdef USE_LSF_FOR_PI_CTRL

SC_MODULE(pi_ctrl)
{
  sca_tdf::sca_in<double> in;
  sca_tdf::sca_out<double> out;

  pi_ctrl( sc_core::sc_module_name, double kp, double ki)
  : in("in"), out("out"), tdf2lsf("tdf2lsf"), gain1("gain1", kp),
    integ1("integ1", ki), add1("add1"), lsf2tdf("lsf2tdf"),
    sig_p("sig_p"), sig_i("sig_i"), sig_pi("sig_pi"), sig_ref("sig_ref")
  {
    tdf2lsf.inp(in);
    tdf2lsf.y(sig_ref);

    gain1.x(sig_ref);
    gain1.y(sig_p);

    integ1.x(sig_ref);
    integ1.y(sig_i);

    add1.x1(sig_p);
    add1.x2(sig_i);
    add1.y(sig_pi);

    lsf2tdf.x(sig_pi);
    lsf2tdf.outp(out);
  }

private:
  sca_lsf::sca_tdf::sca_source tdf2lsf;
  sca_lsf::sca_gain gain1;
  sca_lsf::sca_integ integ1;
  sca_lsf::sca_add add1;
  sca_lsf::sca_tdf::sca_sink lsf2tdf;
  sca_lsf::sca_signal sig_p, sig_i, sig_pi, sig_ref;
};

#else // USE_LSF_FOR_PI_CTRL

// alternative implementation for the PI controller using LTF
SCA_TDF_MODULE(pi_ctrl)
{
  sca_tdf::sca_in<double> in;
  sca_tdf::sca_out<double> out;

  pi_ctrl( sc_core::sc_module_name, double kp_, double ki_)
  : in("in"), out("out"), kp(kp_), ki(ki_) {}

  void set_attributes()
  {
    accept_attribute_changes();
  }

  void initialize()
  {
    num(0)=ki;
    num(1)=kp;
    den(0)=0.0; // H(s) = kp + ki/s = kp*s/s + ki/s = (kp*s + ki ) / s + 0
    den(1)=1.0;
  }

  void processing()
  {
    double ltf_in = in.read();
    double ltf_out;

    ltf_out = ltf_nd( num, den, state, ltf_in );

    out.write(ltf_out);
  }

 private:
  sca_tdf::sca_ltf_nd ltf_nd;
  sca_util::sca_vector<double> num, den, state;
  double kp, ki;
};

#endif // USE_LSF_FOR_PI_CTRL

#endif // PI_CTRL_H

/*
 * Local Variables:
 * mode: C++
 * End:
 */
