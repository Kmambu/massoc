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
 *         Francois Pecheux <francois.pecheux@lip6.fr>, 2010
 *
 * Maintainers: fpx
 */

#include "../include/vci_io.h"
#include "io.h"

#include <stdarg.h>

namespace soclib {
namespace caba {

#define tmpl(typ) template<typename vci_param> typ VciIo<vci_param>

tmpl(bool)::on_write(int seg, typename vci_param::addr_t addr, typename vci_param::data_t data, int be)
{
    int cell = (int)addr / vci_param::B;
	int reg = cell % IO_SPAN;

	switch (reg) {
    case IO_OUT:
        printf("IO_OUT\n");
        r_out = data;
        return true;
    default:
		return false;
	}
}

tmpl(bool)::on_read(int seg, typename vci_param::addr_t addr, typename vci_param::data_t &data)
{
    int cell = (int)addr / vci_param::B;
	int reg = cell % IO_SPAN;

	switch (reg) {
    case IO_IN:
        printf("IO_IN\n");
        data = r_in;
        return true;
    default:
        return false;
	}
}

tmpl(void)::transition()
{
	if (!p_resetn) {
		m_vci_fsm.reset();

        // IO part
        r_out = 0;
		return;
	}

	m_vci_fsm.transition();

	r_in = (p_i3.read() << 3)  | (p_i2.read() << 2)  | (p_i1.read() << 1)  | (p_i0.read()) ;
}

tmpl(void)::genMoore()
{
	m_vci_fsm.genMoore();
	p_o3=(r_out&0x8)?1:0;
	p_o2=(r_out&0x4)?1:0;
	p_o1=(r_out&0x2)?1:0;
	p_o0=(r_out&0x1)?1:0;
}

tmpl(/**/)::VciIo(
                  sc_module_name name,
                  const IntTab &index,
                  const MappingTable &mt,
                  const uint32_t id)
           : soclib::caba::BaseModule(name),
           m_vci_fsm(p_vci, mt.getSegmentList(index)),m_debug(false),
           p_clk("clk"),
           p_resetn("resetn"),
           p_vci("vci"),
           p_o0("o0"),
           p_o1("o1"),
           p_o2("o2"),
           p_o3("o3"),
           p_i0("i0"),
           p_i1("i1"),
           p_i2("i2"),
           p_i3("i3")
{
    m_vci_fsm.on_read_write(on_read, on_write);

    SC_METHOD(transition);
    dont_initialize();
    sensitive << p_clk.pos();

    SC_METHOD(genMoore);
    dont_initialize();
    sensitive << p_clk.neg();
}

tmpl(/**/)::~VciIo()
{
}

}}

// Local Variables:
// tab-width: 4
// c-basic-offset: 4
// c-file-offsets:((innamespace . 0)(inline-open . 0))
// indent-tabs-mode: nil
// End:

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4
