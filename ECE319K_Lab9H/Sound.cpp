// Sound.cpp
// Runs on MSPM0
// Sound assets in sounds/sounds.h
// your name
// your data 
#include <stdint.h>
#include <ti/devices/msp/msp.h>
#include "Sound.h"
#include "sounds/sounds.h"
#include "../inc/DAC5.h"
#include "../inc/Timer.h"

// Global variables for sound playback
const uint8_t *soundPtr;  // Pointer to current sound data
uint32_t soundCount;      // Number of samples remaining
uint32_t soundLength;     // Total length of sound

void SysTick_IntArm(uint32_t period, uint32_t priority){
  // Configure SysTick for 11kHz playback
  SysTick->CTRL = 0;         // Disable SysTick
  SysTick->LOAD = period-1;  // Set reload value
  SysTick->VAL = 0;          // Clear current value
  SysTick->CTRL = 0x00000007; // Enable SysTick with core clock and interrupts
  NVIC_SetPriority(SysTick_IRQn, priority); // Set interrupt priority
}

// initialize a 11kHz SysTick, however no sound should be started
// initialize any global variables
// Initialize the 5-bit DAC
void Sound_Init(void){
  DAC5_Init();  // Initialize 5-bit DAC
  soundPtr = 0; // No sound playing initially
  soundCount = 0;
  soundLength = 0;
}

extern "C" void SysTick_Handler(void);
void SysTick_Handler(void){ // called at 11 kHz
  // output one value to DAC if a sound is active
  if(soundCount > 0){
    DAC5_Out(*soundPtr); // Output current sample
    soundPtr++;          // Move to next sample
    soundCount--;        // Decrement remaining samples
  }
}

//******* Sound_Start ************
// This function does not output to the DAC. 
// Rather, it sets a pointer and counter, and then enables the SysTick interrupt.
// It starts the sound, and the SysTick ISR does the output
// feel free to change the parameters
// Sound should play once and stop
// Input: pt is a pointer to an array of DAC outputs
//        count is the length of the array
// Output: none
// special cases: as you wish to implement
void Sound_Start(const uint8_t *pt, uint32_t count){
  soundPtr = pt;      // Set pointer to start of sound
  soundCount = count; // Set number of samples to play
  soundLength = count;
  SysTick_IntArm(80000000/11025, 1); // Configure SysTick for 11kHz
}

// Individual sound functions
void Sound_Shoot(void){
  Sound_Start(shoot, 4080);
}

void Sound_Coin(void){
  Sound_Start(coin_recieved_230517, 36780);
}

