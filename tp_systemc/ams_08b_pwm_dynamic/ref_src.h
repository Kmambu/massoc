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

  ref_src.h --

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

// constant reference source

#ifndef REF_SRC_H
#define REF_SRC_H

#include <systemc-ams>

SCA_TDF_MODULE(ref_src)
{
  sca_tdf::sca_out<double> out;

  ref_src( sc_core::sc_module_name nm, double value_= 1.0, sca_core::sca_time t_step_ = sca_core::sca_time(0.05, sc_core::SC_MS) )
  : out("out"), value(value_), t_step(t_step_)
  {}

  void set_attributes()
  {
    out.set_timestep(t_step);
    accept_attribute_changes();
  }

  void processing()
  {
    out.write(value);
  }

private:
  double value;
  sca_core::sca_time t_step;
};

#endif // REF_SRC_H

/*
 * Local Variables:
 * mode: C++
 * End:
 */
