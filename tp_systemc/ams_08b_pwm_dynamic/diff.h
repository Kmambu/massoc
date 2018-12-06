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

  diff.h --

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

#ifndef DIFF_H
#define DIFF_H

#include <systemc-ams>

SCA_TDF_MODULE(diff)
{
  sca_tdf::sca_in<double> ref, meas;
  sca_tdf::sca_out<double> err;

  SCA_CTOR(diff)
  : ref("ref"), meas("meas"), err("err")
  {}

  void set_attributes()
  {
    err.set_delay(1); // essential - system has control loop
    // Enable DTDF, attention this will cause variations in the delay
    // introduced to break the algebraic loop.
    accept_attribute_changes();
  }

  void processing()
  {
    err.write( ref.read() - meas.read() );
  }
};

#endif // DIFF_H

/*
 * Local Variables:
 * mode: C++
 * End:
 */
