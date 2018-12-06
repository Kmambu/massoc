      //***************************************************************************
      //                         File : ./rf.h
      //
      // Date       : Mon Jul 11 23:05:11 CEST 2011
      //
      // Language   : SystemC AMS 1.0Beta2
      //
      // Authors    : Michel Vasilevski     : michel.vasilevski@lip6.fr
      //            : Hassan Aboushady      : hassan.aboushady@lip6.fr
      //
      // Laboratory : LIP6/SOC, University Pierre and Marie Curie, Paris 6
      //
      //***************************************************************************
#ifndef RF_H
#define RF_H
#define SCA_SDF_RF_MODULE(name) struct name : public sca_sdf_rf_module
#define IDEAL_R 1

#include <vector>
#include <limits>
template<
class C
>
class sca_sdf_rf_in : public sca_tdf::sca_in<C>{
  public:
    double R;
    sca_sdf_rf_in():sca_tdf::sca_in<C>(){
      R=50;
    }
};
template<
class C
>
class sca_sdf_rf_out : public sca_tdf::sca_out<C>{
  public:
    double R;
    sca_sdf_rf_out():sca_tdf::sca_out<C>(){
      R=50;
    }
};
class sca_sdf_rf_module : public sca_sdf_module{
  public:
    double get_rsource(){
      double rsource;
      sc_interface *m_if;
      sca_channel *m_sca_ch;
      sca_port_base **lport;
      unsigned long nport;
      sca_sdf_rf_in<double> *i_port;

      for(sca_module::sca_port_base_list_iteratorT pit=
          get_port_list().begin();
          pit!=get_port_list().end(); ++pit
         )
        if((*pit)->get_port_type()==sca_port_base::SCA_IN_PORT){
          m_if = (*pit)->sc_get_interface();
          m_sca_ch = dynamic_cast<sca_channel*>(m_if);
          lport=m_sca_ch->get_connected_port_list();
          nport=m_sca_ch->get_number_of_connected_ports();
          for (unsigned long i=0;i<nport;i++){
            i_port=(sca_sdf_rf_in<double> *)lport[i];
            if (i_port->get_port_type()==sca_port_base::SCA_OUT_PORT)
              rsource=i_port->R;
          }
        }
      return rsource;
    }
    double get_rload(){
      double rload=std::numeric_limits<double>::infinity();
      std::vector<double> rlist;
      sc_interface *m_if;
      sca_channel *m_sca_ch;
      sca_port_base **lport;
      unsigned long nport;
      sca_sdf_rf_in<double> *i_port;

      for(sca_module::sca_port_base_list_iteratorT pit=
          get_port_list().begin();
          pit!=get_port_list().end(); ++pit
         )
        if((*pit)->get_port_type()==sca_port_base::SCA_OUT_PORT){
          m_if = (*pit)->sc_get_interface();
          m_sca_ch = dynamic_cast<sca_channel*>(m_if);
          lport=m_sca_ch->get_connected_port_list();
          nport=m_sca_ch->get_number_of_connected_ports();
          for (unsigned long i=0;i<nport;i++){
            i_port=(sca_sdf_rf_in<double> *)lport[i];
            if (i_port->get_port_type()==sca_port_base::SCA_IN_PORT)
              rlist.push_back(i_port->R);
          }
          for(std::vector<double>::iterator It=rlist.begin();It!=rlist.end();It++)
            rload=1/(1/(*It)+1/rload);
        }
      return rload;
    }
};
#endif
