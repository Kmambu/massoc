SCA_TDF_MODULE (analyser_ber)
{
    sca_tdf::sca_in < bool >in1;
    sca_tdf::sca_in < bool >in2;
    sca_tdf::sca_out < bool >out;

    int rate;
    double accu;
    FILE *i_file;
    unsigned int count;
    
    void processing () {
      count++;
      out.write(in1.read()^in2.read());
      if (in1.read()^in2.read()) accu++;
      if ((count+1)%rate==0){
        fprintf(i_file,"%e\n",accu/rate);
        fflush(i_file);
        accu=0;
      }
    }
    void init(int rate){
      this->rate=rate;
      this->accu=0;
      i_file=fopen("./trace/ber.dat","w");
      count=0;
    }
    void set_rate(int rate){
      this->rate=rate;
    }
    SCA_CTOR (analyser_ber) {}
    ~analyser_ber(){
      fclose(i_file);
    }
};
