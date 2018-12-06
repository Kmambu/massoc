/**************************************************************************************
 * File : mips_binary.h
 * Date : 09/12/2003
 * Authors : F. Pecheux 
 * Release ; V0.1
 * It is released under the GNU Public License.
 * 
 * This file defines the VCI ports and signals data types that must be used
 * by all components included in the SOCLIB library of simulation models.
**************************************************************************************/

#ifndef MIPS_BINARY_H
#define MIPS_BINARY_H

#include "binary/soclib_binary.h"

extern "C" void loadexec(void **emem, int *esize, int *eoffset, char *file, char **sections);

class MIPS_BINARY : public SOCLIB_BINARY
{
public:
	MIPS_BINARY(){};
	MIPS_BINARY(char *filename, char **sections)
	{
		size=0;

		if (filename==NULL)
		{
			printf("Error in MIPS_BINARY constructor: the \"filename\" parameter is set to NULL");
			exit(1);
		}
		if (sections==NULL)
		{
			printf("Error in MIPS_BINARY constructor: the \"sections\" parameter is set to NULL");
			exit(1);
		}
		int offset=0;
		loadexec(&ptab,&size,&offset,filename,sections);
	}


	void reload (char *filename, char ** sections)
	{



		size=0;

		if (filename==NULL)
		{
			printf("Error in MIPS_BINARY constructor: the \"filename\" parameter is set to NULL");
			exit(1);
		}
		if (sections==NULL)
		{
			printf("Error in MIPS_BINARY constructor: the \"sections\" parameter is set to NULL");
			exit(1);
		}
		int offset=0;
		loadexec(&ptab,&size,&offset,filename,sections);


	}



	
};

#endif
