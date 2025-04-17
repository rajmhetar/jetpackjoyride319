/* ECE319K_Lab4main.c
 * Traffic light FSM
 * ECE319H students must use pointers for next state
 * ECE319K students can use indices or pointers for next state
 * Put your names here or look silly
 Joshua Lee, Raj Mhetar
  */

#include <ti/devices/msp/msp.h>
#include "../inc/LaunchPad.h"
#include "../inc/Clock.h"
#include "../inc/UART.h"
#include "../inc/Timer.h"
#include "../inc/Dump.h"  // student's Lab 3
#include <stdio.h>
#include <string.h>


#define goS 0
#define waitS 1
#define goWalk 2
#define redW1 3
#define offW1 4
#define redW2 5
#define offW2 6
#define goW 7
#define waitW 8
#define allRedWalk 9
#define allRedW 10
#define allRedS 11
#define red 0b100
#define yellow 0b010
#define green 0b001
#define walkRed 0x04000000
#define walkWhite 0x0C400000
#define walkOff 0x0

struct State {

	uint32_t West;
  uint32_t South;
  uint32_t Walk;
  uint32_t Time;
  uint32_t Next[8];

};
typedef struct State State_t;

uint16_t stateOffset = 0;
uint8_t nextOffset = 0;

// put both EIDs in the next two lines
const char EID1[] = "jhl3263"; //  ;replace abc123 with your EID
const char EID2[] = "rvm554"; //  ;replace abc123 with your EID
// Hint implement Traffic_Out before creating the struct, make struct match your Traffic_Out
// initialize all 6 LED outputs and 3 switch inputs
// assumes LaunchPad_Init resets and powers A and B
void Traffic_Init(void){ // assumes LaunchPad_Init resets and powers A and B
//inputs
IOMUX->SECCFG.PINCM[PB15INDEX] = 0x40081; //PB15 West Input
IOMUX->SECCFG.PINCM[PB16INDEX] = 0x40081; //PB16 South Input
IOMUX->SECCFG.PINCM[PB17INDEX] = 0x40081; //PB15 Walk Input

//outputs
IOMUX->SECCFG.PINCM[PB0INDEX] = 0x81;
GPIOB->DOE31_0 |= 0x01; // PB0 South Green output; 
IOMUX->SECCFG.PINCM[PB1INDEX] = 0x81;
GPIOB->DOE31_0 |= 0x02; // PB1 South Yellow output; 
IOMUX->SECCFG.PINCM[PB2INDEX] = 0x81;
GPIOB->DOE31_0 |= 0x04; // PB2 South Red output; 
IOMUX->SECCFG.PINCM[PB6INDEX] = 0x81;
GPIOB->DOE31_0 |= 0x40; // PB6 West Green output; 
IOMUX->SECCFG.PINCM[PB7INDEX] = 0x81;
GPIOB->DOE31_0 |= 0x80; // PB7 South Yellow output; 
IOMUX->SECCFG.PINCM[PB8INDEX] = 0x81;
GPIOB->DOE31_0 |= 0x100; // PB8 South Red output; 
  
}
/* Activate LEDs
* Inputs: west is 3-bit value to three east/west LEDs
*         south is 3-bit value to three north/south LEDs
*         walk is 3-bit value to 3-color positive logic LED on PB22,PB26,PB27
* Output: none
* - west =1 sets west green
* - west =2 sets west yellow
* - west =4 sets west red
* - south =1 sets south green
* - south =2 sets south yellow
* - south =4 sets south red
* - walk=0 to turn off LED
* - walk bit 22 sets blue color
* - walk bit 26 sets red color
* - walk bit 27 sets green color
* Feel free to change this. But, if you change the way it works, change the test programs too
* Be friendly*/
void Traffic_Out(uint32_t west, uint32_t south, uint32_t walk){
  GPIOB->DOUT31_0 &= 0xF3BFFE38;
  west = west << 6;
  GPIOB->DOUT31_0 |= (west | south | walk);
}
/* Read sensors
 * Input: none
 * Output: sensor values
 * - bit 0 is west car sensor
 * - bit 1 is south car sensor
 * - bit 2 is walk people sensor
* Feel free to change this. But, if you change the way it works, change the test programs too
 */
uint32_t Traffic_In(void){
    return ((0x7)&((GPIOB->DIN31_0)>>15)); // write this
}
// use main1 to determine Lab4 assignment
void Lab4Grader(int mode);
void Grader_Init(void);
int main1(void){ // main1
  Clock_Init80MHz(0);
  LaunchPad_Init();
  Lab4Grader(0); // print assignment, no grading
  while(1){
  }
}
// use main2 to debug LED outputs
// at this point in ECE319K you need to be writing your own test functions
// modify this program so it tests your Traffic_Out  function
int main2(void){ // main2
  Clock_Init80MHz(0);
  LaunchPad_Init();
  Grader_Init(); // execute this line before your code
  LaunchPad_LED1off();
  Traffic_Init(); // your Lab 4 initialization
  if((GPIOB->DOE31_0 & 0x20)==0){
    UART_OutString("access to GPIOB->DOE31_0 should be friendly.\n\r");
  }
  UART_Init();
  UART_OutString("Lab 4, Spring 2025, Step 1. Debug LEDs\n\r");
  UART_OutString("EID1= "); UART_OutString((char*)EID1); UART_OutString("\n\r");
  UART_OutString("EID2= "); UART_OutString((char*)EID2); UART_OutString("\n\r");
  while(1){
    Traffic_Out(1, 0, 0);
    Debug_Dump(GPIOB->DOUT31_0);
    Traffic_Out(2, 0, 0);
    Debug_Dump(GPIOB->DOUT31_0);
    Traffic_Out(4, 0, 0);
    Debug_Dump(GPIOB->DOUT31_0);
    Traffic_Out(0, 1, 0);
    Debug_Dump(GPIOB->DOUT31_0);
    Traffic_Out(0, 2, 0);
    Debug_Dump(GPIOB->DOUT31_0);
    Traffic_Out(0, 4, 0);
    Debug_Dump(GPIOB->DOUT31_0);
    Traffic_Out(0, 0, (67108864));
    Debug_Dump(GPIOB->DOUT31_0);
    Traffic_Out(0, 0, (134217728 | 67108864 | 4194304));
    Debug_Dump(GPIOB->DOUT31_0);
    if((GPIOB->DOUT31_0&0x20) == 0){
      UART_OutString("DOUT not friendly\n\r");
    }
  }
}
// use main3 to debug the three input switches
// at this point in ECE319K you need to be writing your own test functions
// modify this program so it tests your Traffic_In  function
int main3(void){ // main3
  uint32_t last=0,now;
  Clock_Init80MHz(0);
  LaunchPad_Init();
  Traffic_Init(); // your Lab 4 initialization
  Debug_Init();   // Lab 3 debugging
  UART_Init();
  __enable_irq(); // UART uses interrupts
  UART_OutString("Lab 4, Spring 2025, Step 2. Debug switches\n\r");
  UART_OutString("EID1= "); UART_OutString((char*)EID1); UART_OutString("\n\r");
  UART_OutString("EID2= "); UART_OutString((char*)EID2); UART_OutString("\n\r");
  while(1){
    now = Traffic_In(); // Your Lab4 input
    if(now != last){ // change
      UART_OutString("Switch= 0x"); UART_OutUHex(now); UART_OutString("\n\r");
      Debug_Dump(now);
    }
    last = now;
    Clock_Delay(800000); // 10ms, to debounce switch
  }
}
// use main4 to debug using your dump
// proving your machine cycles through all states
int main4(void){// main4
uint32_t input;
  Clock_Init80MHz(0);
  LaunchPad_Init();
  LaunchPad_LED1off();
  Traffic_Init(); // your Lab 4 initialization
 // set initial state
  Debug_Init();   // Lab 3 debugging
  UART_Init();
  __enable_irq(); // UART uses interrupts
  UART_OutString("Lab 4, Spring 2025, Step 3. Debug FSM cycle\n\r");
  UART_OutString("EID1= "); UART_OutString((char*)EID1); UART_OutString("\n\r");
  UART_OutString("EID2= "); UART_OutString((char*)EID2); UART_OutString("\n\r");
// initialize your FSM
  State_t FSM[12] = {
    {red, green, walkRed, 200, {goS, waitS, goS, waitS, waitS, waitS, waitS, waitS}}, //goS
    {red, yellow, walkRed, 50, {allRedS, allRedW, allRedS, allRedW, allRedWalk, allRedWalk, allRedWalk, allRedWalk}}, //waitS
    {red, red, walkWhite, 200, {goWalk, redW1, redW1, redW1, goWalk, redW1, redW1, redW1}}, //goWalk
    {red, red, walkRed, 50, {offW1, offW1, offW1, offW1, offW1, offW1, offW1, offW1}}, //redW1
    {red, red, walkOff, 50, {redW2, redW2, redW2, redW2, redW2, redW2, redW2, redW2}}, //offW1
    {red, red, walkRed, 50, {offW2, offW2, offW2, offW2, offW2, offW2, offW2, offW2}}, //redW2
    {red, red, walkOff, 50, {allRedWalk, allRedW, allRedS, allRedS, allRedWalk, allRedW, allRedS, allRedW}}, //offW1
    {green, red, walkRed, 200, {goW, goW, waitW, waitW, waitW, waitW, waitW, waitW}}, //goW
    {yellow, red, walkRed, 50, {allRedW, allRedW, allRedS, allRedS, goWalk, goWalk, allRedS, allRedS}}, //waitW
    {red, red, walkRed, 50, {goWalk, goWalk, goWalk, goWalk, goWalk, goWalk, goWalk, goWalk}}, //allRedWalk
    {red, red, walkRed, 50, {goW, goW, goW, goW, goW, goW, goW, goW}}, //allRedW
    {red, red, walkRed, 50, {goS, goS, goS, goS, goS, goS, goS, goS}}, //allRedS
  };
  SysTick_Init();   // Initialize SysTick for software waits

  
  while(1){
      // 1) output depending on state using Traffic_Out
      // call your Debug_Dump logging your state number and output
      State_t* structPtr = FSM + stateOffset;
      Traffic_Out(structPtr->West, structPtr->South, structPtr->Walk);
      uint32_t debugDump = 0;
      debugDump |= (stateOffset<<24);
      debugDump |= ((structPtr->West)<<16);
      debugDump |= ((structPtr->South)<<8);
      debugDump |= ((structPtr->Walk));
      Debug_Dump(debugDump);
      // 2) wait depending on state
      SysTick_Wait10ms((structPtr)->Time);
      // 3) hard code this so input always shows all switches pressed
      nextOffset = 0x7;
      // 4) next depends on state and input
      stateOffset = *((structPtr->Next) + nextOffset);
  }
}
// use main5 to grade
int main(void){// main5
  Clock_Init80MHz(0);
  LaunchPad_Init();
  Grader_Init(); // execute this line before your code
  LaunchPad_LED1off();
  Traffic_Init(); // your Lab 4 initialization
// initialize your FSM
  SysTick_Init();   // Initialize SysTick for software waits
  // initialize your FSM
  State_t FSM[12] = {
    {red, green, walkRed, 200, {goS, waitS, goS, waitS, waitS, waitS, waitS, waitS}}, //goS
    {red, yellow, walkRed, 50, {allRedS, allRedW, allRedS, allRedW, allRedWalk, allRedWalk, allRedWalk, allRedWalk}}, //waitS
    {red, red, walkWhite, 200, {goWalk, redW1, redW1, redW1, goWalk, redW1, redW1, redW1}}, //goWalk
    {red, red, walkRed, 50, {offW1, offW1, offW1, offW1, offW1, offW1, offW1, offW1}}, //redW1
    {red, red, walkOff, 50, {redW2, redW2, redW2, redW2, redW2, redW2, redW2, redW2}}, //offW1
    {red, red, walkRed, 50, {offW2, offW2, offW2, offW2, offW2, offW2, offW2, offW2}}, //redW2
    {red, red, walkOff, 50, {allRedWalk, allRedW, allRedS, allRedS, allRedWalk, allRedW, allRedS, allRedW}}, //offW1
    {green, red, walkRed, 200, {goW, goW, waitW, waitW, waitW, waitW, waitW, waitW}}, //goW
    {yellow, red, walkRed, 50, {allRedW, allRedW, allRedS, allRedS, goWalk, goWalk, allRedS, allRedS}}, //waitW
    {red, red, walkRed, 50, {goWalk, goWalk, goWalk, goWalk, goWalk, goWalk, goWalk, goWalk}}, //allRedWalk
    {red, red, walkRed, 50, {goW, goW, goW, goW, goW, goW, goW, goW}}, //allRedW
    {red, red, walkRed, 50, {goS, goS, goS, goS, goS, goS, goS, goS}}, //allRedS
  };
  Lab4Grader(1); // activate UART, grader and interrupts
  while(1){
      // 1) output depending on state using Traffic_Out
      // call your Debug_Dump logging your state number and output
      State_t* structPtr = FSM + stateOffset;
      Traffic_Out(structPtr->West, structPtr->South, structPtr->Walk);
      uint32_t debugDump = 0;
      debugDump |= (stateOffset<<24);
      debugDump |= ((structPtr->West)<<16);
      debugDump |= ((structPtr->South)<<8);
      debugDump |= ((structPtr->Walk));
      Debug_Dump(debugDump);
      // 2) wait depending on state
      SysTick_Wait10ms((structPtr)->Time);
      // 3) hard code this so input always shows all switches pressed
      nextOffset = Traffic_In();
      // 4) next depends on state and input
      stateOffset = *((structPtr->Next) + nextOffset);
  }
}

