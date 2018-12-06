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
#ifndef DESERIALIZE_H
#define DESERIALIZE_H

template <
    int resolution>

SCA_TDF_MODULE (deserialize)
{
    sca_tdf::sca_in < double >in;
    sca_tdf::sc_out < sc_uint<resolution> >out;

    double ampl;

    void init(double a){
        ampl=a;
    }
    void set_attributes() {
      // Attention: Added to restore causality when using TDF converter outports in multirate systems
      out.set_delay(1);
    }
    void processing () {
      double input=in.read();
      if (input>ampl)
        input=ampl;
      if (input<-ampl)
        input=-ampl;

      out.write((uint32_t)floor((0.5+input/(2*ampl))*(pow(2.0,resolution)-1)));
    }
    SCA_CTOR (deserialize) {}
};
#endif
