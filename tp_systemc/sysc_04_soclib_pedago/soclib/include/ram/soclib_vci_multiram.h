// -*- c++ -*-
//////////////////////////////////////////////////////////////////////////
// File : soclib_vci_multiram.h
// Author : Alain Greiner
// Date : 26/05/2004
// This program is released under the GNU Public License
// Copyright : UPMC-LIP6
//
// This component implements a VCI ADAVANCED, multi-segments SRAM.
// It uses the loadexec() function written by F.Petrot to load
// executable binary code in a given memory segment.
// 
// The VCI ADDRESS & DATA fiels must be 32 bits.
// The VCI ERROR field must be 1 bit.
//
// This component can contain up to 16 segments.
// Each segment is defined by a BASE address and a SIZE
// (the SIZE is a number of bytes)
// Both the BASE and SIZE must be multiple of 4 bytes.
// Each segment is implemented by a table of "int" dynamically
// allocated by the RAM constructor.
//
// This component can be used in a 2 level hierarchical interconnected system
// That is why it has got 2 VCI index : 1 for the upper level , 1 for the lower
//
// This component cheks address for segmentation violation,
// and can be used as a default target.
//
// The VCI BE fiels is used for byte and hafword write requests :
// The supported values for the BE field are 1111 / 0011 / 0001 . 
///////////////////////////////////////////////////////////////////////// 
// This component has four "generator" parameters
// - char 			*name   : instance name
// - int			vci_index   : VCI target index
// - SOCLIB_SEGMENT_TABLE	*segmap : pointer on the segment table
/////////////////////////////////////////////////////////////////////////
// This component has 8  "template" parameters :
// VCI_PARAM
//////////////////////////////////////////////////////////////////////////

#ifndef SOCLIB_VCI_MULTIRAM_H
#define SOCLIB_VCI_MULTIRAM_H

#include <systemc.h>
#include <cstdio>
#include "shared/soclib_vci_interfaces.h"
#include "shared/soclib_mapping_table.h"

#ifdef WATCH_RAM_CONTENT
#include <vector>
#include <fstream>
#endif

#include <iostream>

#define MAXSEG 			16


extern "C" void loadexec(void **mem, int *size, int *base, char *file, char ** sections);

#ifndef _ENDIANNESS
#define _ENDIANNESS
bool IsBigEndian()
{
   short int word = 0x0001;
   char *byte = (char *) &word;
   return(byte[0] ? false : true);
}
int swap_bytes (int LE)
{
	int BE;
	BE =   ((LE & 0xFF000000) >> 24 |
		(LE & 0x00FF0000) >> 8  |
		(LE & 0x0000FF00) << 8  |
		(LE & 0x000000FF) << 24 );
	return BE;
}
#endif
////////////////////////////////////////        
//	structure definition
////////////////////////////////////////

template <VCI_PARAM_DECLAR>

struct SOCLIB_VCI_MULTIRAM : sc_module {

#ifdef WATCH_RAM_CONTENT
  struct watched_segment_type {
    unsigned int begin_address;
    unsigned int end_address;
  };
#endif

// IO PORTS
	
sc_in<bool> 		CLK;
sc_in<bool> 		RESETN;
ADVANCED_VCI_TARGET<VCI_PARAM> 	VCI;

//  REGISTERS

sc_signal<int>		FIFO_STATE;
sc_signal<int>		FIFO_RDATA[2];
sc_signal<int>		FIFO_RERROR[2];
sc_signal<int>		FIFO_REOP[2];
sc_signal<int>		FIFO_RSRCID[2];
sc_signal<int>		FIFO_RTRDID[2];
sc_signal<int>		FIFO_RPKTID[2];

#ifdef WATCH_RAM_CONTENT
  std::vector<watched_segment_type> m_watched_segment_list;
  std::vector<std::ofstream*> m_outstream_list;
#endif

//  STRUCTURAL PARAMETERS

const char 		*NAME;			// instance name
int 			NB_SEG;			// segment number
int 			SIZE[MAXSEG];		// segment sizes
int 			BASE[MAXSEG];		// segment bases
const char		*SEGNAME[MAXSEG];	// segment names
int 	 		*RAM[MAXSEG];		// segment buffers

////////////////////////////////////////////////////
//	constructor
////////////////////////////////////////////////////

SC_HAS_PROCESS(SOCLIB_VCI_MULTIRAM);

SOCLIB_VCI_MULTIRAM (sc_module_name       insname ,  // instance name 
  		     unsigned int	  vci_index,     // vci target index
		     SOCLIB_MAPPING_TABLE &segtab)    // segment table
	: CLK("CLK"),
	  RESETN("RESETN"),
	  FIFO_STATE("FIFO_STATE")
{
#ifdef NONAME_RENAME
        char newname[100];
        for (int i=0; i<2; i++ ) {
                sprintf(newname, "FIFO_RDATA_%2.2d", i);
                FIFO_RDATA[i].rename(newname);
                sprintf(newname, "FIFO_RERROR_%2.2d", i);
                FIFO_RERROR[i].rename(newname);
                sprintf(newname, "FIFO_REOP_%2.2d", i);
                FIFO_REOP[i].rename(newname);
                sprintf(newname, "FIFO_RSRCID_%2.2d", i);
                FIFO_RSRCID[i].rename(newname);
                sprintf(newname, "FIFO_RTRDID_%2.2d", i);
                FIFO_RTRDID[i].rename(newname);
                sprintf(newname, "FIFO_RPKTID_%2.2d", i);
                FIFO_RPKTID[i].rename(newname);

		}
#endif

#ifdef MODELSIM
	//Name the vector signals in MODELSIM
	int i, w;
        char str[20];
	for (i=0; i<2; i++ ) {
		w = sprintf(str, "FIFO_RDATA[%d]", i);
		FIFO_RDATA[i].sc_bind_mti_obj_name(str);
	}
	for (i=0; i<2; i++ ) {
		w = sprintf(str, "FIFO_RERROR[%d]", i);
		FIFO_RERROR[i].sc_bind_mti_obj_name(str);
	}
	for (i=0; i<2; i++ ) {
		w = sprintf(str, "FIFO_REOP[%d]", i);
		FIFO_REOP[i].sc_bind_mti_obj_name(str);
	}
	for (i=0; i<2; i++ ) {
		w = sprintf(str, "FIFO_RSRCID[%d]", i);
		FIFO_RSRCID[i].sc_bind_mti_obj_name(str);
	}
	for (i=0; i<2; i++ ) {
		w = sprintf(str, "FIFO_RTRDID[%d]", i);
		FIFO_RTRDID[i].sc_bind_mti_obj_name(str);
	}
	for (i=0; i<2; i++ ) {
		w = sprintf(str, "FIFO_RPKTID[%d]", i);
		FIFO_RPKTID[i].sc_bind_mti_obj_name(str);
	}
#endif

  std::list<SEGMENT_TABLE_ENTRY> 		seglist; 
  std::list<SEGMENT_TABLE_ENTRY>::iterator 	iter;
  int 						base,size;

  SC_METHOD (transition);
  sensitive_pos << CLK;

  SC_METHOD (genMoore);
  sensitive_neg << CLK;

#if defined(SYSTEMCASS_SPECIFIC)
  SAVE_HANDLER(save_state);
#endif

  NAME = (const char*) insname;
  
// checking the VCI parameters

if (ADDRSIZE != 32) {
	printf(" Error in SOCLIB_VCI_MULTIRAM component : %s\n",NAME);
	printf(" the ADDRSIZE parameter\n");
	printf(" must be equal to 32\n");
	sc_stop(); 
}

if (CELLSIZE != 4) {
	printf(" Error in SOCLIB_VCI_MULTIRAM component : %s\n",NAME);
	printf(" the CELLSIZE parameter\n");
	printf(" must be equal to 4\n");
	sc_stop(); 
}

if (ERRSIZE != 1) {
	printf(" Error in SOCLIB_VCI_MULTIRAM component : %s\n",NAME);
	printf(" the ERRSIZE parameter\n");
	printf(" must be equal to 1\n");
	sc_stop(); 
}

if (SRCIDSIZE > 16) {
	printf(" Error in SOCLIB_VCI_MULTIRAM component : %s\n",NAME);
	printf(" the SRCIDSIZE parameter\n");
	printf(" cannot be larger than 16\n");
	sc_stop(); 
}

if (TRDIDSIZE > 32) {
	printf(" Error in SOCLIB_VCI_MULTIRAM component : %s\n",NAME);
	printf(" the TRDIDSIZE parameter\n");
	printf(" cannot be larger than 32\n");
	sc_stop(); 
}

if (PKTIDSIZE > 32) {
	printf(" Error in SOCLIB_VCI_MULTIRAM component : %s\n",NAME);
	printf(" the PKTIDSIZE parameter\n");
	printf(" cannot be larger than 32\n");
	sc_stop(); 
}

  // segments allocation
  
  NB_SEG  = 0;
  seglist = segtab.getSegmentList(vci_index);
  
  for (iter = seglist.begin() ; iter != seglist.end() ; ++iter) {
          base=(*iter).getBase();
          size=(*iter).getSize();
	  if(NB_SEG == MAXSEG) {
		  fprintf(stderr,"Error in component SOCLIB_VCI_MULTIRAM %s\n", NAME);
		  fprintf(stderr,"The number of segments cannot be larger than %d\n",MAXSEG);
		  sc_stop();
	  }
	  if((base & 0x00000003) != 0x0) {
		  fprintf(stderr,"Error in component SOCLIB_VCI_MULTIRAM %s\n", NAME);
		  fprintf(stderr,"The BASE address of '%s' segment must be word aligned\n", iter->getName());
		  sc_stop();
	  }
	  if((size & 0x00000003) != 0x0) {
		  fprintf(stderr,"Error in component SOCLIB_VCI_MULTIRAM %s\n", NAME);
		  fprintf(stderr,"The SIZE parameter of '%s' segment must be multiple of 4\n", iter->getName());
		  sc_stop();
	  }
	  SEGNAME[NB_SEG] = (*iter).getName(); 
	  SIZE[NB_SEG]    = size;
	  BASE[NB_SEG]    = base;
	  RAM[NB_SEG]     = new int[size >> 2];
	  NB_SEG = NB_SEG + 1;
  }  // end for

  printf("Successful Instanciation of SOCLIB_VCI_MULTIRAM : %s\n",NAME);
  for(int k = 0 ; k < NB_SEG ; k++) {
	  printf("segment name = %s\n", SEGNAME[k]);
  }

}; // end constructor

  ~SOCLIB_VCI_MULTIRAM() {
#ifdef WATCH_RAM_CONTENT
    for (unsigned int i=0; i<m_watched_segment_list.size(); ++i) {
      m_outstream_list[i]->close();
      delete(m_outstream_list[i]);
    }
#endif
  }

void add_watch_segment(unsigned int begin_address, unsigned int size) {
#ifdef WATCH_RAM_CONTENT
  watched_segment_type temp;
  char tmp[256];
  std::ofstream *stmp;

  temp.begin_address = begin_address;
  temp.end_address = begin_address+size;
  m_watched_segment_list.push_back(temp);

  sprintf(tmp,"watch_%s_%d",NAME,m_outstream_list.size());
  stmp = new std::ofstream(tmp);
  m_outstream_list.push_back(stmp);
#endif
}


//////////////////////////////////////////////////////////////////////////
//		initSegmentExec()
//	This method is used to load executable binary code 
//	in a given segment.
//	- char	*segname	: segment name
//	- char	*filename	: binary file pathname
//	- char  **sections	: pointer on a table of section names
//////////////////////////////////////////////////////////////////////////

int initSegmentExec(char *segname, char *filename, char **sections)
{
	int	index = -1;
	for(int i = 0 ; i < NB_SEG ; i++) {
		if (strcmp(SEGNAME[i],segname) == 0) index = i;
	}
	if (index == -1) {
		  printf("Error in component SOCLIB_VCI_MULTIRAM %s\n", NAME);
		  printf("The segment '%s' is not defined and cannot\n",segname);
		  printf("be initialized by method initSegmentExec()\n");
		  sc_stop();
	}
	void	*ptab; 
	int	size   = SIZE[index];
	int	offset = 0;
	loadexec(&ptab,&size,&offset,filename,sections);
	if (size > SIZE[index]) {
		  printf("Error in component SOCLIB_VCI_MULTIRAM %s\n", NAME);
		  printf("The size given to segment %s is too small\n",segname);
		  printf("The binary code requests %d bytes\n",size);
		  sc_stop();
	}
/*  	if (offset != BASE[index]) {  //Modifié par Ivan Miro. Permet placer dans une zone de code RAM dans une zone diferente
  		  printf("Error in component SOCLIB_VCI_MULTIRAM %s\n", NAME);
  		  printf("The base address given to segment %s\n", segname);
  		  printf("is not equal to the binary code offset: %d \n", offset);
  		  sc_stop();
  	}*/
	memcpy(RAM[index], ptab, size);
	free(ptab);

	if (IsBigEndian()) //swap bytes in the RAM for big endian host machines
	{
		for (unsigned int i=0; i<(size/sizeof(int)); ++i) {RAM[index][i]=swap_bytes(RAM[index][i]);}
	}
	return(size);
} // end initSegmentExec()

///////////////////////////////////////////////////////
//	rw_seg() method
//	The memory organisation is litle endian
///////////////////////////////////////////////////////        

int rw_seg(int* tab, int index, int wdata, int be, int cmd, int lsb)// corrige par jieming pour le decalage de wdata
{
	if ((cmd & 0x1) == 0x1) {	// read
		return(tab[index]);
	} else {		
		switch (be) {
			case 0x1 :  // write byte 
			if (lsb == 0) tab[index] = (tab[index] & 0xFFFFFF00) |  (wdata & 0x000000FF);
			if (lsb == 1) tab[index] = (tab[index] & 0xFFFF00FF) | ((wdata & 0x000000FF) << 8);
			if (lsb == 2) tab[index] = (tab[index] & 0xFF00FFFF) | ((wdata & 0x000000FF) << 16);
			if (lsb == 3) tab[index] = (tab[index] & 0x00FFFFFF) | ((wdata & 0x000000FF) << 24);
			break;

			case 0x3 :  // write half 
			if (lsb == 0) tab[index] = (tab[index] & 0xFFFF0000) |  (wdata & 0x0000FFFF);
			if (lsb == 1) tab[index] = (tab[index] & 0xFFFF0000) |  (wdata & 0x0000FFFF);
			if (lsb == 2) tab[index] = (tab[index] & 0x0000FFFF) | ((wdata & 0x0000FFFF) << 16);
			if (lsb == 3) tab[index] = (tab[index] & 0x0000FFFF) | ((wdata & 0x0000FFFF) << 16);
			break;

			case 0xf :  // write word
			tab[index] = wdata;
			break;

			default :
			printf("Error in SOCLIB_VCI_MULTIRAM %s\n", NAME);
			printf("illegal value of the VCI BE field : %0x\n", be);
			printf("the supported values are : 1111 / 0011 / 0001\n");
			break;
		} // end switch be
		return(0);
	} // end write
} // end rw_seg()

//////////////////////////////////////////////
//	Transition method
//////////////////////////////////////////////

void transition()
{

int    			adrword;
int			adrlsb;
int    			wdata;
int    			rdata;
int    			be;   
int    			cmd;    
int			rerror;
  
if (RESETN == false) {
FIFO_STATE = 0;
return;
} // end RESETN

switch (FIFO_STATE) {
       	
case 0 :
	if (VCI.CMDVAL == true) { 
		adrword = ((int)VCI.ADDRESS.read()) & 0xFFFFFFFC; 
		adrlsb  = ((int)VCI.ADDRESS.read()) & 0x00000003; 
		wdata   = (int)VCI.WDATA.read(); 
		be      = (int)VCI.BE.read(); 
		cmd     = (int)VCI.CMD.read();
		rdata	= 0;
		rerror 	= 1;
		for (int i = 0 ; ((i < NB_SEG) && (rerror != 0)) ; i++) { 
			if ((adrword >= BASE[i]) && (adrword <  BASE[i] + SIZE[i])) {
#ifdef WATCH_RAM_CONTENT
			  for (unsigned int j=0; j<m_watched_segment_list.size(); ++j) {
			    if ((adrword >= m_watched_segment_list[j].begin_address) &&
				(adrword <  m_watched_segment_list[j].end_address)) {
			      *(m_outstream_list[j]) << NAME
						     << ": access in watched segment "<< j << ".\n"
						     << " Time    = " << sc_time_stamp() << "\n"
						     << " Address = " << std::hex<<adrword<<std::dec<<std::endl;
			      if ((cmd&0x1)==1) {
				*(m_outstream_list[j]) << " Type    = READ\n"
						       << " Data    = "<<std::hex<<RAM[i][(adrword - BASE[i]) >> 2]<<std::dec
						       << std::endl;
			      } else {
				*(m_outstream_list[j]) << " Type    = WRITE\n"
						       << " Data    = "<<std::hex<<wdata <<std::dec
						       << std::endl;
			      }
			      break;
			    }
			  }
#endif
#ifdef VERBOSE_MULTIRAM
			  std::cout << NAME << ": received a request at @ 0x"<<std::hex<<adrword<<std::dec
				    << " and time " << sc_time_stamp()
				    << std::endl;
#endif
			  rdata  = rw_seg(RAM[i], (adrword - BASE[i]) >> 2, wdata, be, cmd, adrlsb);
			  rerror = 0;
			} // end if 
		} // end for
		FIFO_RDATA[0] 	= rdata; 
		FIFO_RERROR[0]	= rerror;
		FIFO_RSRCID[0]	= (int)VCI.SRCID.read();
		FIFO_RTRDID[0]	= (int)VCI.TRDID.read();
		FIFO_RPKTID[0]	= (int)VCI.PKTID.read();
		FIFO_REOP[0]	= (int)VCI.EOP.read();
		FIFO_STATE	= 1;
	} else {
		FIFO_STATE = 0;
	}
break;

case 1 :
	if (VCI.CMDVAL == true) { 
		adrword = ((int)VCI.ADDRESS.read()) & 0xFFFFFFFC; 
		adrlsb  = ((int)VCI.ADDRESS.read()) & 0x00000003; 
		wdata   = (int)VCI.WDATA.read(); 
		be      = (int)VCI.BE.read(); 
		cmd     = (int)VCI.CMD.read();
		rdata	= 0;
		rerror 	= 1;
		for (int i = 0 ; ((i < NB_SEG) && (rerror != 0)) ; i++) { 
			if ((adrword >= BASE[i]) && (adrword <  BASE[i] + SIZE[i])) { 
#ifdef VERBOSE_MULTIRAM
			  std::cout << NAME << ": received a request at @ 0x"<<std::hex<<adrword<<std::dec
				    << " and time " << sc_time_stamp()
				    << std::endl;
#endif
#ifdef WATCH_RAM_CONTENT
			  for (unsigned int j=0; j<m_watched_segment_list.size(); ++j) {
			    if ((adrword >= m_watched_segment_list[j].begin_address) &&
				(adrword <  m_watched_segment_list[j].end_address)) {
			      *(m_outstream_list[j]) << NAME
						     << ": access in watched segment "<< j << ".\n"
						     << " Time    = " << sc_time_stamp() << "\n"
						     << " Address = " << std::hex<<adrword<<std::dec<<std::endl;
			      if ((cmd&0x1)==1) {
				*(m_outstream_list[j]) << " Type    = READ\n"
						       << " Data    = "<<std::hex<<RAM[i][(adrword - BASE[i]) >> 2]<<std::dec
						       << std::endl;
			      } else {
				*(m_outstream_list[j]) << " Type    = WRITE\n"
						       << " Data    = "<<std::hex<<wdata <<std::dec
						       << std::endl;
			      }
			      break;
			    }
			  }
#endif
			  rdata  = rw_seg(RAM[i], (adrword - BASE[i]) >> 2, wdata, be, cmd, adrlsb);
			  rerror = 0;
			} // end if 
		} // end for
		if (VCI.RSPACK == true) { 
			FIFO_RDATA[0] 	= rdata; 
			FIFO_RERROR[0]	= rerror;
			FIFO_RSRCID[0]	= (int)VCI.SRCID.read();
			FIFO_RTRDID[0]	= (int)VCI.TRDID.read();
			FIFO_RPKTID[0]	= (int)VCI.PKTID.read();
			FIFO_REOP[0]	= (int)VCI.EOP.read();
			FIFO_STATE	= 1;
		} else {
			FIFO_RDATA[1] 	= rdata; 
			FIFO_RERROR[1]	= rerror;
			FIFO_RSRCID[1]	= (int)VCI.SRCID.read();
			FIFO_RTRDID[1]	= (int)VCI.TRDID.read();
			FIFO_RPKTID[1]	= (int)VCI.PKTID.read();
			FIFO_REOP[1]	= (int)VCI.EOP.read();
			FIFO_STATE	= 2;
		}
	} else {  //  CMDVAL == false
		if (VCI.RSPACK == true) { 
			FIFO_STATE	= 0;
		} else {
			FIFO_STATE	= 1;
		}
	}
break;

case 2 :
	if (VCI.RSPACK == true) {
		FIFO_RDATA[0] 	= FIFO_RDATA[1]; 
		FIFO_RERROR[0]	= FIFO_RERROR[1];
		FIFO_RSRCID[0]	= FIFO_RSRCID[1];
		FIFO_RTRDID[0]	= FIFO_RTRDID[1];
		FIFO_RPKTID[0]	= FIFO_RPKTID[1];
		FIFO_REOP[0]	= FIFO_REOP[1];
		FIFO_STATE	= 1;
	} else {
		FIFO_STATE	= 2;
	}
break;
	
} // end switch FIFO_STATE

}; // end transition()

/////////////////////////////////////////////////////
//	genMoore() method
/////////////////////////////////////////////////////

void genMoore()
{

switch(FIFO_STATE) {

	case 0 :
		VCI.CMDACK = true;
		VCI.RSPVAL = false;
#if 0
		VCI.RDATA  = 0;
#endif
		VCI.RERROR = 0;
#if 0
		VCI.RSRCID = 0;
		VCI.RPKTID = 0;
		VCI.RTRDID = 0;
		VCI.REOP   = false;
#endif
		break;

	case 1 :
		VCI.CMDACK = true;
		VCI.RSPVAL = true;
		VCI.RDATA  = (sc_uint<8*CELLSIZE>)FIFO_RDATA[0];
		VCI.RERROR = (sc_uint<ERRSIZE>) FIFO_RERROR[0];
		VCI.RSRCID = (sc_uint<SRCIDSIZE>) FIFO_RSRCID[0];
		VCI.RTRDID = (sc_uint<TRDIDSIZE>) FIFO_RTRDID[0];
		VCI.RPKTID = (sc_uint<PKTIDSIZE>) FIFO_RPKTID[0];
		VCI.REOP   = (bool) FIFO_REOP[0];
		break;

	case 2 :
		VCI.CMDACK = false;
		VCI.RSPVAL = true;
		VCI.RDATA  = (sc_uint<8*CELLSIZE>)FIFO_RDATA[0];
		VCI.RERROR = (sc_uint<ERRSIZE>) FIFO_RERROR[0];
		VCI.RSRCID = (sc_uint<SRCIDSIZE>) FIFO_RSRCID[0];
		VCI.RTRDID = (sc_uint<TRDIDSIZE>) FIFO_RTRDID[0];
		VCI.RPKTID = (sc_uint<PKTIDSIZE>) FIFO_RPKTID[0];
		VCI.REOP   = (bool) FIFO_REOP[0];
		break;

} // end switch FIFO_STATE

}; // end genMoore()
#if defined(STATESAVER_SYSTEM)
    /*virtual implicit*/ void save_state(FILE *fichier)
    {
	for (int i=0;i<NB_SEG;i++)
	{
		fprintf(fichier,"segment:%s\n", SEGNAME[i]); 
		fprintf(fichier,"size:%x\n", SIZE[i]); 
		fprintf(fichier,"base:%x\n", BASE[i]); 
//		fprintf(fichier,"%s\n", SEGNAME[i]); 
//		fprintf(fichier,"%d\n", SIZE[i]); 
//		fprintf(fichier,"%d\n", BASE[i]); 
		int *ram=RAM[i];
		for (int j=0;j<(SIZE[i]>>2);j++)
			fprintf(fichier,"%x:%x\n",BASE[i]+4*j,*(ram+j));
	}
	printf("save multiram state\n");
    }

    /*virtual implicit*/ void restore_state(FILE *fichier)
    {
	char temp[200];
	int dummy;

	for (int i=0;i<NB_SEG;i++)
	{
//		fgets(temp,100,fichier); // SEGNAME
//		fgets(temp,100,fichier); // SIZE
//		fgets(temp,100,fichier); // BASE
    int size, base;
    fscanf(fichier,"segment:%s\n", temp);
    fscanf(fichier,"size:%x\n", &size);
    fscanf(fichier,"base:%x\n", &base);
    if (strcmp (SEGNAME[i], temp) != 0)
      printf("segment names differs.\n");
    if (size != SIZE[i])
      printf("segment sizes differs.\n");
    if (base != BASE[i])
      printf("segment bases differs.\n");
		int *ram=RAM[i];
		for (int j=0;j<(SIZE[i]>>2);j++)
			fscanf(fichier,"%x:%x\n",&dummy,(ram+j));
	}
        printf("restore multiram state\n");
    }

#endif
};  // end structure SOCLIB_VCI_MULTIRAM
//*****************************************************************

#endif // ifndef SOCLIB_VCI_MULTIRAM_H


