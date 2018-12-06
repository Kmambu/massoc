#ifndef BASEBAND_H
#define BASEBAND_H
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
    BB operator/ (double x) const{
      BB y(DC/x,I1/x,I2/x,I3/x,Q1/x,Q2/x,Q3/x,w);
      return y;
    }
    BB operator* (double x) const{
      BB y(DC*x,I1*x,I2*x,I3*x,Q1*x,Q2*x,Q3*x,w);
      return y;
    }
    BB operator* (BB x) const{
      BB z(
          DC*x.DC+I1*x.I1/2+I2*x.I2/2+I3*x.I3/2+Q1*x.Q1/2+Q2*x.Q2/2+Q3*x.Q3/2,
          I1*x.DC+DC*x.I1+I2*x.I1/2+I1*x.I2/2+I3*x.I2/2+I2*x.I3/2+Q2*x.Q1/2+Q1*x.Q2/2+Q3*x.Q2/2+Q2*x.Q3/2,
          I2*x.DC+I1*x.I1/2+I3*x.I1/2+DC*x.I2+I1*x.I3/2-Q1*x.Q1/2+Q3*x.Q1/2+Q1*x.Q3/2,
          I3*x.DC+I2*x.I1/2+I1*x.I2/2+DC*x.I3-Q2*x.Q1/2-Q1*x.Q2/2,
          Q1*x.DC+Q2*x.I1/2-Q1*x.I2/2+Q3*x.I2/2-Q2*x.I3/2+DC*x.Q1-I2*x.Q1/2+I1*x.Q2/2-I3*x.Q2/2+I2*x.Q3/2,
          Q2*x.DC+Q1*x.I1/2+Q3*x.I1/2-Q1*x.I3/2+I1*x.Q1/2-I3*x.Q1/2+DC*x.Q2+I1*x.Q3/2,
          Q3*x.DC+Q2*x.I1/2+Q1*x.I2/2+I2*x.Q1/2+I1*x.Q2/2+DC*x.Q3,
          w>x.w?w:x.w
          );
      return z;
    }
    BB operator+ (BB x) const{
      BB z(
          DC+x.DC,
          I1+x.I1,
          I2+x.I2,
          I3+x.I3,
          Q1+x.Q1,
          Q2+x.Q2,
          Q3+x.Q3,
          w
          );
      return z;
    }
    BB operator- (BB x) const{
      BB z(
          DC-x.DC,
          I1-x.I1,
          I2-x.I2,
          I3-x.I3,
          Q1-x.Q1,
          Q2-x.Q2,
          Q3-x.Q3,
          w
          );
      return z;
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
