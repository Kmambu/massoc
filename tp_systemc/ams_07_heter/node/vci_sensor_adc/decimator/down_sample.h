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
#ifndef DOWN_SAMPLE_H
#define DOWN_SAMPLE_H

template <
    int rate >

SCA_TDF_MODULE (down_sample)
{
    sca_tdf::sca_in < double >in;
    sca_tdf::sca_out < double >out;

    void set_attributes() {
        in.set_rate(rate);
        out.set_rate(1);
    }

    void processing () {
        out.write(in.read());
    }

    SCA_CTOR (down_sample) {}
};
#endif
