/* -*- c++ -*-
 * This file is part of SoCLIB.
 *
 * SoCLIB is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * SoCLIB is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SoCLIB; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * Copyright (c) UPMC, Lip6, SoC
 *         Nicolas Pouillon <nipo@ssji.net>, 2007
 *
 * Maintainers: nipo
 */
#ifndef SOCLIB_ISS_H_
#define SOCLIB_ISS_H_

#include <inttypes.h>
#include "caba/util/base_module.h"
#include "common/endian.h"
#include "caba/interface/xcache_processor_ports.h"

namespace soclib { namespace iss {

using namespace soclib;

class Iss
{
protected:
	const std::string m_name;

	enum DataAccess {
		MEM_NONE,
		MEM_LB,
		MEM_LBU,
		MEM_LH,
		MEM_LHU,
		MEM_LW,
		MEM_SB,
		MEM_SH,
		MEM_SW,
		MEM_INVAL
	};

	enum DErrReason {
		DERR_NONE,
		DERR_BERR,
	};

	enum IErrReason {
		IERR_NONE,
		IERR_BERR,
	};

public:
	virtual void reset() = 0;
	virtual bool getIReq( uint32_t &addr ) = 0;
	virtual bool getDReq( uint32_t &addr, DataAccess &type, uint32_t &data ) = 0;
	virtual void setIns( uint32_t ins ) = 0;
	virtual void setData( uint32_t data ) = 0;
	virtual void setIrq( uint32_t irqs ) = 0;
	virtual void setDBerr( Iss::DErrReason reason ) = 0;
	virtual void setIBerr( Iss::IErrReason reason ) = 0;
	virtual void newCycle() = 0;
	virtual void step() = 0;

	Iss( const std::string &name );
	virtual ~Iss() = 0;
};

}}

#endif /* SOCLIB_ISS_H_ */
