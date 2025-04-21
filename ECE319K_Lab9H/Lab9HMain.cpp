// Lab9HMain.cpp
// Runs on MSPM0G3507
// Lab 9 ECE319H
// Your name
// Last Modified: 12/26/2024

#include <stdio.h>
#include <stdint.h>
#include <ti/devices/msp/msp.h>
#include "../inc/ST7735.h"
#include "../inc/Clock.h"
#include "../inc/LaunchPad.h"
#include "../inc/TExaS.h"
#include "../inc/Timer.h"
#include "../inc/SlidePot.h"
#include "../inc/DAC5.h"
#include "SmallFont.h"
#include "LED.h"
#include "Switch.h"
#include "Sound.h"
#include "images/images.h"
#include <math.h>
extern "C" void __disable_irq(void);
extern "C" void __enable_irq(void);
extern "C" void TIMG12_IRQHandler(void);
// ****note to ECE319K students****
// the data sheet says the ADC does not work when clock is 80 MHz
// however, the ADC seems to work on my boards at 80 MHz
// I suggest you try 80MHz, but if it doesn't work, switch to 40MHz
void PLL_Init(void){ // set phase lock loop (PLL)
  // Clock_Init40MHz(); // run this line for 40MHz
  Clock_Init80MHz(0);   // run this line for 80MHz
}

uint32_t M=1;
uint32_t Random32(void){
  M = 1664525*M+1013904223;
  return M;
}
uint32_t Random(uint32_t n){
  return (Random32()>>16)%n;
}

SlidePot Sensor(1500,0); // copy calibration from Lab 7

typedef enum {English, Spanish, Portuguese, French} Language_t;

// Game states
typedef enum {
    LANGUAGE_SELECT,
    START_SCREEN,
    GET_READY,
    PLAYING,
    GAME_OVER
} GameState_t;

// Game state structure
typedef struct {
    GameState_t currentState;
    uint32_t score;
    uint32_t lives;
    uint32_t level;
    int32_t backgroundX;  // for scrolling background
    uint32_t backgroundFrame;  // for cycling between backgrounds
    Language_t language;
} GameState;

// Global game state
GameState game;
// Global semaphore for TimerG12 interrupt
volatile bool gameSemaphore = false;










// games  engine runs at 30Hz
void TIMG12_IRQHandler(void){uint32_t pos,msg;
  if((TIMG12->CPU_INT.IIDX) == 1){ // this will acknowledge
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
// game engine goes here
    // 1) sample slide pot
    // 2) read input switches
    // 3) move sprites
    // 4) start sounds
    // 5) set semaphore
    gameSemaphore = true;  // Set semaphore to trigger game update
  }
}
uint8_t TExaS_LaunchPadLogicPB27PB26(void){
  return (0x80|((GPIOB->DOUT31_0>>26)&0x03));
}

Language_t myLanguage=English;
typedef enum {HELLO, GOODBYE, LANGUAGE} phrase_t;
const char Hello_English[] ="Hello";
const char Hello_Spanish[] ="\xADHola!";
const char Hello_Portuguese[] = "Ol\xA0";
const char Hello_French[] ="All\x83";
const char Goodbye_English[]="Goodbye";
const char Goodbye_Spanish[]="Adi\xA2s";
const char Goodbye_Portuguese[] = "Tchau";
const char Goodbye_French[] = "Au revoir";
const char Language_English[]="English";
const char Language_Spanish[]="Espa\xA4ol";
const char Language_Portuguese[]="Portugu\x88s";
const char Language_French[]="Fran\x87" "ais";
const char *Phrases[3][4]={
  {Hello_English,Hello_Spanish,Hello_Portuguese,Hello_French},
  {Goodbye_English,Goodbye_Spanish,Goodbye_Portuguese,Goodbye_French},
  {Language_English,Language_Spanish,Language_Portuguese,Language_French}
};

int main3(void){ // main3
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();

  // Configure all directional buttons as input with pull-down (positive logic)
  IOMUX->SECCFG.PINCM[PA28INDEX] = 0x00050081; // Left button
  IOMUX->SECCFG.PINCM[PB13INDEX] = 0x00050081; // Up button
  IOMUX->SECCFG.PINCM[PA16INDEX] = 0x00050081; // Down button
  IOMUX->SECCFG.PINCM[PA17INDEX] = 0x00050081; // Right button
  

//   ST7735_InitPrintf(INITR_BLACKTAB); // initialize screen
  ST7735_FillScreen(ST7735_BLACK); // clear screen

  // Display instructions
  ST7735_SetCursor(1, 1);
  ST7735_OutString((char* )"Switch Test");
  ST7735_SetCursor(1, 2);
  ST7735_OutString((char*)"Press buttons");

  while(1){
    // Read all button states
    uint32_t leftState = (GPIOA->DIN31_0 & (1 << 28)) >> 28;
    uint32_t upState = (GPIOB->DIN31_0 & (1 << 13)) >> 13;
    uint32_t downState = (GPIOA->DIN31_0 & (1 << 16)) >> 16;
    uint32_t rightState = (GPIOA->DIN31_0 & (1 << 17)) >> 17;

    // Display left button state
    ST7735_SetCursor(1, 4);
    ST7735_OutString((char* )"Left (PA28): ");
    ST7735_OutString((char*)(leftState ? "Pressed" : "Released"));

    // Display up button state
    ST7735_SetCursor(1, 5);
    ST7735_OutString((char* )"Up (PB13):   ");
    ST7735_OutString((char*)(upState ? "Pressed" : "Released"));

    // Display down button state
    ST7735_SetCursor(1, 6);
    ST7735_OutString((char* )"Down (PA16): ");
    ST7735_OutString((char*)(downState ? "Pressed" : "Released"));

    // Display right button state
    ST7735_SetCursor(1, 7);
    ST7735_OutString((char* )"Right (PA17):");
    ST7735_OutString((char*)(rightState ? "Pressed" : "Released"));

    Clock_Delay1ms(100); // debounce delay
  }
}

// use main4 to test sound outputs
int main4(void){ uint32_t last=0,now;
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  Switch_Init(); // initialize switches
  LED_Init(); // initialize LED
  Sound_Init();  // initialize sound
  TExaS_Init(ADC0,6,0); // ADC1 channel 6 is PB20, TExaS scope
  __enable_irq();
  while(1){
    now = Switch_In(); // one of your buttons
    if((last == 0)&&(now == 1)){
      Sound_Shoot(); // call one of your sounds
    }
    if((last == 0)&&(now == 2)){
      Sound_Coin(); // play coin sound
    }
    if((last == 0)&&(now == 4)){
       // call one of your sounds
    }
    if((last == 0)&&(now == 8)){
       // call one of your sounds
    }
    last = now; // update last state
    // modify this to test all your sounds
  }
}

// ALL ST7735 OUTPUT MUST OCCUR IN MAIN
int main(void){ // final main
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();

  IOMUX->SECCFG.PINCM[PA28INDEX] = 0x00050081; // Left button
  IOMUX->SECCFG.PINCM[PB13INDEX] = 0x00050081; // Up button
  IOMUX->SECCFG.PINCM[PA16INDEX] = 0x00050081; // Down button
  IOMUX->SECCFG.PINCM[PA17INDEX] = 0x00050081; // Right button

  ST7735_InitPrintf(INITR_BLACKTAB);
  ST7735_InvertDisplay(1);
    //note: if you colors are weird, see different options for
    // ST7735_InitR(INITR_REDTAB); inside ST7735_InitPrintf()
  ST7735_FillScreen(ST7735_BLACK);
  Sensor.Init(); // PB18 = ADC1 channel 5, slidepot
  Switch_Init(); // initialize switches
  LED_Init();    // initialize LED
  Sound_Init();  // initialize sound
  TExaS_Init(0,0,&TExaS_LaunchPadLogicPB27PB26); // PB27 and PB26
  
  // Initialize TimerG12 for 30Hz interrupts
  TimerG12_Init();  // Initialize timer hardware
  TimerG12_IntArm(2666666, 0);  // 80MHz/30Hz = 2,666,666, highest priority
  
  // initialize game state
  game.currentState = LANGUAGE_SELECT;
  game.score = 0;
  game.lives = 3;
  game.level = 1;
  game.backgroundX = 0;
  game.backgroundFrame = 0;
  game.language = English;
  gameSemaphore = false;
  
  __enable_irq();

  while(1){
    // wait for semaphore
        static int prevBarryY = 120;
    if(gameSemaphore) {
        gameSemaphore = false;  // clear semaphore
        
        // Handle state-specific logic
switch(game.currentState) {
  // LANGUAGE_SELECT state
// LANGUAGE_SELECT state
case LANGUAGE_SELECT:
    // Only clear screen when entering this state for the first time
    static bool firstTimeLanguage = true;
    if(firstTimeLanguage) {
        ST7735_FillScreen(ST7735_BLACK);
        firstTimeLanguage = false;
    }
    
    // Draw language selection screen
    ST7735_SetTextColor(ST7735_WHITE);
    if(game.language == English) {
        ST7735_DrawString(2, 4, (char*)"SELECT LANGUAGE:", ST7735_WHITE);
        ST7735_DrawString(2, 6, (char*)"> ENGLISH", ST7735_Color565(0,255,0));
        ST7735_DrawString(2, 8, (char*)"  SPANISH", ST7735_WHITE);
    } else {
        ST7735_DrawString(2, 4, (char*)"SELECT LANGUAGE:", ST7735_WHITE);
        ST7735_DrawString(2, 6, (char*)"  ENGLISH", ST7735_WHITE);
        ST7735_DrawString(2, 8, (char*)"> SPANISH", ST7735_SwapColor(ST7735_GREEN));
    }
    ST7735_DrawString(2, 12, (char*)"UP/DOWN TO SELECT", ST7735_WHITE);
    ST7735_DrawString(2, 14, (char*)"RIGHT BTN TO ENTER", ST7735_WHITE);
    
    {  // Use a block to limit variable scope
        // Read directional button states
        uint32_t upState = (GPIOB->DIN31_0 & (1 << 13)) >> 13;
        uint32_t downState = (GPIOA->DIN31_0 & (1 << 16)) >> 16;
        uint32_t rightState = (GPIOA->DIN31_0 & (1 << 17)) >> 17;
        
        // Use directional buttons for language selection
        if(upState) { 
            game.language = English;
            Clock_Delay1ms(100); // debounce
        } else if(downState) { 
            game.language = Spanish;
            Clock_Delay1ms(100); // debounce
        }
        
        // Use right button as "Fire" to select
        if(rightState) {
            game.currentState = START_SCREEN;
            firstTimeLanguage = true; // Reset for next time
            ST7735_FillScreen(ST7735_BLACK); // Clear screen before next state
            Clock_Delay1ms(100); // debounce
        }
    }
    break;

case START_SCREEN:
    // Only clear screen when entering this state for the first time
    static bool firstTimeStart = true;
    if(firstTimeStart) {
        ST7735_FillScreen(ST7735_BLACK);
        firstTimeStart = false;
    }
    
    {  // Use a block to limit variable scope
        // Read right button state
        uint32_t rightState = (GPIOA->DIN31_0 & (1 << 17)) >> 17;
        
        // Display start screen based on selected language
        if(game.language == English) {
            ST7735_DrawString(2, 4, (char*)"JETPACK JOYRIDE", ST7735_WHITE);
            ST7735_DrawString(2, 7, (char*)"RIGHT BTN TO START", ST7735_WHITE);
        } else { // Spanish
            ST7735_DrawString(2, 4, (char*)"PASEO EN JETPACK", ST7735_WHITE);
            ST7735_DrawString(2, 7, (char*)"DERECHA PARA JUGAR", ST7735_WHITE);
        }
        
        // Use right button to start the game
        if(rightState) {
            game.currentState = GET_READY;
            firstTimeStart = true; // Reset for next time
            ST7735_FillScreen(ST7735_BLACK); // Clear screen before next state
            Clock_Delay1ms(100); // debounce
        }
    }
    break;
case GET_READY:
    {
        static bool firstTimeGetReady = true;
        static uint32_t countdownTimer = 0;  // Timer for countdown
        static uint32_t countdownValue = 3;  // Start at 3
        
        // Initialize when first entering this state
        if(firstTimeGetReady) {
            ST7735_FillScreen(ST7735_BLACK);
            firstTimeGetReady = false;
            countdownTimer = 0;
            countdownValue = 3;
            
            // Setup the LEDs for countdown - initialize PB16 (red), PB12 (yellow), and PB17 (green)
            IOMUX->SECCFG.PINCM[PB16INDEX] = 0x00000081;  // Red LED (PB16) as GPIO output
            IOMUX->SECCFG.PINCM[PB12INDEX] = 0x00000081;  // Yellow LED (PB12) as GPIO output
            IOMUX->SECCFG.PINCM[PB17INDEX] = 0x00000081;  // Green LED (PB17) as GPIO output
            
            // Set as outputs
            GPIOB->DOE31_0 |= (1<<16)|(1<<12)|(1<<17);
            
            // Turn off all LEDs to start
            GPIOB->DOUTCLR31_0 = (1<<16)|(1<<12)|(1<<17);
            
            // Draw game area background (similar to PLAYING state)
            // Fill only the game area (40-120) with the dark rectangles
            const uint16_t darkGrayColors[5] = {
                ST7735_Color565(41, 41, 41),   // #292929
                ST7735_Color565(43, 43, 43),   // #2b2b2b
                ST7735_Color565(44, 44, 44),   // #2c2c2c
                ST7735_Color565(45, 45, 45),   // #2d2d2d
                ST7735_Color565(46, 46, 46)    // #2e2e2e
            };
            
            // Fill top area (40-55 height) - this is the ceiling
            for(int y = 40; y < 55; y += 8) {
                for(int x = 0; x < 128; x += 8) {
                    uint16_t randomColor = darkGrayColors[Random(5)];
                    ST7735_FillRect(x, y, 8, 8, randomColor);
                }
            }
            
            // Fill bottom area (121-135 height) - this is the floor
            for(int y = 121; y < 135; y += 8) {
                for(int x = 0; x < 128; x += 8) {
                    uint16_t randomColor = darkGrayColors[Random(5)];
                    ST7735_FillRect(x, y, 8, 8, randomColor);
                }
            }
        }
        
        // Draw the background similar to the playing state
        ST7735_DrawBitmap(game.backgroundX, 121, bg_spaceship_2, 128, 82);
        if(game.backgroundX < 0) {
            ST7735_DrawBitmap(game.backgroundX + 128, 121, bg_spaceship_2, 128, 82);
        }
        
        // Display "GET READY!" text at the top of the screen (outside game area)
        if(game.language == English) {
            // Clear the text area first
            ST7735_FillRect(0, 0, 128, 38, ST7735_BLACK);
            
            // Draw title text
            ST7735_DrawString(4, 2, (char*)"GET READY!", ST7735_WHITE);
        } else { // Spanish
            // Clear the text area first
            ST7735_FillRect(0, 0, 128, 38, ST7735_BLACK);
            
            // Draw title text
            ST7735_DrawString(4, 2, (char*)"PREPARATE!", ST7735_WHITE);
        }
        
        // Display countdown number in the center of the screen (extra large)
        char countdownStr[2];
        sprintf(countdownStr, "%d", countdownValue);
        
        // Set countdown color based on the current value
        uint16_t countdownColor;
        if(countdownValue == 3) {
            countdownColor = ST7735_RED;
            // Turn on RED LED, turn off others
            GPIOB->DOUTSET31_0 = (1<<16);
            GPIOB->DOUTCLR31_0 = (1<<12)|(1<<17);
        } else if(countdownValue == 2) {
            countdownColor = ST7735_YELLOW;
            // Turn on YELLOW LED, turn off others
            GPIOB->DOUTSET31_0 = (1<<12);
            GPIOB->DOUTCLR31_0 = (1<<16)|(1<<17);
        } else {
            countdownColor = ST7735_GREEN;
            // Turn on GREEN LED, turn off others
            GPIOB->DOUTSET31_0 = (1<<17);
            GPIOB->DOUTCLR31_0 = (1<<16)|(1<<12);
        }
        
        // Draw big countdown number at the bottom (outside game area)
        // First clear the area
        ST7735_FillRect(0, 136, 128, 24, ST7735_BLACK);
        
        // Draw the countdown number extra large in center bottom
        ST7735_DrawString(6, 17, countdownStr, countdownColor);
        
        // Display instructions for controls outside gameplay area
        if(game.language == English) {
            ST7735_DrawString(1, 13, (char*)"UP BUTTON = FLY", ST7735_WHITE);
        } else { // Spanish
            ST7735_DrawString(0, 13, (char*)"ARRIBA = VOLAR", ST7735_WHITE);
        }
        
        // Increment timer and update countdown
        countdownTimer++;
        
        // Update the countdown every 30 frames (about 1 second at 30Hz)
        if(countdownTimer >= 30) {
            countdownTimer = 0;
            countdownValue--;
            
            // Play sound for countdown
            Sound_Coin();
            
            // If countdown reached zero, change state to PLAYING
            if(countdownValue == 0) {
                game.currentState = PLAYING;
                firstTimeGetReady = true;
                
                // Reset game state for starting gameplay
                game.score = 0;
                
                // Turn off all LEDs
                GPIOB->DOUTCLR31_0 = (1<<16)|(1<<12)|(1<<17);
                
                // Clear screen before next state
                ST7735_FillScreen(ST7735_BLACK);
                
                // Play start sound
                Sound_Shoot();
            }
        }
    }
    break;

case PLAYING:
    // Update background X position (move left)
    game.backgroundX -= 2;  // Adjust speed as needed
    
    // If background has scrolled completely off screen
    if(game.backgroundX < -128) {
        game.backgroundX = 0;      // Reset position
    }
    
    // Only initialize the rectangles at the start
    static bool rectanglesInitialized = false;
    if(!rectanglesInitialized) {
        // Create an array of the 5 dark gray color variations
        const uint16_t darkGrayColors[5] = {
            ST7735_Color565(41, 41, 41),   // #292929
            ST7735_Color565(43, 43, 43),   // #2b2b2b
            ST7735_Color565(44, 44, 44),   // #2c2c2c
            ST7735_Color565(45, 45, 45),   // #2d2d2d
            ST7735_Color565(46, 46, 46)    // #2e2e2e
        };
        
        // Fill top area with rectangles (0-39 height)
        for(int y = 0; y < 39; y += 8) {
            for(int x = 0; x < 128; x += 8) {
                uint16_t randomColor = darkGrayColors[Random(5)];
                ST7735_FillRect(x, y, 8, 8, randomColor);
            }
        }
        
        // Fill bottom area with rectangles (121-160 height)
        for(int y = 121; y < 160; y += 8) {
            for(int x = 0; x < 128; x += 8) {
                uint16_t randomColor = darkGrayColors[Random(5)];
                ST7735_FillRect(x, y, 8, 8, randomColor);
            }
        }
        
        rectanglesInitialized = true;
    }
    
    // Define Barry's play region buffer (for layered drawing)
    static uint16_t barryRegion[22 * 66]; // 22 pixels wide, 66 pixels high (55-120)
    static int lastBarryY = -999; // Force initial draw
    
    // STEP 1: Redraw the scrolling background first
    // Draw background (original implementation)
    ST7735_DrawBitmap(game.backgroundX, 121, bg_spaceship_2, 128, 82);
    // Draw wrapped part (if needed)
    if(game.backgroundX < 0) {
        ST7735_DrawBitmap(game.backgroundX + 128, 121, bg_spaceship_2, 128, 82);
    }
    
{  // Scope block for Barry's movement and drawing
    // Define Barry's position and velocity
    static int barryY = 95;              // Barry's Y position - start in middle
    static float barryVelocity = 0;      // Barry's vertical velocity
    
    // Physics constants
    static const float GRAVITY = 0.3;    // Gravity acceleration
    static const float THRUST = -0.6;    // Upward acceleration when button pressed
    static const float TERMINAL_VEL = 4; // Maximum falling speed
    static const float MAX_THRUST = -3;  // Maximum upward speed
    
    // Read up/down button states
    uint32_t upState = (GPIOB->DIN31_0 & (1 << 13)) >> 13;    // Up button
    
    // Apply acceleration based on button state
    if(upState) {
        barryVelocity += THRUST;  // Apply upward thrust (negative is up)
        
        // Limit upward velocity
        if(barryVelocity < MAX_THRUST) {
            barryVelocity = MAX_THRUST;
        }
    } else {
        // Apply gravity
        barryVelocity += GRAVITY;
        
        // Limit falling velocity
        if(barryVelocity > TERMINAL_VEL) {
            barryVelocity = TERMINAL_VEL;
        }
    }
    
    // Update Barry's position using his velocity
    barryY += (int)barryVelocity;
    
    // Keep Barry within screen bounds
    if(barryY < 69) {
        barryY = 69;  // Top boundary (below ceiling)
        barryVelocity = 0; // Stop velocity when hitting ceiling
    }
    if(barryY > 120) {
        barryY = 120; // Bottom boundary (above floor)
        barryVelocity = 0; // Stop velocity when hitting floor
    }
    
    // STEP 2: Always draw Barry on every frame - no conditionals 
    // First, clear Barry's area with a solid black rectangle (precise dimensions)
    ST7735_FillRect(20, 55, 22, 66, ST7735_BLACK);
    
    // Then, draw Barry sprite at the current position
    ST7735_DrawBitmap(20, barryY, barry0, 22, 29);
    
    // STEP 3: Draw flame effect directly after Barry is drawn (if needed)
    if(upState) {
        // Draw a proper flame when the jetpack is active
        ST7735_FillRect(24, barryY + 22, 3, 4, ST7735_YELLOW);
    }
    
    // Update last position for next frame
    lastBarryY = barryY;
    
    // Display score and lives
    char scoreStr[20];
    if(game.language == English) {
        sprintf(scoreStr, "%dm", game.score);
        ST7735_DrawString(0, 0, scoreStr, ST7735_WHITE);
    } else { // Spanish
        sprintf(scoreStr, "Puntos: %d", game.score);
        ST7735_DrawString(0, 0, scoreStr, ST7735_WHITE);
    }
    
    // Increment score every frame (you can adjust this as needed)
    game.score++;
}
break;
                
            case GAME_OVER:
                // TODO: Implement game over state later
                break;
        }
    }
  }
}
// Test edit
