#ifndef SOCLIB_VCI_SIMPLERAM_H
#define SOCLIB_VCI_SIMPLERAM_H

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
struct SOCLIB_VCI_SIMPLERAM : sc_module {

  /*** Declaration des interfaces ***/
  sc_in<bool> CLK;
  sc_in<bool> RESETN;
  ADVANCED_VCI_TARGET<ADDRSIZE, CELLSIZE, ERRSIZE, PLENSIZE, CLENSIZE, SRCIDSIZE, TRDIDSIZE, PKTIDSIZE > VCI_TARGET;

  /*** Instance name ***/
  const char *NAME;
  sc_register<int>	TARGET_FSM,DT,REG_EOP,WRITE_COUNTER,READ_COUNTER;
  sc_register<int>	WRITE_COUNTER_INIT,READ_COUNTER_INIT;
  int mem[1024];

  enum{
    TARGET_IDLE = 0,
    TARGET_WRITE_WAIT,
    TARGET_WRITE,
    TARGET_READ_WAIT,
    TARGET_READ,
  };

  /*** Constructeur et Destructeur ***/
  SC_HAS_PROCESS(SOCLIB_VCI_SIMPLERAM);

  SOCLIB_VCI_SIMPLERAM(
                       sc_module_name insname // nom de l'instance
                       )
  {
#ifdef NONAME_RENAME
    TARGET_FSM.rename("TARGET_FSM");
    DT.rename("DT");
    REG_EOP.rename("REG_EOP");
    WRITE_COUNTER.rename("WRITE_COUNTER");
    WRITE_COUNTER_INIT.rename("WRITE_COUNTER_INIT");
    READ_COUNTER.rename("READ_COUNTER");
    READ_COUNTER_INIT.rename("READ_COUNTER_INIT");
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
    printf("SOCLIB_VCI_SIMPLERAM instanciated with name %s\n",NAME);

    mem[0x00 >> 2]=0x20010010;	// addi $1,$0, 0x10
    mem[0x04 >> 2]=0x20020014;	// addi $2,$0, 0x14
    mem[0x08 >> 2]=0x8c220000;	// lw $2,0($1) = 0x11223344
    mem[0x0C >> 2]=0x8c230004;	// lw $3,4($1) = 0x55667788
    mem[0x10 >> 2]=0x11223344;
    mem[0x14 >> 2]=0x55667788;
    WRITE_COUNTER_INIT=3;
    READ_COUNTER_INIT=3;
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
                REG_EOP= VCI_TARGET.EOP;
                if (VCI_TARGET.CMD.read() == VCI_CMD_WRITE)
                  {
                    int addr=(int)VCI_TARGET.ADDRESS.read();
                    int wdata=(int)VCI_TARGET.WDATA.read();
                    mem[addr>>2]=wdata;
                    TARGET_FSM = TARGET_WRITE_WAIT;
                    WRITE_COUNTER = WRITE_COUNTER_INIT;
                  }
                else
                  {
                    int addr=(int)VCI_TARGET.ADDRESS.read();
                    DT = mem[addr>>2];

                    TARGET_FSM = TARGET_READ_WAIT;
                    READ_COUNTER = READ_COUNTER_INIT;
                  }
              }
            break;
          case TARGET_WRITE_WAIT :
            WRITE_COUNTER=WRITE_COUNTER-1;
            if (WRITE_COUNTER==1)
              TARGET_FSM=TARGET_WRITE;
            break;
          case TARGET_READ_WAIT :
            READ_COUNTER=READ_COUNTER-1;
            if (READ_COUNTER==1)
              TARGET_FSM=TARGET_READ;
            break;
          case TARGET_READ :
          case TARGET_WRITE :
            if(VCI_TARGET.RSPACK == true)
              {
                TARGET_FSM = TARGET_IDLE;
              }
            break;
          } // end switch TARGET FSM
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
      case TARGET_WRITE_WAIT:
        VCI_TARGET.CMDACK = false;
        VCI_TARGET.RSPVAL = false;
        break;
      case TARGET_READ_WAIT:
        VCI_TARGET.CMDACK = false;
        VCI_TARGET.RSPVAL = false;
        break;
      case TARGET_WRITE:
        VCI_TARGET.CMDACK = false;
        VCI_TARGET.RSPVAL = true;
        VCI_TARGET.RDATA  = 0;
        VCI_TARGET.RERROR = 0;
        VCI_TARGET.REOP = REG_EOP;
        break;
      case TARGET_READ:
        VCI_TARGET.CMDACK = false;
        VCI_TARGET.RSPVAL = true;
        VCI_TARGET.RDATA  = (sc_uint<32>) DT;
        VCI_TARGET.RERROR = 0;
        VCI_TARGET.REOP = REG_EOP;
        break;
      }
  }

};
#endif
