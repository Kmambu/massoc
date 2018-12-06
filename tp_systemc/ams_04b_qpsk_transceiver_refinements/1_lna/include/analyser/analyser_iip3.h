#ifndef ANALYSER_IIP3_H
#define ANALYSER_IIP3_H

#include "spectrum.h"

SCA_TDF_MODULE (analyser_iip3)
{
  sca_tdf::sca_in < double >in;

  unsigned int N;
  double f1;
  double f2;
  double AindB;
  double *a1;
  double *a3;

  double fs;

  double *intab;

  unsigned int count;
  
  fftw_complex *outtab;

  double IIP3theo;
  double IIP3calc;
  double IIP3graph;

  void set_amp(double AindB){
    this->AindB=AindB;
  }
  void init(double fs,unsigned int N,double f1,double f2, double AindB, double *a1, double *a3){
    this->fs=fs;
    this->N = N;
    this->intab = (double *)fftw_malloc( N * sizeof(double));
    this->f1=f1;
    this->f2=f2;
    this->AindB=AindB;
    this->a1=a1;
    this->a3=a3;
    count=0;
  }
  void set_attributes(){
    in.set_timestep(sc_time(1/fs,SC_SEC));
  }
  void processing () {
    count++;
    intab[count%N] = in.read();
    cout << count%N << endl;

    if (count%N==N-1){
      // Blackman-Harris
      for (unsigned int i = 0; i < N; i++) {
        intab[i] *= 0.35875 - 0.48829 * cos(2 * M_PI * i /(N-1)) + 0.14128 * cos(4 * M_PI * i/(N-1)) - 0.01168 * cos(6 * M_PI * i/(N-1));
      }
      //
      outtab = fft(intab,N);

      FILE *i_file_2=fopen("./trace/fft_sc.dat","w");
      show_fftw_complex(outtab,N/2,i_file_2,fs/N);
      fclose(i_file_2);

      int s1_i=(int)(f1*N/fs);
      int s3_i=(int)((2*f1-f2)*N/fs);
      
      double A1=pow(10,AindB/20);
      double s1=(*a1)*A1;
      double s3=(3*(*a3)*pow(A1,3)/4);

      double s1dB=db(absolute(outtab[s1_i],N/2));
      double s3dB=db(absolute(outtab[s3_i],N/2));

      IIP3theo=dbm(sqrt(4*fabs(*a1/(*a3))/3));
      IIP3calc=dbm(A1)+(db(s1)-db(s3))/2;
      IIP3graph=dbm(A1)+(s1dB-s3dB)/2;

      fftw_free(outtab);
      this->intab = (double *)fftw_malloc( N * sizeof(double));  // was deleted by fft()
    }
  }
  void end_of_simulation() {
      printf("IIP3 :\nTheorical: %f\nComputed: %f\nGraphical: %f\n\n", IIP3theo, IIP3calc, IIP3graph);
  }
  SCA_CTOR (analyser_iip3)
  : in("in"), N(), f1(), f2(), AindB(), a1(0), a3(0), fs(), intab(0),
    count(0), outtab(0), IIP3theo(), IIP3calc(), IIP3graph()
  {}

  ~analyser_iip3(){
    fftw_free(intab);
  }
};
#endif
