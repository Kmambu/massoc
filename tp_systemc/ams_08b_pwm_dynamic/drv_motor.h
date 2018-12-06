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

  drv_motor.h --

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

// TODO: review default values (especially omega0)
//
// TODO: Specify maximum time step based on omega0. The load is limiting the
//       maximum time step based on the omega0 parameter and an additional
//       oversampling parameter.
//       I assume that the ltf_zp functor can cope with the variable time step.

#ifndef DRV_MOTOR_H
#define DRV_MOTOR_H

#include <systemc-ams>

SCA_TDF_MODULE(drv_motor)
{
  sca_tdf::sca_in<double> in;
  sca_tdf::sca_out<double> out;

  drv_motor( sc_core::sc_module_name nm, double h0_ = 255.0, double omega0_ = 20.0 )
  : in("in"), out("out"), h0(h0_), omega0(omega0_),
    ltf_zp("ltf_zp"), poles(), zeros()
  {}

  void set_attributes()
  {
    accept_attribute_changes();
    //set_max_timestep(t_max_step);
  }

  void initialize()
  {
    // no zeros required
    poles(0) = sca_util::sca_complex( -omega0, 0.0 );
  }

  void processing()
  {
    double tmp = ltf_zp( zeros, poles, in.read(), h0 * omega0 );
    out.write(tmp);
  }

private:
  double h0, omega0;
  //sca_core::sca_time t_max_step;
  sca_tdf::sca_ltf_zp ltf_zp; // Laplace transfer function
  sca_util::sca_vector<sca_util::sca_complex > poles, zeros; // poles and zeros as complex values
};

#endif // DRV_MOTOR_H

/*
 * Local Variables:
 * mode: C++
 * End:
 */
