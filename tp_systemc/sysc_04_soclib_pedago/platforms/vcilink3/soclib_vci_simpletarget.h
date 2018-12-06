#ifndef SOCLIB_VCI_SIMPLETARGET_H
#define SOCLIB_VCI_SIMPLETARGET_H

#define sc_register sc_signal

#include <signal.h>
//#include <iostream.h>
//#include <fstream.h>
#include <stdlib.h>
#include <systemc.h>

template<
  int ADDRSIZE,
  int CELLSIZE,
  int ERRSIZE,
  int PLENSIZE,
  int CLENSIZE,
  int SRCIDSIZE,
  int TRDIDSIZE,
  int PKTIDSIZE >

struct SOCLIB_VCI_SIMPLETARGET : sc_module {

  sc_in<bool> CLK;
  sc_in<bool> RESETN;
  ADVANCED_VCI_TARGET<ADDRSIZE, CELLSIZE, ERRSIZE, PLENSIZE, CLENSIZE, SRCIDSIZE, TRDIDSIZE, PKTIDSIZE > VCI_TARGET;

  const char *NAME;
  sc_register<int> TARGET_FSM;
  sc_register<int> REG1;

  enum{
    TARGET_IDLE = 0,
    TARGET_RSP  = 1,
    TARGET_EOP  = 2
  };

  enum{
    REG1_ADR = 0,
  };

  SC_HAS_PROCESS(SOCLIB_VCI_SIMPLETARGET);

  SOCLIB_VCI_SIMPLETARGET(
                          sc_module_name insname // nom de l'instance
                          )
  {
#ifdef NONAME_RENAME
    TARGET_FSM.rename("TARGET_FSM");
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
    printf("SOCLIB_VCI_SIMPLETARGET instanciated with name %s\n",NAME);

  } // fin fonction de creation

  void transition()
  {
    if(RESETN == false)
      {
        TARGET_FSM = TARGET_IDLE;
      } else
      {
        switch(TARGET_FSM)
          {
          case TARGET_IDLE :
            if(VCI_TARGET.CMDVAL == true)
              {
                if(VCI_TARGET.EOP == true)
                  {
                    TARGET_FSM = TARGET_EOP;
                  }
                else
                  {
                    TARGET_FSM = TARGET_RSP;
                  }
                if ((VCI_TARGET.CMD.read() == VCI_CMD_WRITE) &&
                    ((VCI_TARGET.ADDRESS.read() & 0xC) == REG1_ADR))
                  {
                    REG1 = (sc_uint<32>)VCI_TARGET.WDATA;
                    printf("written value %d\n",(int)VCI_TARGET.WDATA.read());
                  }
              }
            break;
          case TARGET_RSP :
            if(VCI_TARGET.RSPACK == true)
              {
                TARGET_FSM = TARGET_IDLE;
              }
            break;
          case TARGET_EOP :
            if(VCI_TARGET.RSPACK == true)
              {
                TARGET_FSM = TARGET_IDLE;
              }
            break;
          }

      }
  }

  void genMoore()
  {
    switch (TARGET_FSM)
      {
      case TARGET_IDLE:
        VCI_TARGET.CMDACK = true;
        VCI_TARGET.RSPVAL = false;
        break;
      case TARGET_RSP:
        VCI_TARGET.CMDACK = false;
        VCI_TARGET.RSPVAL = true;
        VCI_TARGET.RDATA  = 0;
        VCI_TARGET.RERROR = 0;
        VCI_TARGET.REOP = false;
        break;
      case TARGET_EOP:
        VCI_TARGET.CMDACK = false;
        VCI_TARGET.RSPVAL = true;
        VCI_TARGET.RDATA  = 0;
        VCI_TARGET.RERROR = 0;
        VCI_TARGET.REOP = true;
        break;
      }
  }

};
#endif
