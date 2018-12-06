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

#ifndef WAVEGEN_H
#define WAVEGEN_H

#include "systemc-ams.h"
#include <math.h>
#include <stdio.h>
#include <iostream>

#define WAVE_SIZE 100
#define NB_SENSORS 4
#define WAVE_AMPL 0.56

SCA_TDF_MODULE (wavegen)
{
  sca_tdf::sca_out < double > out_sensor[NB_SENSORS];

  double wave[WAVE_SIZE][WAVE_SIZE][3];
  double cd;
  int pos_x_sensor[NB_SENSORS];
  int pos_y_sensor[NB_SENSORS];
  int pos_x_epicenter;
  int pos_y_epicenter;
  int iter;
  int sample;

  void processing () {
    int x,y;
    int c;

    for (x=1;x<WAVE_SIZE-1;x++)
    {
      for (y=1;y<WAVE_SIZE-1;y++)
      {
        wave[x][y][2]=2.0*wave[x][y][1]-wave[x][y][0]
          +cd*cd*(wave[x+1][y][1] + wave[x-1][y][1] + wave[x][y+1][1] +
		  wave[x][y-1][1] - 4.0*wave[x][y][1]);
      }
    }

    for (y=0;y<WAVE_SIZE;y++)
    {
      wave[0][y][2]=wave[2][y][2];
      wave[WAVE_SIZE-1][y][2]=wave[WAVE_SIZE-3][y][2];
    }

    for (x=0;x<WAVE_SIZE;x++)
    {
      wave[x][0][2]=wave[x][2][2];
      wave[x][WAVE_SIZE-1][2]=wave[x][WAVE_SIZE-3][2];
    }

    for (c=0 ; c < NB_SENSORS; c++)
    {
      out_sensor[c].write(wave[pos_x_sensor[c]][pos_y_sensor[c]][2]);
    }

    for (x=0;x<WAVE_SIZE;x++)
    {
      for (y=0;y<WAVE_SIZE;y++)
      {
        wave[x][y][0]=wave[x][y][1];
        wave[x][y][1]=wave[x][y][2];
      }
    }

    if (iter%sample==0)
    {
      char s[100];
      FILE *fout;
      sprintf(s,"iter%5.5d",iter);
      std::cout << s << " " << out_sensor[0].get_time().to_seconds() << std::endl;

      //printf("%s %d\n",s,out_sensor[0].get_time().to_seconds());
      fout=fopen(s,"w");
      for (x=0;x<WAVE_SIZE;x++)
      {
        for (y=0;y<WAVE_SIZE;y++)
        {
          fprintf(fout,"%d %d %f\n",x,y,wave[x][y][2]);
        }
        fprintf(fout,"\n");
      }
      fclose(fout);
    }

    iter++;
  }

  void init (int _x_epi, int _y_epi, sc_core::sc_time sample_time) {

    int x,y;
    int k;

    int radius=10;
    double DX=M_PI/radius;
    double DY=M_PI/radius;

    pos_x_epicenter= _x_epi % WAVE_SIZE;
    pos_y_epicenter= _y_epi % WAVE_SIZE;

    for (x=0;x<WAVE_SIZE;x++)
    {
      for (y=0;y<WAVE_SIZE;y++)
      {
        for (k=0;k<3;k++)
        {
          wave[x][y][k]=0.0;
        }
      }
    }

    cd=50e3*sample_time.to_seconds()/WAVE_SPEED;

    for (x=0;x<radius;x++)
    {
      for (y=0;y<radius;y++)
      {
        wave[pos_x_epicenter-radius/2+x][pos_y_epicenter-radius/2+y][0]=WAVE_AMPL*sin(x*DX)*sin(y*DY);
      }
    }

    for (x=1;x<WAVE_SIZE-1;x++)
    {
      for (y=1;y<WAVE_SIZE-1;y++)
      {
        wave[x][y][1]=wave[x][y][0]+(wave[x+1][y][0] + wave[x-1][y][0] + wave[x][y+1][0] + wave[x][y-1][0] - 4.0*wave[x][y][0])*cd;
      }
    }
    //    out_sensor[0].set_timestep(sample_time);
    this->set_timestep(sample_time);
  }

  void set_sensor_pos(int num, int x, int y) {
    pos_x_sensor[num % NB_SENSORS]= x % WAVE_SIZE;
    pos_y_sensor[num % NB_SENSORS]= y % WAVE_SIZE;
  }

  SCA_CTOR (wavegen)
  : iter(0), sample(1000)
  {}
};
#endif
