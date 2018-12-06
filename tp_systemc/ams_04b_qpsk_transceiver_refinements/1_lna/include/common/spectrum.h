#ifndef SPECTRUM_H
#define SPECTRUM_H

#include <fftw3.h>

double absolute(double *in, unsigned long size){
  return sqrt(in[0]*in[0]+in[1]*in[1])/size;
}

void show_double(double * in, unsigned long size, FILE *f_trace)
{
  unsigned long i;

  for( i = 0 ; i < size ; i++ )
  {
    fprintf(f_trace,"%d %e\n",(int)i, in[i]);
  }
}

void show_fftw_complex(fftw_complex * in, unsigned long size, FILE *f_trace, double ratio)
{
  int i;
  double val;

  for( i = 0 ; i < (int) size ; i++ )
  {
    val=absolute(in[i],size);
    fprintf(f_trace,"%e %e\n",i*ratio, val);
  }
}

fftw_complex *fft(double *in, int N){
  fftw_complex * out;
  fftw_plan p;

  out = (fftw_complex *)fftw_malloc( N * sizeof(fftw_complex));

  /* Initialisation du plan de calcul pour la fft */
  p = fftw_plan_dft_r2c_1d(N,in,out,FFTW_ESTIMATE|FFTW_DESTROY_INPUT);

  /* Calcule la fft */
  fftw_execute(p);

  fftw_destroy_plan(p);
  fftw_free(in);
  return out;
}
double db(double val){
  return 20*log10(fabs(val));
}

double dbm(double val){
  return 10*log10(pow(fabs(val)/(2*sqrt(2)),2)/50e-3);
}

double undb(double val){
  return pow(10,val/20);
}
double undbm(double val){
  return sqrt(50e-3*pow(10,val/10))*2*sqrt(2);
}
#endif
