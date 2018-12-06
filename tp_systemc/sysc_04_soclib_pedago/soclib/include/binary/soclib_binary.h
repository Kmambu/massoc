/**************************************************************************************
 * File : soclib_binary.h
 * Date : 09/12/2003
 * Authors : F. Pecheux 
 * Release ; V0.1
 * It is released under the GNU Public License.
 * 
 * This file defines the VCI ports and signals data types that must be used
 * by all components included in the SOCLIB library of simulation models.
**************************************************************************************/

#ifndef SOCLIB_BINARY_H
#define SOCLIB_BINARY_H

class SOCLIB_BINARY
{

public:
	void *ptab;
	int size; // Size of loaded data in bytes
};

#endif
