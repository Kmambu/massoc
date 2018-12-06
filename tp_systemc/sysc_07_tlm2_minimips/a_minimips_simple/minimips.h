#ifndef MINIMIPS_H_INCLUDED
#define MINIMIPS_H_INCLUDED

#include <sstream>
#include <iomanip>

#include <tlm>
#include <tlm_utils/simple_initiator_socket.h>

/*
 * Minimips processor component
 */
struct Minimips : public sc_core::sc_module
{
  /*
   * port definition
   */
  tlm_utils::simple_initiator_socket<Minimips> p_mem; /* port to memory */

  /*
   * internal variables
   */
  sc_dt::sc_uint<32> r_ir;     /* instruction register */
  sc_dt::sc_uint<32> r_pc;     /* program counter */
  sc_dt::sc_uint<32> r_rf[32]; /* register file */

  /*
   * constructor
   */
  SC_HAS_PROCESS(Minimips);
  Minimips(sc_core::sc_module_name nm)
  : p_mem("p_mem")
  {
    /* declare our behavior loop */
    SC_THREAD(behavior);

    /* reset the processor */
    r_pc = 0x00000000;
  }

  /*
   * instruction fetch
   */
  void fetch_instruction()
  {
    /* create a transaction for ifetch */
    tlm::tlm_generic_payload trans_ifetch;
    sc_dt::uint_type ir;

    /* fill up the transaction:
     * - it's a read
     * - at the address pointed by PC
     * - the response goes into IR
     */
    trans_ifetch.set_read();
    trans_ifetch.set_address(r_pc);
    trans_ifetch.set_data_ptr((unsigned char*)&ir);
    trans_ifetch.set_data_length(4);
    trans_ifetch.set_streaming_width(4);
    trans_ifetch.set_byte_enable_ptr(0);
    trans_ifetch.set_dmi_allowed(false);
    trans_ifetch.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

    /* send the transaction to the memory (blocking call) */
    sc_core::sc_time mem_delay = sc_core::sc_time(0, sc_core::SC_NS);
    p_mem->b_transport(trans_ifetch, mem_delay);
    wait(mem_delay);

    /* check response status */
    if (trans_ifetch.is_response_error())
      SC_REPORT_ERROR(this->name(), ("Response error for ifetch: " + trans_ifetch.get_response_string()).c_str());

#if defined(DEBUG)
    {
      std::stringstream ss;
      ss << "instruction fetch @ " << sc_core::sc_time_stamp() << ": <"
         << std::hex
         << " @:0x" << std::setfill('0') << std::setw(8) << (int)r_pc.value()
         << " data:0x" << std::setfill('0') << std::setw(8) << (int)ir << ">";
      SC_REPORT_INFO(this->name(), ss.str().c_str());
    }
#endif

    /* update IR */
    r_ir = ir;

    /* update PC */
    r_pc += 4;
  }

  /*
   * Data memory access
   */
  void access_data_memory(bool cmd, sc_dt::uint_type &addr, sc_dt::uint_type &data)
  {
    /* create a transaction for ifetch */
    tlm::tlm_generic_payload trans_data;

    /* fill up the data transaction */
    if (cmd) {
      /* cmd == true means it's a write operation */
      trans_data.set_write();
    } else {
      /* false means it's a read operation */
      trans_data.set_read();
    }
    trans_data.set_address(addr);
    trans_data.set_data_ptr((unsigned char*)&data);
    trans_data.set_data_length(4);
    trans_data.set_streaming_width(4);
    trans_data.set_byte_enable_ptr(0);
    trans_data.set_dmi_allowed(false);
    trans_data.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

    /* send the transaction to the memory (blocking call) */
    sc_core::sc_time mem_delay = sc_core::sc_time(0, sc_core::SC_NS);
    p_mem->b_transport(trans_data, mem_delay);
    wait(mem_delay);

    /* check response status */
    if (trans_data.is_response_error())
      SC_REPORT_ERROR(this->name(), ("Response error for data access" + trans_data.get_response_string()).c_str());

#if defined(DEBUG)
    {
      std::stringstream ss;
      const char *rw = cmd ? "wt" : "rd";
      ss << "data access @ " << sc_core::sc_time_stamp() << ": <" << rw
         << std::hex
         << " @:0x" << std::setfill('0') << std::setw(8) << (int)addr
         << " data:0x" << std::setfill('0') << std::setw(8) << (int)data << ">";

      SC_REPORT_INFO(this->name(), ss.str().c_str());
    }
#endif

  }

  enum mips_opcod {
    OP_SPECIAL  = 0,
    OP_BEQ      = 4,
    OP_BNE      = 5,
    OP_ADDI     = 8,
    OP_LW       = 35,
    OP_SW       = 43,
  };

  enum mips_opcod_special {
    OP_BREAK    = 13,
    OP_ADD      = 32,
  };

  sc_dt::uint_type rf_read_register(sc_dt::uint_type reg)
  {
    if (reg)
      return r_rf[reg].value();
    else
      return 0; /* $0 is always 0 */
  }

  void rf_write_register(sc_dt::uint_type reg, sc_dt::uint_type data)
  {
    /* don't write in $0 */
    if (reg)
      r_rf[reg] = data;
  }

  /*
   * behavior loop
   */
  void behavior()
  {
    for (;;)
      {
        /* advance time to the next cycle */
        sc_core::sc_time delay = sc_core::sc_time(10, sc_core::SC_NS);
        wait(delay);

        /* fetch a new instruction to execute */
        fetch_instruction();

        /* decode the instruction */
        sc_dt::uint_type ir_opcod = r_ir.range(31, 26).value();

        sc_dt::uint_type ir_rs    = r_ir.range(25, 21).value();
        sc_dt::uint_type ir_rt    = r_ir.range(20, 16).value();
        sc_dt::uint_type ir_rd    = r_ir.range(15, 11).value();
        sc_dt::uint_type ir_sh    = r_ir.range(10, 6).value();
        sc_dt::uint_type ir_func  = r_ir.range(5, 0).value();

        sc_dt::int_type  ir_simm   = (signed short)r_ir.range(15, 0).value();
        sc_dt::uint_type ir_uimm   = r_ir.range(15, 0).value();


        /* execute the instruction */
        switch (ir_opcod)
          {
          case OP_BEQ:
#if defined(DEBUG)
            SC_REPORT_INFO(this->name(), "Executing BEQ");
#endif
            if (rf_read_register(ir_rs) == rf_read_register(ir_rt))
              r_pc += ir_simm * 4;
            break;

          case OP_BNE:
#if defined(DEBUG)
            SC_REPORT_INFO(this->name(), "Executing BNE");
#endif
            if (rf_read_register(ir_rs) != rf_read_register(ir_rt))
              r_pc += ir_simm * 4;
            break;

          case OP_ADDI:
            {
#if defined(DEBUG)
              SC_REPORT_INFO(this->name(), "Executing ADDI");
#endif
              sc_dt::uint_type res = rf_read_register(ir_rs)
                + ir_simm;

              rf_write_register(ir_rt, res);
            }
            break;

          case OP_LW:
            {
#if defined(DEBUG)
              SC_REPORT_INFO(this->name(), "Executing LW");
#endif
              sc_dt::uint_type addr = rf_read_register(ir_rs)
                + ir_simm;

              sc_dt::uint_type data;

              access_data_memory(false, addr, data);

              rf_write_register(ir_rt, data);
            }
            break;

          case OP_SW:
            {
#if defined(DEBUG)
              SC_REPORT_INFO(this->name(), "Executing SW");
#endif
              sc_dt::uint_type addr = rf_read_register(ir_rs)
                + ir_simm;

              sc_dt::uint_type data = rf_read_register(ir_rt);

              access_data_memory(true, addr, data);
            }
            break;

          case OP_SPECIAL:
            {
              switch(ir_func)
                {
                case OP_BREAK:
                  {
                    SC_REPORT_INFO(this->name(), "Break instruction");
                    sc_core::sc_stop();
                  }
                  break;

                case OP_ADD:
                  {
#if defined(DEBUG)
                    SC_REPORT_INFO(this->name(), "Executing ADD");
#endif
                    sc_dt::uint_type res = rf_read_register(ir_rs)
                      + rf_read_register(ir_rt);

                    rf_write_register(ir_rd, res);
                  }
                  break;

                default:
                  SC_REPORT_ERROR(this->name(), "Unknow special opcod");
                  break;
                }
            }
            break;

          default:
            SC_REPORT_ERROR(this->name(), "Unknow opcod");
            break;
          }
      }
  }
};

#endif // MINIMIPS_H_INCLUDED

/*
 * Local Variables:
 * mode: C++
 * End:
 */
