#ifndef SOCLIB_VCI_SIMPLEINITIATOR_H
#define SOCLIB_VCI_SIMPLEINITIATOR_H

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

struct SOCLIB_VCI_SIMPLEINITIATOR : sc_module {

  sc_in<bool> CLK;
  sc_in<bool> RESETN;
  ADVANCED_VCI_INITIATOR<ADDRSIZE, CELLSIZE, ERRSIZE, PLENSIZE, CLENSIZE, SRCIDSIZE, TRDIDSIZE, PKTIDSIZE> VCI_INITIATOR;

  const char *NAME;
  sc_register<int> INITIATOR_FSM;
  sc_register<int> REG1;

  enum{
    INITIATOR_IDLE      = 0,
    INITIATOR_REQ_WRITE = 1,
    INITIATOR_RSP_WRITE = 2
  };

  enum{
    REG1_ADR = 0,
    REG2_ADR = 4
  };

  SC_HAS_PROCESS(SOCLIB_VCI_SIMPLEINITIATOR);

  SOCLIB_VCI_SIMPLEINITIATOR(
                             sc_module_name insname // nom de l'instance
                             )
  {
#ifdef NONAME_RENAME
    INITIATOR_FSM.rename("INITIATOR_FSM");
    REG1.rename("REG1");
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
    printf("SOCLIB_VCI_SIMPLEINITIATOR instanciated with name %s\n",NAME);
  }

  void transition()
  {
    if(RESETN == false)
      {
        INITIATOR_FSM = INITIATOR_IDLE;
        REG1 = 0;
      } else
      {
        switch(INITIATOR_FSM)
          {
          case INITIATOR_IDLE :
            INITIATOR_FSM = INITIATOR_REQ_WRITE;
            break;
          case INITIATOR_REQ_WRITE :
            if(VCI_INITIATOR.CMDACK == true)
              {
                INITIATOR_FSM = INITIATOR_RSP_WRITE;
              }
            break;
          case INITIATOR_RSP_WRITE :
            if(VCI_INITIATOR.RSPVAL == true)
              {
                INITIATOR_FSM = INITIATOR_IDLE;
              }
            break;
          }

        REG1 = REG1 +1;
      }
  }

  void genMoore()
  {
    switch (INITIATOR_FSM)
      {
      case INITIATOR_IDLE:
        VCI_INITIATOR.CMDVAL = false;
        VCI_INITIATOR.RSPACK = false;
        break;
      case INITIATOR_REQ_WRITE:
        VCI_INITIATOR.CMDVAL = true;
        VCI_INITIATOR.RSPACK = false;
        VCI_INITIATOR.ADDRESS = 0;
        VCI_INITIATOR.WDATA = (sc_uint<32>)REG1;
        VCI_INITIATOR.CMD = VCI_CMD_WRITE;
        VCI_INITIATOR.EOP = true;
        VCI_INITIATOR.BE = 0xF;
        VCI_INITIATOR.PLEN = 1 << 2;
        break;
      case INITIATOR_RSP_WRITE:
        VCI_INITIATOR.CMDVAL = false;
        VCI_INITIATOR.RSPACK = true;
        break;
      }

  }

};

#endif
