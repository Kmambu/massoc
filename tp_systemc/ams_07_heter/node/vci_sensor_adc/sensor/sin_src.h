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
      //***************************************************************************
      //                         Fichier : ./sin_src.h
      //                                                                           
      // Language   : C++     Version : 1.0    Date : Fri Jul 18 19:02:54 CEST 2008
      //
      // Author     : Michel Vasilevski
      // 
      // Laboratory : LIP6/SOC, University Pierre and Marie Curie, Paris 6
      //                                                                           
      //***************************************************************************
#ifndef SIN_SRC_H
#define SIN_SRC_H

SCA_SDF_MODULE (sin_src)
{
    sca_tdf::sca_out < double >out;

    double amp;
    double freq;

    void set_freq (double f) { freq = f; }
    void set_amp (double a) { amp = a; }
    void init (double a, double f)
    {
      amp=a;
      freq=f;
    }
    void init (double a, double f, double fsimu)
    {
      amp=a;
      freq=f;
      out.set_T(1/fsimu,SC_SEC);
    }
    void processing ()
    {
        out.write (amp * sin (2.0 * M_PI * freq * get_time().to_seconds()));
    }
    SCA_CTOR (sin_src) {}
};
#endif
