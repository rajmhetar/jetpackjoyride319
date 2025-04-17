/* IRxmt.cpp
 * Your name
 * Date:
 * PA8 GPIO output to IR LED, to other microcontroller IR sensor, PA22 UART2 Rx
 */


#include <cstdint>
#include <ti/devices/msp/msp.h>
#include "IRxmt.h"


#define PA8INDEX  18 // UART1_TX  SPI0_CS0  UART0_RTS TIMA0_C0  TIMA1_C0N
#define IR (1<<8)
#define BAUD 2375  // bps
#define PulsePerBit (38000/2375) // 16 pulses each bit
// to get  38kHz wave, look at output on scope and
// adjust the constant IRPULSE to get 38kHz output on PA8
#define IRPULSE 338


// Initialize GPIO on PA8
// Baud rate=2375 evenly divides into 38,000
// 38 kHz is 26.315us period
// when pulsing, 38 kHz wave is on for 13.158us, off for 13.158us
void IRxmt_Init(void){
  IOMUX->SECCFG.PINCM[PA8INDEX] = 0x00000081;  // 0x81 means GPIO with software control
  GPIOA->DOE31_0 |= IR;                       // enable output
  GPIOA->DOUTCLR31_0 = IR;                    // output low (IR off)
}


extern "C" void Delay(void);
void Delay(uint32_t time){
#ifdef __GNUC__
    __asm(".syntax unified");
#endif
    __asm volatile(
"Delay_Loop: SUBS  R0, R0, #1; \n"
"            BNE   Delay_Loop; \n"
    );
}


// baud rate = 2375 bps
// bit time = 1/2375 = 421.05us
// 16 pulses per bit (receiver needs at least 10 pulses to decode IR signal)
// each pulse is 421.06us/16 = 26.315us
// negative logic: 38KHz IR pulses occur with bit=0
// if bit=0 the PA8 pulses 16 times at on for 13.158us, off for 13.158us
// if bit=1 no PA8 pulses, 16 times at off for 13.158us, off for 13.158us
void IRxmt_SendBit(int bit){
   // write this
    for(uint32_t i = 0; i < 16; i++){
        if(bit == 0){
            GPIOA->DOUTTGL31_0 = IR; // toggle PA8
            Delay(IRPULSE);          // ~13.16 us
        }  
    else {
            GPIOA->DOUTCLR31_0 = IR; // force PA8 low (no IR light)
            Delay(IRPULSE);          // still wait same time to match duration
        }
    }
}
// output ASCII character to IR transmitter
// blind cycle synchronization
// in Lab 8 this will wait until transmission is done
// start=0,bit0,bit1,bit2,bit2,bit4,bit5,bit6,bit7,stop=1
// 0 means 38 kHz pulse for 1 bit time
// 1 means no pulses for 1 bit time
// should take 10*421.06us = 4.2106ms
void IRxmt_OutChar(char data){
     // write this
     //first send a 0
     IRxmt_SendBit(0);
     for(int mask = 0; mask < 8; mask++){
        IRxmt_SendBit((data >> mask) & 0x01);
     }
     IRxmt_SendBit(1);
}





