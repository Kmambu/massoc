#ifndef COS_SRC_H
#define COS_SRC_H

SCA_TDF_MODULE (cos_src)
{
  sca_tdf::sca_out< double >out;

  double a1;

  double w;
  void processing (){
    out.write (a1 * cos (w * sca_get_time().to_seconds()));
  }
  void post_processing(){
    printf("cos_src :\n\tgain=%e\n",a1);
    using namespace std;
    cout << endl << sc_time_stamp() << " : " << out.get_time().to_seconds()/out.get_timestep().to_seconds()+1 << "  sc_main finished" << endl;
  }
  void init (double gain_db, double freq){ 
    this->a1=pow(10,gain_db/20);
    this->w=2.0*M_PI*freq;
  }
  SCA_CTOR (cos_src) {}
};
#endif
