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
#ifndef DECIMATOR_FIR2_H
#define DECIMATOR_FIR2_H

#include "decimator/avg3_fir2.h"
#include "decimator/down_sample.h"
#include "decimator/deserialize.h"
template <
    int nb_bits
  >
SC_MODULE (decimator)
{
    sca_tdf::sca_in < double >in;
    sc_out < sc_uint<nb_bits> >out;

    sca_tdf::sca_signal < double >sig_avg3_fir2_1;
    sca_tdf::sca_signal < double >sig_avg3_fir2_2;
    sca_tdf::sca_signal < double >sig_avg3_fir2_3;
    sca_tdf::sca_signal < double >sig_avg3_fir2_4;
    sca_tdf::sca_signal < double >sig_avg3_fir2_5;
    sca_tdf::sca_signal < double >sig_down_sample_1;

    avg3_fir2 *i_avg3_fir2_1;
    avg3_fir2 *i_avg3_fir2_2;
    avg3_fir2 *i_avg3_fir2_3;
    avg3_fir2 *i_avg3_fir2_4;
    avg3_fir2 *i_avg3_fir2_5;

    down_sample <2> *i_down_sample_1;
    deserialize <nb_bits> *i_deserialize_1;

    void init (double ampl, int sim_rate) {
      i_avg3_fir2_1->init (sim_rate);
      i_avg3_fir2_2->init (2);
      i_avg3_fir2_3->init (2);
      i_avg3_fir2_4->init (2);
      i_avg3_fir2_5->init (2);
      i_deserialize_1->init(ampl);
    }

    SC_CTOR (decimator) {

      i_avg3_fir2_1 = new avg3_fir2("avg31");
      i_avg3_fir2_1->in (in);
      i_avg3_fir2_1->out (sig_avg3_fir2_1);

      i_avg3_fir2_2 = new avg3_fir2("avg32");
      i_avg3_fir2_2->in (sig_avg3_fir2_1);
      i_avg3_fir2_2->out (sig_avg3_fir2_2);

      i_avg3_fir2_3 = new avg3_fir2("avg33");
      i_avg3_fir2_3->in (sig_avg3_fir2_2);
      i_avg3_fir2_3->out (sig_avg3_fir2_3);

      i_avg3_fir2_4 = new avg3_fir2("avg34");
      i_avg3_fir2_4->in (sig_avg3_fir2_3);
      i_avg3_fir2_4->out (sig_avg3_fir2_4);

      i_avg3_fir2_5 = new avg3_fir2("avg35");
      i_avg3_fir2_5->in (sig_avg3_fir2_4);
      i_avg3_fir2_5->out (sig_avg3_fir2_5);

      i_down_sample_1 = new down_sample <2>("down_sample1");
      i_down_sample_1->in (sig_avg3_fir2_5);
      i_down_sample_1->out (sig_down_sample_1);

      i_deserialize_1 = new deserialize <nb_bits>("i_deserialize_1");
      i_deserialize_1->in (sig_down_sample_1);
      i_deserialize_1->out (out);
    }
};
#endif
