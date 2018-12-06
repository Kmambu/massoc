/**************************************************************************************
 * File : arm_binary.h
 * Date : 07/03/2004
 * Authors : F. Pecheux 
 * Release ; V0.1
 * It is released under the GNU Public License.
 * 
 * This file defines the VCI ports and signals data types that must be used
 * by all components included in the SOCLIB library of simulation models.
**************************************************************************************/

#ifndef ARM_BINARY_H
#define ARM_BINARY_H

#include "binary/soclib_binary.h"

extern "C" void loadexec(void **emem, int *esize, int *eoffset, char *file, char **sections);

class ARM_BINARY : public SOCLIB_BINARY
{
public:
	ARM_BINARY(char *filename, char **sections)
	{
		size=0;

		if (filename==NULL)
		{
			printf("Error in ARM_BINARY constructor: the \"filename\" parameter is set to NULL");
			exit(1);
		}
		if (sections==NULL)
		{
			printf("Error in ARM_BINARY constructor: the \"sections\" parameter is set to NULL");
			exit(1);
		}
		int offset=0;
		loadexec(&ptab,&size,&offset,filename,sections);
	}
};

#endif
