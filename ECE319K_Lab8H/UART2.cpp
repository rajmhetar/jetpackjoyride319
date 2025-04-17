/* UART2.cpp
 * Your name
 * Data:
 * PA22 UART2 Rx from other microcontroller PA8 IR output<br>
 */


#include <ti/devices/msp/msp.h>
#include "UART2.h"
#include "../inc/Clock.h"
#include "../inc/LaunchPad.h"
#include "../inc/FIFO2.h"

uint32_t LostData;
Queue FIFO2;

// power Domain PD0
// for 80MHz bus clock, UART2 clock is ULPCLK 40MHz
// initialize UART2 for 2375 baud rate
// no transmit, interrupt on receive timeout
void UART2_Init(void){
  LostData = 0;
    // RSTCLR to GPIOA and UART2 peripherals
   // write this
   UART2->GPRCM.RSTCTL = 0xB1000003; // reset UART2
   UART2->GPRCM.PWREN = 0x26000001; // activate UART2
   Clock_Delay(24); // time for uart to activate
//bit 7 PC connected
//bits 5-0=2 for UART0_Tx
   IOMUX->SECCFG.PINCM[PA22INDEX] = 0x00040082;
//bit 18 INENA input enable
//bit 7 PC connected
//bits 5-0=2 for UART0_Rx
   UART2->CLKSEL = 0x08; // bus clock
   UART2->CLKDIV = 0x00; // no divide
   UART2->CTL0 &= ~0x01; // disable UART2
   UART2->CTL0 = 0x00020018; // enable fifos, tx and rx
// 40000000/16 = 2,500,000, 2,500,000/2375 = 1052.63157895
   UART2->IBRD = 1052;
   UART2->FBRD = 40;
   UART2->LCRH = 0x00000030; // 8bit, 1 stop, no parity

       // Set FIFO level select for receiver timeout:
    // Clear RXTOSEL bits (bits 11-8) and set them to 4.
    // This causes an interrupt to occur 4 bit times after a message is received.
  UART2->IFLS = (UART2->IFLS & ~0xF00) | (4 << 8);

   UART2->CTL0 |= 0x01; // enable UART2
}


//------------UART2_InChar------------
// Get new serial port receive data from FIFO2
// Input: none
// Output: Return 0 if the FIFO2 is empty
//         Return nonzero data from the FIFO1 if available
char UART2_InChar(void){char out;
// write this
  if (FIFO2.Get(&out)) {
    return out;
  }
  return 0;
}

uint32_t RxCounter = 0;

extern "C" void UART2_IRQHandler(void);
void UART2_IRQHandler(void){ uint32_t status; char letter;
  status = UART2->CPU_INT.IIDX; // reading clears bit in RTOUT
  if(status == 0x01){   // 0x01 receive timeout
    GPIOB->DOUTTGL31_0 = BLUE; // toggle PB22 (minimally intrusive debugging)
    GPIOB->DOUTTGL31_0 = BLUE; // toggle PB22 (minimally intrusive debugging)
    // read all data, putting in FIFO
    // finish writing this
    
while ((UART2->STAT & (1 << 2)) == 0) {
  letter = UART2->RXDATA;

  if (!FIFO2.Put(letter)) {
    LostData++;
  }
}
  RxCounter++;
    GPIOB->DOUTTGL31_0 = BLUE; // toggle PB22 (minimally intrusive debugging)
  }
}
