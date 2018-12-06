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
#ifndef AVG3_FIR2_H
#define AVG3_FIR2_H
#include "decimator/avg_fir2.h"

SC_MODULE (avg3_fir2)
{
  sca_tdf::sca_in < double >in;
  sca_tdf::sca_out < double >out;

  sca_tdf::sca_signal < double >sig_avg_fir2_1;
  sca_tdf::sca_signal < double >sig_avg_fir2_2;

  avg_fir2 *i_avg_fir2_1;
  avg_fir2 *i_avg_fir2_2;
  avg_fir2 *i_avg_fir2_3;

  void init (int r) {
    i_avg_fir2_1->init (r);
  }

  SC_CTOR (avg3_fir2) {
    i_avg_fir2_1 = new avg_fir2("avg1");
    i_avg_fir2_1->in (in);
    i_avg_fir2_1->out (sig_avg_fir2_1);

    i_avg_fir2_2 = new avg_fir2("avg2");
    i_avg_fir2_2->in (sig_avg_fir2_1);
    i_avg_fir2_2->out (sig_avg_fir2_2);

    i_avg_fir2_3 = new avg_fir2("avg3");
    i_avg_fir2_3->in (sig_avg_fir2_2);
    i_avg_fir2_3->out (out);
  }
};
#endif
