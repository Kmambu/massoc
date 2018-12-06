#ifndef NOISE_H
#define NOISE_H

#define K 1.3806504e-23
#define T 293
double randu()
{
  return (double)rand()/RAND_MAX;// RAND_MAX = 32767
}
double randn()
{
  double res=randu();
  if(res)
    return sqrt(-2*log(res))*cos(2*M_PI*randu()) ;
  return randn();
}
#endif
