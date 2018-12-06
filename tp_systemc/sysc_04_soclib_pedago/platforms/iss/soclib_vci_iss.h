#ifndef SOCLIB_VCI_ISS_H
#define SOCLIB_VCI_ISS_H

#define sc_register sc_signal

#include <signal.h>
//#include <iostream.h>
//#include <fstream.h>
#include <stdlib.h>
#include <systemc.h>

template <
  int ADDRSIZE,
  int CELLSIZE,
  int ERRSIZE,
  int PLENSIZE,
  int CLENSIZE,
  int SRCIDSIZE,
  int TRDIDSIZE,
  int PKTIDSIZE >
struct SOCLIB_VCI_ISS : sc_module {

  sc_in<bool> CLK;
  sc_in<bool> RESETN;
  ADVANCED_VCI_INITIATOR<ADDRSIZE, CELLSIZE, ERRSIZE, PLENSIZE, CLENSIZE, SRCIDSIZE, TRDIDSIZE, PKTIDSIZE> VCI_INITIATOR;

  const char *NAME;
  sc_register<int> ISS_FSM;
  sc_register<int> GPR[32];
  sc_register<int> PC, IR, EA;

  enum{
    ISS_REQ_IFETCH = 0,
    ISS_RSP_IFETCH = 1,
    ISS_DECODE_AND_EXECUTE = 2,
    ISS_REQ_LOAD = 3,
    ISS_RSP_LOAD = 4,
    ISS_REQ_STORE = 5,
    ISS_RSP_STORE = 6
  };

  enum {
    OP_SPECIAL = 0,
    OP_ADDI = 8,
    OP_ORI = 13,
    OP_LW = 35,
    OP_SW = 43,
    FUNC_ADD = 32,
    FUNC_SUB = 34,
  };

  SC_HAS_PROCESS (SOCLIB_VCI_ISS);
  SOCLIB_VCI_ISS(
                 sc_module_name insname
                )
  {
#ifdef NONAME_RENAME
    char name[100];
    ISS_FSM.rename("ISS_FSM");
    PC.rename("PC");
    IR.rename("IR");
    EA.rename("EA");
    for (int j=0 ; j < 32; j++)
      {
        sprintf(name,"GPR_%2.2d",j);
        GPR[j].rename(name);
      }

#endif

    SC_METHOD (transition);
    sensitive << CLK.pos();
    SC_METHOD (genMoore);
    sensitive << CLK.neg();

    NAME = (char*) strdup(insname);

    if (NAME == NULL) {
      perror("malloc");
      exit(1);
    }
    printf("SOCLIB_VCI_ISS instanciated with name %s\n",NAME);
  }

  void transition()
  {
    if(RESETN == false)
      {
        ISS_FSM = ISS_REQ_IFETCH;
        IR=0;
        PC=0x00000000;
        EA=0;
        for (int i=0;i<32;i++)
          GPR[i]=0;
      } else
      {
        printf("ISS_FSM=%d\n",(int)ISS_FSM.read());
        switch(ISS_FSM)
          {
          case ISS_REQ_IFETCH :
            if(VCI_INITIATOR.CMDACK == true)
              {
                ISS_FSM = ISS_RSP_IFETCH;
              }
            break;
          case ISS_RSP_IFETCH :
            if(VCI_INITIATOR.RSPVAL == true)
              {
                ISS_FSM = ISS_DECODE_AND_EXECUTE;
                IR = (int) VCI_INITIATOR.RDATA.read();
              }
            break;
          case ISS_DECODE_AND_EXECUTE :
            {
              int opcod=(IR>>26)&0x3F;
              int rs,rt,rd,func,imm;

              switch (opcod)
                {
                case OP_ADDI:
                  rs=(IR>>21)&0x1F;
                  rt=(IR>>16)&0x1F;
                  imm=(IR&0xFFFF);
                  if (imm & 0x8000)
                    imm |= 0xFFFF0000;

                  GPR[rt]=GPR[rs] + imm;

                  PC=PC+4;
                  ISS_FSM=ISS_REQ_IFETCH;
                  break;
                case OP_ORI:
                  rs=(IR>>21)&0x1F;
                  rt=(IR>>16)&0x1F;
                  imm=(IR&0xFFFF);
                  if (imm & 0x8000)
                    imm |= 0xFFFF0000;

                  GPR[rt]=GPR[rs] | imm;

                  PC=PC+4;
                  ISS_FSM=ISS_REQ_IFETCH;
                  break;
                case OP_SPECIAL:
                  rs=(IR>>21)&0x1F;
                  rt=(IR>>16)&0x1F;
                  rd=(IR>>11)&0x1F;
                  func=IR&0x3F;
                  switch (func)
                    {
                    case FUNC_ADD:
                      GPR[rd]=GPR[rs] + GPR[rt];
                      break;
                    case FUNC_SUB:
                      GPR[rd]=GPR[rs] - GPR[rt];
                      break;
                    }
                  PC=PC+4;
                  ISS_FSM=ISS_REQ_IFETCH;
                  break;
                case OP_LW:
                  rs=(IR>>21)&0x1F;
                  imm=(IR&0xFFFF);
                  if (imm & 0x8000)
                    imm |= 0xFFFF0000;
                  EA=GPR[rs]+imm;
                  ISS_FSM=ISS_REQ_LOAD;
                  break;
                case OP_SW:
                  rs=(IR>>21)&0x1F;
                  imm=(IR&0xFFFF);
                  if (imm & 0x8000)
                    imm |= 0xFFFF0000;
                  EA=GPR[rs]+imm;
                  ISS_FSM=ISS_REQ_STORE;
                  break;
                }
            }
            break;
          case ISS_REQ_LOAD :
            if(VCI_INITIATOR.CMDACK == true)
              {
                ISS_FSM = ISS_RSP_LOAD;
              }
            break;
          case ISS_RSP_LOAD :
            if(VCI_INITIATOR.RSPVAL == true)
              {
                int rt;
                rt=(IR>>16)&0x1F;

                GPR[rt] = (int)VCI_INITIATOR.RDATA.read();

                PC=PC+4;
                ISS_FSM = ISS_REQ_IFETCH;
              }
            break;
          case ISS_REQ_STORE :
            if(VCI_INITIATOR.CMDACK == true)
              {
                ISS_FSM = ISS_RSP_STORE;
              }
            break;
          case ISS_RSP_STORE :
            if(VCI_INITIATOR.RSPVAL == true)
              {
                PC=PC+4;
                ISS_FSM = ISS_REQ_IFETCH;
              }
            break;
          }
      }
  }

  void genMoore()
  {
    int rt;

    switch (ISS_FSM)
      {
      case ISS_REQ_IFETCH:
        VCI_INITIATOR.CMDVAL = true;
        VCI_INITIATOR.RSPACK = false;
        VCI_INITIATOR.ADDRESS = (sc_uint<32>) PC.read();
        VCI_INITIATOR.WDATA = 0;
        VCI_INITIATOR.CMD = VCI_CMD_READ;
        VCI_INITIATOR.EOP = true;
        VCI_INITIATOR.BE = 0xF;
        VCI_INITIATOR.PLEN = 1 << 2;
        break;
      case ISS_RSP_IFETCH:
        VCI_INITIATOR.CMDVAL = false;
        VCI_INITIATOR.RSPACK = true;
        break;
      case ISS_REQ_LOAD:
        VCI_INITIATOR.CMDVAL = true;
        VCI_INITIATOR.RSPACK = false;
        VCI_INITIATOR.ADDRESS = (sc_uint<32>) EA.read();
        VCI_INITIATOR.WDATA = 0;
        VCI_INITIATOR.CMD = VCI_CMD_READ;
        VCI_INITIATOR.EOP = true;
        VCI_INITIATOR.BE = 0xF;
        VCI_INITIATOR.PLEN = 1 << 2;
        break;
      case ISS_RSP_LOAD:
        VCI_INITIATOR.CMDVAL = false;
        VCI_INITIATOR.RSPACK = true;
        break;
      case ISS_REQ_STORE:
        VCI_INITIATOR.CMDVAL = true;
        VCI_INITIATOR.RSPACK = false;
        VCI_INITIATOR.ADDRESS = (sc_uint<32>) EA.read();
        rt=(IR>>16)&0x1F;
        VCI_INITIATOR.WDATA = (sc_uint<32>) GPR[rt];
        VCI_INITIATOR.CMD = VCI_CMD_WRITE;
        VCI_INITIATOR.EOP = true;
        VCI_INITIATOR.BE = 0xF;
        VCI_INITIATOR.PLEN = 1 << 2;
        break;
      case ISS_RSP_STORE:
        VCI_INITIATOR.CMDVAL = false;
        VCI_INITIATOR.RSPACK = true;
        break;
      }
  }

};

#endif
