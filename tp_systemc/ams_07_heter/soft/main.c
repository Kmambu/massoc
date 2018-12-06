
#include <stdio.h>
#include "system.h"
#include "../segmentation.h"
#include "../coords.h"
#include "soclib/timer.h"
#include "soclib/uart.h"
#include "soclib/sigma_delta.h"
#include "epicentre.h"

static const int period = 99999;
volatile uint32_t id;
volatile uint32_t sensor_cpt;
volatile uint32_t has_value;
volatile uint32_t tdma_slot;
volatile uint32_t tdma_trigger;

void irq_handler(int irq)
{
  uint32_t d;

  switch (irq) {
  case 0:
    sensor_cpt=soclib_io_get(base(SENSOR), SD_COUNTER);
    printf("IRQ received...\n");
    has_value=1;
    disable_hw_irq(0);
    break;
  case 1:
    d=soclib_io_get(base(UART),
		    UART_DATA);
    printf("data %d received from node %d\n",d & 0x0fffffff,(d>>28)&0xf);
    remplir_table((d>>28)&0xf, d & 0x0fffffff);
    break;
  case 2:
    d = soclib_io_get(
		      base(TIMER),
		      0*TIMER_SPAN+TIMER_VALUE);
    soclib_io_set(
		  base(TIMER),
		  0*TIMER_SPAN+TIMER_RESETIRQ,
		  0);
    tdma_slot=(d/100000)%4;
    tdma_trigger=1;
    break;
  }
}

void init() {
  sensor_acquired=0;
  has_value=0;
  flag_calcul=0;
  tdma_trigger=0;

  set_cp0(12,0,get_cp0(12,0) | 1<<29); //activation fpu (bit cu1 du reg de status)
  
  //uart config
  soclib_io_set(base(UART), UART_BR, 500); //baudrate 500
  soclib_io_set(base(UART), UART_CTRL, 5); // 5 : reception et irq
  id = soclib_io_get(base(UART), UART_ID); //uart_id is the ID of the node

  //timer config
  soclib_io_set(
                base(TIMER),
                0*TIMER_SPAN+TIMER_PERIOD,
                period);
  soclib_io_set(
                base(TIMER),
                0*TIMER_SPAN+TIMER_MODE,
                TIMER_RUNNING|TIMER_IRQ_ENABLED);

  //enabling mips irq 0, 1 & 2
  set_irq_handler(irq_handler);
  enable_hw_irq(0);
  enable_hw_irq(1);
  enable_hw_irq(2);
  irq_enable();
}

int main(int argc, char**argv)
{
  uint32_t data;

  init();

  printf("Sensor %d is alive...\n", id);

  while (1){
    if (tdma_trigger==1) {
      tdma_trigger=0;
      if(!flag_calcul){
	if (tdma_slot==id) {
	  if(has_value) {
	    soclib_io_set(base(UART), UART_CTRL, 2); //rf emission
	    data=(id<<28) | (sensor_cpt & 0x0fffffff );
	    printf("sends data %x\n",data);
	    soclib_io_set(base(UART),
			  UART_DATA,
			  data);
	    remplir_table(id,sensor_cpt);
	    has_value=0;
	  }
	}
	else
	  soclib_io_set(base(UART),UART_CTRL,5); //rf reception
      }
      else {
	irq_disable();
	calcule_epicentre();
      }
    }
  }
	  
  return 0;
}
