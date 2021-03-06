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

  pwm.h --

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

#ifndef PWM_H
#define PWM_H

#include <cmath>
#include <systemc-ams>

SCA_TDF_MODULE(pwm)
{
  sca_tdf::sca_in<double> in;
  sca_tdf::sca_out<double> out;

  pwm(sc_core::sc_module_name nm, double v0_ = 0.0, double v1_ = 1.0,
      const sca_core::sca_time& t_period_ = sca_core::sca_time(5.0, sc_core::SC_MS),
      const sca_core::sca_time& t_ramp_ = sca_core::sca_time(0.05, sc_core::SC_MS),
      const sca_core::sca_time& t_step_ = sca_core::sca_time(0.01, sc_core::SC_MS) )
  : in("in"), out("out"), v0(v0_), v1(v1_), t_period( t_period_.to_seconds() ),
    t_ramp( t_ramp_.to_seconds() ), t_duty_max( t_period - 2.0 * t_ramp ),
    t_duty( t_duty_max ), t_step( t_step_ ), last_period(0), activations(0) {}

  void set_attributes()
  {
    set_timestep( t_step ); // fixed time step for module activation
  }

  void processing()
  {
    double t = get_time().to_seconds(); // current time
    double t_pos = std::fmod( t, t_period ); // time position inside pulse period
    long current_period = t / t_period + 1;

    if (current_period != last_period)  // calculate and clamp duty time
    {
        t_duty = in.read() * t_duty_max;
        if ( t_duty < 0.0 ) t_duty = 0.0;
        if ( t_duty > t_duty_max ) t_duty = t_duty_max;
    }
    last_period = current_period;
    ++activations;

    double val = v0; // initial value

    if ( t_pos < t_ramp ) // rising edge
      val = ( (v1 - v0) / t_ramp ) * t_pos + v0;
    else if ( t_pos < t_ramp + t_duty ) // plateau
      val = v1;
    else if ( t_pos < t_ramp + t_duty + t_ramp ) // falling edge
      val = ( (v0 - v1) / t_ramp ) * ( t_pos - t_ramp - t_duty ) + v1;
    // else return to initial value

    out.write(val);
  }

  void end_of_simulation()
  {
    std::cout << this->name() << ".end_of_simulation() @ " << this->get_time() << std::endl
              << "  activations = " << activations << std::endl
              << "  periods = " << last_period << std::endl
              << "  activations / period = " << static_cast<double>(activations) / last_period << std::endl;
  }

 private:
  const double v0, v1;             // initial and plateau values
  const double t_period, t_ramp;   // pulse period and ramp time
  const double t_duty_max;         // maximum duty time
  double t_duty;                   // current duty time
  sca_core::sca_time t_step;       // module time step
  long last_period;                // last period the module was activated
  long activations;                // number of processing() activations
};

#endif // PWM_H

/*
 * Local Variables:
 * mode: C++
 * End:
 */
