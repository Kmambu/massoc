/*
 *
 * SOCLIB_GPL_HEADER_BEGIN
 *
 * This file is part of SoCLib, GNU GPLv2.
 *
 * SoCLib is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * SoCLib is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SoCLib; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 * SOCLIB_GPL_HEADER_END
 *
 * Copyright (c) UPMC, Lip6, SoC
 *         Nicolas Pouillon <nipo@ssji.net>, 2006-2007
 *
 * Maintainers: nipo
 */

#include "system.h"

#include "../segmentation.h"

#include "soclib/timer.h"
#include "soclib/icu.h"
#include <stdio.h>

static const int period = 300000;
volatile static uint32_t cptms=0;
volatile static uint32_t irq_flag;

void irq_handler(int irq)
{
  uint32_t iv;
  uint32_t ti;
  uint32_t d;

  iv = soclib_io_get(base(ICU),
                     ICU_IT_VECTOR);

  switch (iv)
    {
    case 0:
      cptms++;
      cptms=cptms%10;
      printf("irq cptms=%d\n", cptms);

      ti = soclib_io_get(base(TIMER),
                         0*TIMER_SPAN+TIMER_VALUE);
      //printf("IRQ %d received at cycle %d on cpu %d\n", irq, ti, procnum());
      soclib_io_set(base(TIMER),
                    0*TIMER_SPAN+TIMER_RESETIRQ,
                    0);
      irq_flag=1;
      break;
    }
}

int main(void)
{
  irq_flag=0;

  set_irq_handler(irq_handler);

  printf("Hello from node 0\n");

  soclib_io_set(base(ICU),
                ICU_MASK_SET,
                1);

  soclib_io_set(base(TIMER),
                0*TIMER_SPAN+TIMER_PERIOD,
                period);
  soclib_io_set(base(TIMER),
                0*TIMER_SPAN+TIMER_MODE,
                TIMER_RUNNING|TIMER_IRQ_ENABLED);

  enable_hw_irq(0);
  irq_enable();

  while (1)
    {
      if (irq_flag==1)
        {
          irq_flag=0;
        }
    }

  return 0;
}
