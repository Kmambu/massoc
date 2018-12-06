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
      //                         Fichier : ./view.h
      //                                                                           
      // Language   : C++     Version : 1.0    Date : Mon Jun 9 14:35:08 CEST 2008
      //
      // Author     : Michel Vasilevski
      // 
      // Laboratory : LIP6/SOC, University Pierre and Marie Curie, Paris 6
      //                                                                           
      //***************************************************************************
#ifndef VIEW_H
#define VIEW_H

#define SIGNAL 1
#define SIGNALDB 2
#define SIGNALDBM 3
#define SPECTRE 4

#include "systemc-ams.h"

class Display{
    FILE *file_trace;
    char *path;
    double ts;
    double bw;
    double N;
    double fin;
    int sim_rate;

  public:
    Display(char *path, char *filename){
      char file[100];
      this->path=path;
      sprintf(file,"%s/%s",path, filename);
      file_trace=fopen(file,"w");
      fprintf(file_trace,"close all\nclear all\n");
      fflush(file_trace);
    }
    Display(char *path, char *filename,double fs,double fin,int sim_rate,double N,double bw){
      char file[100];
      this->fin=fin;
      this->ts=1/fs;
      this->bw=bw;
      this->N=N;
      this->sim_rate=sim_rate;
      this->path=path;
      sprintf(file,"%s/%s",path, filename);
      file_trace=fopen(file,"w");
      fprintf(file_trace,"close all\nclear all\n");
      fflush(file_trace);
    }
    ~Display(){
      fclose(file_trace);
    }
    template <
      class C
      >
      void view(char *text, sca_sdf_signal <C> *sig, int type, int rate=1){
        char name[100];
        sprintf(name,TRACE_PATH"/%s.dat",text);
        fprintf(file_trace,"load '%s'\n",name);
        sca_trace_file *tf = sca_create_tabular_trace_file (name);
        sca_trace (tf, *sig, "");
        switch (type){
          case SIGNAL:
            fprintf(file_trace,"figure\nplot(%s(1:%d:length(%s),1),%s(:,2))\n",text,rate,text,text);
            fprintf(file_trace,"htitle=title('%s');\nset(htitle,'FontSize',20);\n",text);
            break;
          case SIGNALDB:
            fprintf(file_trace,"figure\nplot(%s(1:%d:length(%s),1),db(%s(:,2)))\n",text,rate,text,text);
            fprintf(file_trace,"htitle=title('%s');\nset(htitle,'FontSize',20);\n",text);
            break;
          case SIGNALDBM:
            fprintf(file_trace,"figure\nplot(%s(1:%d:length(%s),1),dbm(%s(:,2)))\n",text,rate,text,text);
            fprintf(file_trace,"htitle=title('%s');\nset(htitle,'FontSize',20);\n",text);
            break;
          case SPECTRE:
            fprintf(file_trace,"%s=%s(1:%d:length(%s),2);\n",text,text,rate,text);
            fprintf(file_trace,"Sp=SPEC(%s,length(%s));\n",text,text);
//            fprintf(file_trace,"snr=SNR(Sp,%f,5,%e)\n",sim_rate*N*ts*fin+1,sim_rate*N*ts*bw);
            fprintf(file_trace,"figure\nsemilogx([0:length(%s)/2-1]/%e,db(Sp))\n",text,sim_rate*N*ts);
//            fprintf(file_trace,"htitle=title(['%s,snr=',num2str(snr)]);\nset(htitle,'FontSize',20);\n",text);
            break;
        }

        fflush(file_trace);
      }
    void view(char *text, int type, int rate=1){
      char name[100];
      sprintf(name,TRACE_PATH"/%s.dat",text);
      fprintf(file_trace,"load '%s'\n",name);
      switch (type){
        case SIGNAL:
          fprintf(file_trace,"figure\nplot(%s(1:%d:length(%s),1),%s(:,2))\n",text,rate,text,text);
          fprintf(file_trace,"htitle=title('%s');\nset(htitle,'FontSize',20);\n",text);
          break;
        case SIGNALDB:
          fprintf(file_trace,"figure\nplot(%s(1:%d:length(%s),1),db(%s(:,2)))\n",text,rate,text,text);
          fprintf(file_trace,"htitle=title('%s');\nset(htitle,'FontSize',20);\n",text);
          break;
        case SIGNALDBM:
          fprintf(file_trace,"figure\nplot(%s(1:%d:length(%s),1),dbm(%s(:,2)))\n",text,rate,text,text);
          fprintf(file_trace,"htitle=title('%s');\nset(htitle,'FontSize',20);\n",text);
          break;
        case SPECTRE:
          fprintf(file_trace,"%s=%s(1:%d:length(%s),2);\n",text,text,rate,text);
          fprintf(file_trace,"Sp=SPEC(%s,length(%s));\n",text,text);
//          fprintf(file_trace,"snr=SNR(Sp,%f,5,%e)\n",sim_rate*N*ts*fin+1,sim_rate*N*ts*bw);
          fprintf(file_trace,"figure\nsemilogx([0:length(%s)/2-1]/%e,db(Sp))\n",text,sim_rate*N*ts);
//          fprintf(file_trace,"htitle=title(['%s,snr=',num2str(snr)]);\nset(htitle,'FontSize',20);\n",text);
          break;
      }
      fflush(file_trace);
    }
    template <
      class C
      >
      void view(char *text, sca_sdf_signal <C> *sig1, sca_sdf_signal <C> *sig2){
        char name[100];
        sprintf(name,TRACE_PATH"/%s.dat",text);
        fprintf(file_trace,"load '%s'\n",name);
        sca_trace_file *tf = sca_create_tabular_trace_file (name);
        sca_trace (tf, *sig1, "");
        sca_trace (tf, *sig2, "");
        fprintf(file_trace,"figure\nplot(%s(:,2),%s(:,3),'o')\ngrid\n",text,text);
        fprintf(file_trace,"htitle=title('%s');\nset(htitle,'FontSize',20);",text);
        fflush(file_trace);
      }

    void view(char *text){
      fprintf(file_trace,text);
      fflush(file_trace);
    }
    void set_fin(double fin){
      this->fin=fin;
    }
    void set_bw(double bw){
      this->bw=bw;
    }
};
#endif
