/* -*- c++ -*-
 *
 * SOCLIB_LGPL_HEADER_BEGIN
 * 
 * This file is part of SoCLib, GNU LGPLv2.1.
 * 
 * SoCLib is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; version 2.1 of the License.
 * 
 * SoCLib is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with SoCLib; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 * 
 * SOCLIB_LGPL_HEADER_END
 *
 * Copyright (c) UPMC, Lip6, Asim
 */

#ifndef SEISMIC_SENSOR_H
#define SEISMIC_SENSOR_H

#include "systemc-ams.h"

SCA_SDF_MODULE (sensor)
{
  sca_tdf::sca_in < double  > in_sensor;

  uint32_t ident;
  char first;

  void processing () {
    double v=in_sensor.read();
    if ((fabs(v) > 0.08) && (first==1))
      {
	std::cout << "Sensor " << ident << " " << sca_get_time().to_seconds() << " " << v << std::endl;
	first=0;
      }
  }

  SCA_CTOR (sensor) :
    ident(0),
    first(1){
  }

  void set_ident(uint32_t _ident) {
    ident = _ident;
  }

};
#endif
