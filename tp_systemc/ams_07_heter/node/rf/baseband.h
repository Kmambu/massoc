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
      //                         Fichier : ./baseband.h
      //                                                                           
      // Language   : C++     Version : 1.0    Date : Mon Jun 9 14:35:12 CEST 2008
      //
      // Author     : Michel Vasilevski
      // 
      // Laboratory : LIP6/SOC, University Pierre and Marie Curie, Paris 6
      //                                                                           
      //***************************************************************************
#ifndef BASEBAND_H
#define BASEBAND_H
#include <iostream>
#include "systemc-ams.h"

#include "constant.h"
class BB{
  public: 
    double DC,I1,I2,I3,Q1,Q2,Q3,w;
    BB(double DC, double I1, double I2, double I3, double Q1, double Q2, double Q3, double w){
      if (w>0){
        this->DC=DC;
        this->I1=I1;
        this->I2=I2;
        this->I3=I3;
        this->Q1=Q1;
        this->Q2=Q2;
        this->Q3=Q3;
        this->w=w;
      }else{
        this->DC=DC+I1+I2+I3+Q1+Q2+Q3;
        this->I1=0;
        this->I2=0;
        this->I3=0;
        this->Q1=0;
        this->Q2=0;
        this->Q3=0;
        this->w=0;
      }
    }
    BB(){
      this->DC=0;
      this->I1=0;
      this->I2=0;
      this->I3=0;
      this->Q1=0;
      this->Q2=0;
      this->Q3=0;
      this->w=0;
    }
    BB(double x){
      this->DC=x;
      this->I1=0;
      this->I2=0;
      this->I3=0;
      this->Q1=0;
      this->Q2=0;
      this->Q3=0;
      this->w=0;
    }
    BB(double x,double y,double w){
      if (w>0){
        this->DC=x;
        this->I1=y;
        this->I2=0;
        this->I3=0;
        this->Q1=0;
        this->Q2=0;
        this->Q3=0;
        this->w=w;
      }else{
        this->DC=x+y;
        this->I1=0;
        this->I2=0;
        this->I3=0;
        this->Q1=0;
        this->Q2=0;
        this->Q3=0;
        this->w=0;
      }
    }
    double toPB() const{
      double time=current_time;
      return DC+I1*cos(w*time)+I2*cos(2*w*time)+I3*cos(3*w*time)+Q1*sin(w*time)+Q2*sin(2*w*time)+Q3*sin(3*w*time);
    }
    BB operator/ (double x) const{
      BB y(DC/x,I1/x,I2/x,I3/x,Q1/x,Q2/x,Q3/x,w);
      return y;
    }
    BB operator* (double x) const{
      BB y(DC*x,I1*x,I2*x,I3*x,Q1*x,Q2*x,Q3*x,w);
      return y;
    }
    BB operator* (BB x) const{
      double zw=0;
      BB y=*this;
      if (w==0||x.w==0){
        zw=w+x.w;
      }else if(w>x.w){
        zw=x.w;
        y=translate(x.w);
      }else if(x.w>w){
        zw=w;
        x=x.translate(w);
      }else{
        zw=w;
      }
      BB z(
          y.DC*x.DC+y.I1*x.I1/2+y.I2*x.I2/2+y.I3*x.I3/2+y.Q1*x.Q1/2+y.Q2*x.Q2/2+y.Q3*x.Q3/2,
          y.I1*x.DC+y.DC*x.I1+y.I2*x.I1/2+y.I1*x.I2/2+y.I3*x.I2/2+y.I2*x.I3/2+y.Q2*x.Q1/2+y.Q1*x.Q2/2+y.Q3*x.Q2/2+y.Q2*x.Q3/2,
          y.I2*x.DC+y.I1*x.I1/2+y.I3*x.I1/2+y.DC*x.I2+y.I1*x.I3/2-y.Q1*x.Q1/2+y.Q3*x.Q1/2+y.Q1*x.Q3/2,
          y.I3*x.DC+y.I2*x.I1/2+y.I1*x.I2/2+y.DC*x.I3-y.Q2*x.Q1/2-y.Q1*x.Q2/2,
          y.Q1*x.DC+y.Q2*x.I1/2-y.Q1*x.I2/2+y.Q3*x.I2/2-y.Q2*x.I3/2+y.DC*x.Q1-y.I2*x.Q1/2+y.I1*x.Q2/2-y.I3*x.Q2/2+y.I2*x.Q3/2,
          y.Q2*x.DC+y.Q1*x.I1/2+y.Q3*x.I1/2-y.Q1*x.I3/2+y.I1*x.Q1/2-y.I3*x.Q1/2+y.DC*x.Q2+y.I1*x.Q3/2,
          y.Q3*x.DC+y.Q2*x.I1/2+y.Q1*x.I2/2+y.I2*x.Q1/2+y.I1*x.Q2/2+y.DC*x.Q3,
          zw
          );
      return z;
    }
    BB operator+ (BB x) const{
      double zw=0;
      BB y=*this;
      if (w==0||x.w==0){
        zw=w+x.w;
      }else if(w>x.w){
        zw=x.w;
        y=translate(x.w);
      }else if(x.w>w){
        zw=w;
        x=x.translate(w);
      }else{
        zw=w;
      }
      BB z(
          y.DC+x.DC,
          y.I1+x.I1,
          y.I2+x.I2,
          y.I3+x.I3,
          y.Q1+x.Q1,
          y.Q2+x.Q2,
          y.Q3+x.Q3,
          zw
          );
      return z;
    }
    BB operator- (BB x) const{
      double zw=0;
      BB y=*this;
      if (w==0||x.w==0){
        zw=w+x.w;
      }else if(w>x.w){
        zw=x.w;
        y=translate(x.w);
      }else if(x.w>w){
        zw=w;
        x=x.translate(w);
      }else{
        zw=w;
      }
      BB z(
          y.DC-x.DC,
          y.I1-x.I1,
          y.I2-x.I2,
          y.I3-x.I3,
          y.Q1-x.Q1,
          y.Q2-x.Q2,
          y.Q3-x.Q3,
          w
          );
      return z;
    }
    BB translate(double w) const{
      double epsilon=this->w-w;
      double t=current_time;
      BB y(
          this->DC,
          this->I1*cos(epsilon*t)+this->Q1*sin(epsilon*t),
          this->I2*cos(2*epsilon*t)+this->Q2*sin(2*epsilon*t),
          this->I3*cos(3*epsilon*t)+this->Q3*sin(3*epsilon*t),
          this->Q1*cos(epsilon*t)-this->I1*sin(epsilon*t),
          this->Q2*cos(2*epsilon*t)-this->I2*sin(2*epsilon*t),
          this->Q3*cos(3*epsilon*t)-this->I3*sin(3*epsilon*t),
          w
          );
      return y;
    }
};
std::ostream &operator<<(std::ostream &f, BB bb)
{
  return f << bb.DC << " " << bb.I1 << " " << bb.I2 << " " << bb.I3 << " " << bb.Q1 << " " << bb.Q2 << " " << bb.Q3 << " " << bb.w << " ";
}
const BB operator* (double x,BB bb){
  BB y(x*bb.DC,x*bb.I1,x*bb.I2,x*bb.I3,x*bb.Q1,x*bb.Q2,x*bb.Q3,bb.w);
  return y;
}
BB pow(const BB& x,int y){
  BB z=1;
  while (y-->0){
    z=z*x;
  }
  return z;
}
#endif
