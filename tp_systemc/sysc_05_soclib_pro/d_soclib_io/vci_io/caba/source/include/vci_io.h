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
#ifndef SOCLIB_VCI_IO_H
#define SOCLIB_VCI_IO_H

#include <systemc>
#include "vci_target_fsm.h"
#include "caba_base_module.h"
#include "mapping_table.h"

namespace soclib {
namespace caba {

using namespace sc_core;

template<typename vci_param>
class VciIo
	: public soclib::caba::BaseModule
{
private:
    /* Internal states */
    soclib::caba::VciTargetFsm<vci_param, true> m_vci_fsm;

    bool on_write(int seg, typename vci_param::addr_t addr, typename vci_param::data_t data, int be);
    bool on_read(int seg, typename vci_param::addr_t addr, typename vci_param::data_t &data);
    void transition();
    void genMoore();

    bool m_debug;

public:
    // Registers
    uint32_t r_out;
    uint32_t r_in;

protected:
    SC_HAS_PROCESS(VciIo);

public:
    sc_in<bool> p_clk;
    sc_in<bool> p_resetn;
    soclib::caba::VciTarget<vci_param> p_vci;
    sc_out<bool> p_o0;
    sc_out<bool> p_o1;
    sc_out<bool> p_o2;
    sc_out<bool> p_o3;
    sc_in<bool> p_i0;
    sc_in<bool> p_i1;
    sc_in<bool> p_i2;
    sc_in<bool> p_i3;

	VciIo(
          sc_module_name name,
          const IntTab &index,
          const MappingTable &mt,
          const uint32_t id);

    ~VciIo();
};

}}

#endif /* SOCLIB_VCI_IO_H */

// Local Variables:
// tab-width: 4
// c-basic-offset: 4
// c-file-offsets:((innamespace . 0)(inline-open . 0))
// indent-tabs-mode: nil
// End:

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4
