/*!
 * @defgroup InputCapture
 * @brief Input capture measures period
 <table>
<caption id="ICpins">Input capture pins </caption>
<tr><th>Pin <th>Timer<th>Description
<tr><td>PA12<td>TIMG0<td>interrupts on rising edge of PA12   
</table>
 * @{*/
/**
 * @file      InputCapture.h
 * @brief     Software driver functions for input capture
 * @details   PA12 is digital wave input,  index 33 in IOMUX PINCM table.<br>
 * - G0 in power domain PD0, so
 *    - for 32MHz bus clock, SYSCLK clock is 32MHz
 *    - for 40MHz bus clock, SYSCLK clock is ULPCLK 20MHz
 *    - for 80MHz bus clock, SYSCLK clock is ULPCLK 40MHz<br>

 * @version   ECE319K v1.2
 * @author    Daniel Valvano and Jonathan Valvano
 * @copyright Copyright 2023 by Jonathan W. Valvano, valvano@mail.utexas.edu,
 * @warning   AS-IS
 * @note      For more information see  http://users.ece.utexas.edu/~valvano/
 * @date      December 23, 2024
 <table>
<caption id="ICpins2">Input capture pins </caption>
<tr><th>Pin <th>Timer<th>Description
<tr><td>PA12<td>TIMG0<td>interrupts on rising edge of PA12   
</table>
  ******************************************************************************/

#ifndef __INPUTCAPTURE_H__
#define __INPUTCAPTURE_H__



/**
 * \brief use INPUTCAPTUREUSELFCLK to select 32768 Hz LFCLK
 */
#define INPUTCAPTUREUSELFCLK 2
/**
 * \brief use INPUTCAPTUREUSEMFCLK to select MFCLK (not tested)
 */
#define INPUTCAPTUREUSEMFCLK 4
/**
 * \brief use INPUTCAPTUREUSEBUSCLK to select BUSCLK
 */
#define INPUTCAPTUREUSEBUSCLK 8

/**
 * Arm interrupts on rising edge of PA12 <br>
 * Let <b>P</b> be the period of the selected clock<br>
 * <b>resolution</b> = P*(timerClkPrescale+1)<br>
 * <b>precision</b> = 16 bits<br>
 * <b>range</b> = 65535*resolution (max)<br>
 * timerClkDivRatio = 1<br>
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))<br>
 * resolution = (timerClkDivRatio * (timerClkPrescale + 1)))/timerClkSrc<br>
 * User defines their own ISR like the following
 * @code
uint32_t Data[20];
uint32_t Index=0;
uint32_t Time,Last,Period;
uint32_t iidx,count=0;
void TIMG0_IRQHandler(void){
  count++;
  iidx = TIMG0->CPU_INT.IIDX;// this will acknowledge
  if((iidx) == 5){ // 5 means capture CCD0
    Time = TIMG0->COUNTERREGS.CC_01[0]; // time now
    Period  = (Last-Time)&0xFFFF; // elapsed time since last
    Last = Time;
    if(Index < 20){
      Data[Index] = Period;
      Index++;
    }
  }
}
 @endcode
 * @image html InputCaptureInt.png width=500px            
 * - timerClkSrc  
 *    - 2 for 32768 Hz LFCLK<br>
 *    - 4 for MFCLK (not tested)<br>
 *    - 8 for BUSCLK

 * - For example
 *    - source=LFCLK, prescale=255, resolution = 7.8125ms
 *    - source=BUSCLK, prescale=47, resolution = 1us

 * @param timerClkSrc could be LFCLK, MFCLK, or BUSCLK
 * @param timerClkPrescale is prescale divide clock by timerClkPrescale+1, 0 to 255
 * @param priority is 0(highest),1,2 or 3(lowest)
 * @return none 
 * @brief  Initialize input capture
 * @note Assumes LaunchPad_Init has been called
 */
void InputCapture_Init(uint32_t timerClkSrc, uint32_t timerClkPrescale, uint32_t priority);


/**
 * Arm interrupts on rising edge of PB8 PB12 for RSLK2<br>
 * Let <b>P</b> be the period of the selected clock<br>
 * <b>resolution</b> = P*(timerClkPrescale+1)<br>
 * <b>precision</b> = 16 bits<br>
 * <b>range</b> = 65535*resolution (max)<br>
 * timerClkDivRatio = 1<br>
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))<br>
 * resolution = (timerClkDivRatio * (timerClkPrescale + 1)))/timerClkSrc<br>/ Power Domain PD1
 * dual signal waves connected to
 *   PB8  ELA  TA0_C0
 *   PB12 ERA  TA0_C1
 *  initialize A0 for rising edge input capture interrupt
 * for 32MHz bus clock, Timer clock is 32MHz
 * for 40MHz bus clock, Timer clock is MCLK 40MHz
 *  for 80MHz bus clock, Timer clock is MCLK 80MHz
 * interrupts should be enabled in main after all initialization
 * timerClkSrc = 2 for 32768 Hz LFCLK
 *             = 4 for 4MHz MFCLK
 *             = 8 for 80/32/4 BUSCLK
   * timerClkSrc could be 80 MHz, 40MHz, 32MHz, 4MHz, or 32767Hz
   * timerClkDivRatio = 1
   * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
   * resolution = (timerClkDivRatio * (timerClkPrescale + 1)))/timerClkSrc
   * For example, source=LFCLK, prescale=255, resolution = 7.8125ms
   * For example, source=BUSCLK, prescale=79, resolution = 1us
 * @param timerClkSrc could be LFCLK, MFCLK, or BUSCLK
 * @param timerClkPrescale is prescale divide clock by timerClkPrescale+1, 0 to 255
 * @param priority is 0(highest),1,2 or 3(lowest)
 * @return none 
 * @brief  Initialize input capture
 * @note Assumes LaunchPad_Init has been called
 */
void InputCapture_InitDual(uint32_t timerClkSrc, uint32_t timerClkPrescale, uint32_t priority);

#endif // __INPUTCAPTURE_H__
