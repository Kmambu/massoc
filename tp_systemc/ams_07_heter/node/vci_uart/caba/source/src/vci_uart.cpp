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
 *         Francois Pecheux <francois.pecheux@lip6.fr>, 2009
 *
 * Maintainers: fpx 
 */

#include "../include/vci_uart.h"
#include "uart.h"

#include <stdarg.h>

namespace soclib {
namespace caba {

#define tmpl(typ) template<typename vci_param> typ VciUart<vci_param>

tmpl(bool)::on_write(int seg, typename vci_param::addr_t addr, typename vci_param::data_t data, int be)
{
    int cell = (int)addr / vci_param::B;
	int reg = cell % UART_SPAN;

	switch (reg) {
	case UART_DATA:
//printf("UART_DATA_WRITE\n");
        r_txval = data;
        hasWriteData = true;
        return true;
	case UART_BR:
//printf("UART_BR_WRITE\n");
        r_baudgen = data;
        r_baudrxcounter = 0;
        r_baudtxcounter = 0;
        m_cpt_write++;
		return true;
	case UART_CTRL:
//printf("UART_CTRL_WRITE\n");
        r_control = data;
        r_baudrxcounter = 0;
        r_baudtxcounter = 0;
        m_cpt_write++;
		return true;
	default:
		return false;
	}
}

tmpl(bool)::on_read(int seg, typename vci_param::addr_t addr, typename vci_param::data_t &data)
{
    int cell = (int)addr / vci_param::B;
	int reg = cell % UART_SPAN;

	switch (reg) {
	case UART_DATA:
        data = r_rxval;
        r_irq = false;
        r_uartstatus = r_uartstatus & 0xFFFFFFFE;
        m_cpt_read++;
        return true;
	case UART_STATUS:
        data = r_uartstatus;
        m_cpt_read++;
        return true;
	case UART_ID:
        data = r_id;
        return true;
	case UART_BR:
        data = r_baudgen;
        m_cpt_read++;
        return true;
	case UART_CTRL:
        data = r_control;
        m_cpt_read++;
        return true;
	default:
		return false;
	}
}

tmpl(void)::transition()
{
	if (!p_resetn) {
		m_vci_fsm.reset();

        // UART part
        r_control = 0;
        r_baudtxcounter = 0;
        r_baudrxcounter = 0;
        r_bittxcounter = 0;
        r_bitrxcounter = 0;
        r_uarttxfsm = UART_TX_IDLE;
        r_uartrxfsm = UART_RX_IDLE;
        r_uartstatus = 0;
        r_txval = r_rxval = 0;
        r_irq = 0;
		return;
	}

    hasWriteData = false;
	m_vci_fsm.transition();
    // Uart FSM

    switch (r_uarttxfsm)
    {
        case UART_TX_IDLE:
            r_baudtxcounter = 0;
            r_bittxcounter = 0;
            if (hasWriteData)
            {
                r_uarttxfsm = UART_TX_START;
                r_uartstatus = r_uartstatus | 0x00000002;
            }
            break;
        case UART_TX_START:
            r_baudtxcounter++;
            if (r_baudtxcounter == r_baudgen)
            {
                r_baudtxcounter = 0;
                r_uarttxfsm = UART_TX_BIT;
            }
            break;
        case UART_TX_BIT:
            r_baudtxcounter++;
            if (r_baudtxcounter == r_baudgen)
            {
                r_baudtxcounter = 0;
                r_bittxcounter++;
                if (r_bittxcounter == 32)
                {
                    r_bittxcounter=0;
                    r_uarttxfsm = UART_TX_PARITY;
                }
            }
            break;
        case UART_TX_PARITY:
            r_baudtxcounter++;
            if (r_baudtxcounter == r_baudgen)
            {
                r_baudtxcounter = 0;
                r_uarttxfsm = UART_TX_STOP;
            }
            break;
        case UART_TX_STOP:
            r_baudtxcounter++;
            if (r_baudtxcounter == r_baudgen)
            {
                r_baudtxcounter = 0;
                r_uarttxfsm = UART_TX_IDLE;
                r_uartstatus = r_uartstatus & 0xFFFFFFFD;
            }
            break;
    }

//if (m_debug) printf("r_rxval=%x\n",r_rxval);
//if (m_debug) printf("rx=%d ",p_rx.read());
//if (m_debug) printf("T r_uartrxfsm=%d r_bitrxcounter=%d\n",r_uartrxfsm,r_bitrxcounter);
    switch (r_uartrxfsm)
    {
        case UART_RX_IDLE:
            r_baudrxcounter = 0;
            r_bitrxcounter = 0;
            if (p_rx == 1)
            {
                r_uartrxfsm = UART_RX_START;
            }
            break;
        case UART_RX_START:
            r_baudrxcounter++;
//if (m_debug) printf("r_baudrxcounter=%d\n",r_baudrxcounter);
//if (m_debug) printf("r_baudgen/2=%d\n",r_baudgen>>1);
            if (r_baudrxcounter == (r_baudgen + (r_baudgen >> 1)))
            {
                r_baudrxcounter = 0;
                r_rxval = 0;
                r_uartrxfsm = UART_RX_BIT;
            }
            break;
        case UART_RX_BIT:
            r_rxval = p_rx  << r_bitrxcounter | r_rxval;
            r_uartrxfsm = UART_RX_WAITFORNEXTBIT;
            break;
        case UART_RX_WAITFORNEXTBIT:
            r_baudrxcounter++;
            if (r_baudrxcounter == (r_baudgen-1))
            {
                r_baudrxcounter=0;
                r_bitrxcounter++;
                if (r_bitrxcounter == 32)
                    r_uartrxfsm = UART_RX_PARITY;
                else
                    r_uartrxfsm= UART_RX_BIT;
            }
            break;
        case UART_RX_PARITY:
            r_uartrxfsm = UART_RX_WAITFORSTOPBIT;
            break;
        case UART_RX_WAITFORSTOPBIT:
            r_baudrxcounter++;
            if (r_baudrxcounter == (r_baudgen-1))
            {
                r_baudrxcounter=0;
                r_uartrxfsm = UART_RX_STOP;
            }
            break;
        case UART_RX_STOP:
            r_uartrxfsm = UART_RX_WAITFOREND;
            break;
        case UART_RX_WAITFOREND:
            r_baudrxcounter++;
            if (r_baudrxcounter == (r_baudgen-1))
            {
                r_baudrxcounter=0;
                r_uartrxfsm = UART_RX_IDLE;
                if (r_control & 1)
                    r_irq=true;
                r_uartstatus = r_uartstatus | 0x00000001;
            }
            break;
    }
}

tmpl(void)::genMoore()
{
	m_vci_fsm.genMoore();
//if (m_debug) printf("MOORE r_uartrxfsm=%d r_bitrxcounter=%d\n",r_uartrxfsm,r_bitrxcounter);

    switch (r_uarttxfsm)
    {
        case UART_TX_IDLE:
            p_tx = 0;
            break;
        case UART_TX_START:
            p_tx = 1;
            break;
        case UART_TX_BIT:
            p_tx = (r_txval >> r_bittxcounter) & 1;
            break;
        case UART_TX_PARITY:
            p_tx = 0;
            break;
        case UART_TX_STOP:
            p_tx = 0;
            break;
    }

    p_irq = r_irq;

    if (r_control & 2)
        p_tx_en = true;
    else
        p_tx_en = false;

    if (r_control & 4)
        p_rx_en = true;
    else
        p_rx_en = false;
/*
    for ( size_t i=0; i<m_term.size(); i++ )
        if ( p_irq[i].read() || r_counter % 1024 == 0 )
            p_irq[i] = m_term[i]->hasData();
*/
}

tmpl(/**/)::VciUart(
    sc_module_name name,
    const IntTab &index,
    const MappingTable &mt, bool debug,
    const uint32_t id)
    : soclib::caba::BaseModule(name),
      m_vci_fsm(p_vci, mt.getSegmentList(index)),m_debug(debug),
      r_id(id),
      p_clk("clk"),
      p_resetn("resetn"),
      p_vci("vci"),
      p_tx("tx"),
      p_rx("rx"),
      p_irq("irq")
{
    m_vci_fsm.on_read_write(on_read, on_write);

    SC_METHOD(transition);
    dont_initialize();
    sensitive << p_clk.pos();

    SC_METHOD(genMoore);
    dont_initialize();
    sensitive << p_clk.neg();
}

tmpl(/**/)::~VciUart()
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

