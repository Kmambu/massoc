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
#ifndef AVG_FIR2_H
#define AVG_FIR2_H

SCA_TDF_MODULE (avg_fir2)
{
  sca_tdf::sca_in < double >in;
  sca_tdf::sca_out < double >out;

  double in_rate;
  double buf;

  void init (int r) {
    in_rate = r;
  }

  void set_attributes() {
    in.set_rate(in_rate);
    out.set_rate(1);
  }

  void processing () {
    double input=in.read()/2;
    out.write(buf+input);
    buf=input;
  }

  SCA_CTOR (avg_fir2) {
    buf=0;
    in_rate=1;
  }
};
#endif
