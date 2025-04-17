// Dump.c
// Your solution to ECE319K Lab 3 Spring 2025
// Author: Joshua Lee
// Last Modified: 2/9/2025


#include <ti/devices/msp/msp.h>
#include "../inc/Timer.h"
#define MAXBUF 50
uint32_t DataBuffer[MAXBUF];
uint32_t TimeBuffer[MAXBUF];
uint32_t DebugCnt; // 0 to MAXBUF (0 is empty, MAXBUF is full)
uint32_t Debug_Dump2_Called = 0;

// *****Debug_Init******
// Initializes your index or pointer.
// Input: none
// Output:none
void Debug_Init(void) 
{
// students write this for Lab 3
// This function should also initialize Timer G12, call TimerG12_Init.
  for(int a = 1; a < DebugCnt; a++)
  {
    DataBuffer[a] = 0;
    TimeBuffer[a] = 0;
  }
  DebugCnt = 0;
  TimerG12_Init();
}

// *****Debug_Dump******
// Records one data and one time into the two arrays.
// Input: data is value to store in DataBuffer
// Output: 1 for success, 0 for failure (buffers full)
uint32_t Debug_Dump(uint32_t data) {
// students write this for Lab 3
// The software simply reads TIMG12->COUNTERREGS.CTR to get the current time in bus cycles.
  int success = 1;
  if (DebugCnt >= MAXBUF)
  {
    success = 0;
  }
  else 
  {
    DataBuffer[DebugCnt] = data;
    TimeBuffer[DebugCnt] = TIMG12->COUNTERREGS.CTR;
    DebugCnt++;
  }
  return success; // success
}
// *****Debug_Dump2******
// Always record data and time on the first call to Debug_Dump2
// However, after the first call
//    Records one data and one time into the two arrays, only if the data is different from the previous call.
//    Do not record data or time if the data is the same as the data from the previous call
// Input: data is value to store in DataBuffer
// Output: 1 for success (saved or skipped), 0 for failure (buffers full)
uint32_t Debug_Dump2(uint32_t data) {
// students write this for Lab 3
// The software simply reads TIMG12->COUNTERREGS.CTR to get the current time in bus cycles.
  int success = 1;
  
  if (Debug_Dump2_Called == 0) 
  {
    if (DebugCnt >= MAXBUF)
    {
      success = 0;
    }  
    else 
    {
      DataBuffer[DebugCnt] = data;
      TimeBuffer[DebugCnt] = TIMG12->COUNTERREGS.CTR;
      DebugCnt++;
    }
  }
  else 
  {
    if (DebugCnt >= MAXBUF)
    {
      success = 0;
    }  
    else if (DebugCnt > 0 && DataBuffer[DebugCnt - 1] != data)
    {
      DataBuffer[DebugCnt] = data;
      TimeBuffer[DebugCnt] = TIMG12->COUNTERREGS.CTR;
      DebugCnt++;
    }
  }
  if (success)
  {
  Debug_Dump2_Called = 1;
  }
  return success;
}

// *****Debug_Period******
// Calculate period of the recorded data using mask
// Input: mask specifies which bit(s) to observe
// Output: period in bus cycles
// Period is defined as rising edge (low to high) to the next rising edge.
// Return 0 if there is not enough collected data to calculate period .
uint32_t Debug_Period(uint32_t mask){
// students write this for Lab 3
// This function should not alter the recorded data.
// AND each recorded data with mask,
//    if nonzero the signal is considered high.
//    if zero, the signal is considered low.
  int averagePeriod = 0;
  
  if (DebugCnt >= 3)
  {
    int prevState = 0;
    int prevRise = 0;
    int nextRise = 0;
    int count = 0;
    int total = 0;
    for (int i = 0; i < DebugCnt - 1; i++)
    {
      if (mask & DataBuffer[i])
      {
        if (prevState == 0)
        {
          if (count == 0)
          {
            prevRise = TimeBuffer[i];
            prevState = 1;
            count++;
          }
          else 
          {
            nextRise = TimeBuffer[i];
            if (nextRise > prevRise)
            {
              total += prevRise + (0xFFFF - nextRise);
            }
            else 
            {
              total += prevRise - nextRise;
            }
            count++;
            prevState = 1;
            prevRise = nextRise;
          }
        }
      }
      else 
      {
          prevState = 0;
      }
    }
    count -= 1;
    averagePeriod = total/count;
  } 
  return averagePeriod; // average period
}


// *****Debug_Duty******
// Calculate duty cycle of the recorded data using mask
// Input: mask specifies which bit(s) to observe
// Output: period in percent (0 to 100)
// Period is defined as rising edge (low to high) to the next rising edge.
// High is defined as rising edge (low to high) to the next falling edge.
// Duty cycle is (100*High)/Period
// Return 0 if there is not enough collected data to calculate duty cycle.
uint32_t Debug_Duty(uint32_t mask)
{
// students write this for Lab 3
// This function should not alter the recorded data.
// AND each recorded data with mask,
//    if nonzero the signal is considered high.
//    if zero, the signal is considered low.
  int dutyCycle = 0;
  double dutyCycleTemp = 0.0;
  int averagePeriod = Debug_Period(mask);
  if (averagePeriod != 0)
  {
    int prevState = 0;
    int highTotal = 0;
    int lowTotal = 0;
    int prevChange = 0;
    int nextChange = 0;
    for (int i = 0; i < DebugCnt - 1; i++)
    {
      if (mask & DataBuffer[i])
      {
        if (i == 0)
        {
          prevChange = TimeBuffer[i];
          prevState = 1;
        }
        else 
        {
          if (prevState == 0) // 0 -> 1
          {
            nextChange = TimeBuffer[i];
            if (nextChange > prevChange)
            {
              lowTotal += prevChange + (0xFFFF - nextChange);
            }
            else 
            {
              lowTotal += prevChange - nextChange;
            }
            prevChange = nextChange;
            prevState = 1;
          }
          else //1 -> 1
          {
            prevState = 1;
          }
        }
      }
      else 
      {
        if (i == 0)
        {
          prevChange = TimeBuffer[i];
          prevState = 0;
        }
        else 
        {
          if (prevState == 1) // 1 -> 0
          {
            nextChange = TimeBuffer[i];
            if (nextChange > prevChange)
            {
              highTotal += prevChange + (0xFFFF - nextChange);
            }
            else 
            {
              highTotal += prevChange - nextChange;
            }
            prevChange = nextChange;
            prevState = 0;
          }
          else //0 -> 0
          {
            prevState = 0;
          }
        }
      }
    }
    dutyCycleTemp = ((double)(highTotal))/(highTotal + lowTotal);
     dutyCycle = (int)(100.0*dutyCycleTemp);
    }
  return dutyCycle;
}

// Lab2 specific debugging code
uint32_t Theperiod;
uint32_t Thedutycycle;
void Dump(void){
  uint32_t out = GPIOB->DOUT31_0&0x0070000; // PB18-PB16 outputs
  uint32_t in = GPIOB->DIN31_0&0x0F;        // PB3-PB0 inputs
  uint32_t data = out|in;                   // PB18-PB16, PB3-PB0
  uint32_t result = Debug_Dump(data);       // calls your Lab3 function
  if(result == 0){ // 0 means full
    Theperiod = Debug_Period(1<<17);        // calls your Lab3 function
    Thedutycycle = Debug_Duty(1<<17);
   __asm volatile("bkpt; \n"); // breakpoint here
// observe Theperiod
  }
}




