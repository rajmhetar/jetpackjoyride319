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

// Add collision detection function
bool CheckCollision(int barryX, int barryY, int barryW, int barryH, int laserX, int laserY, int laserW, int laserH) {
    return (barryX < laserX + laserW &&
            barryX + barryW > laserX &&
            barryY < laserY + laserH &&
            barryY + barryH > laserY);
}

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

# define MAX_LASERS 5  // Maximum number of lasers active at once

// Define maximum number of coins active at once
#define MAX_COINS 8  // Increased to allow for formations

// Define coin formation types
typedef enum {
    HORIZONTAL_LINE,
    VERTICAL_LINE,
    SINGLE_COIN
} CoinFormation_t;

// Laser types
typedef enum {
    HORIZONTAL,
    VERTICAL,
    DIAGONAL
} LaserType_t;

// Structure for coins - only position and active status
typedef struct {
    int32_t x;        // X position
    int32_t y;        // Y position
    bool active;      // Whether the coin is currently active
    CoinFormation_t formation;  // Type of formation this coin belongs to
    int32_t formationId;       // ID to group coins in same formation
} Coin_t;

// Laser structure
typedef struct {
    int32_t x;        // X position
    int32_t y;        // Y position
    bool active;      // Whether the laser is currently active
    LaserType_t type; // Type of laser
    int32_t size;     // Size of laser (width for horizontal, height for vertical, length for diagonal)
} Laser_t;

// Game state structure
typedef struct {
    GameState_t currentState;
    uint32_t score;
    uint32_t level;
    int32_t backgroundX;  // for scrolling background
    uint32_t backgroundFrame;  // for cycling between backgrounds
    Language_t language;
    Laser_t lasers[MAX_LASERS];  // Array of lasers
    uint32_t laserSpawnTimer;    // Timer for spawning new lasers
    Coin_t coins[MAX_COINS];     // Array of coins - only for drawing
    uint32_t coinSpawnTimer;     // Timer for spawning new coins
    uint32_t coinsCollected;     // Counter for collected coins
} GameState;

// Global game state
GameState game;
// Global semaphore for TimerG12 interrupt
volatile bool gameSemaphore = false;

// Add these at the top of the file with other global variables
static uint16_t framebuffer[128][160];  // Double buffer for smooth animation

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
  game.level = 1;
  game.backgroundX = 0;
  game.backgroundFrame = 0;
  game.language = English;
  game.laserSpawnTimer = 0;
  game.coinSpawnTimer = 0;
  game.coinsCollected = 0;
  // Initialize all lasers as inactive
  for(int i = 0; i < MAX_LASERS; i++) {
      game.lasers[i].active = false;
  }
  // Initialize all coins as inactive
  for(int i = 0; i < MAX_COINS; i++) {
      game.coins[i].active = false;
  }
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
    
    // Laser spawning and movement logic
    {
        // Increment spawn timer
        game.laserSpawnTimer++;
        
        // Try to spawn a new laser every 30-90 frames (1-3 seconds at 30Hz)
        if(game.laserSpawnTimer >= (30 + Random(60))) {
            game.laserSpawnTimer = 0;
            
            // Try to find an inactive laser slot
            for(int i = 0; i < MAX_LASERS; i++) {
                if(!game.lasers[i].active) {
                    // Spawn a new laser
                    game.lasers[i].active = true;
                    game.lasers[i].x = 128;  // Start at right edge
                    
                    // Define playable area bounds (between ceiling and floor)
                    const int ceilingY = 55;  // Top boundary (ceiling)
                    const int floorY = 120;   // Bottom boundary (floor)
                    
                    // Determine laser type first
                    game.lasers[i].type = static_cast<LaserType_t>(Random(3)); // Random laser type
                    
                    // Random size between 10 and 24
                    game.lasers[i].size = Random(15) + 10;
                    
                    // Adjust Y position based on laser type to keep within bounds
                    switch(game.lasers[i].type) {
                        case HORIZONTAL:
                            // For horizontal lasers, account for height (4 pixels)
                            game.lasers[i].y = ceilingY + 2 + Random(floorY - ceilingY - 4);
                            break;
                        case VERTICAL:
                            // For vertical lasers, ensure the full height stays within bounds
                            game.lasers[i].y = ceilingY + Random(floorY - ceilingY - game.lasers[i].size);
                            break;
                        case DIAGONAL:
                            // For diagonal lasers, ensure they stay within bounds
                            game.lasers[i].y = ceilingY + Random(floorY - ceilingY - game.lasers[i].size);
                            break;
                    }
                    
                    break;  // Only spawn one laser at a time
                }
            }
        }
        
        // Move and draw active lasers
        for(int i = 0; i < MAX_LASERS; i++) {
            if(game.lasers[i].active) {
                // Move laser left with background
                game.lasers[i].x -= 2;  // Same speed as background
                
                // If laser moves off screen, deactivate it
                if(game.lasers[i].x < -game.lasers[i].size) {
                    game.lasers[i].active = false;
                } else {
                    // Draw laser based on its type
                    switch(game.lasers[i].type) {
                        case HORIZONTAL:
                            // Draw a wider horizontal laser with a glowing effect
                            ST7735_FillRect(game.lasers[i].x, game.lasers[i].y - 1, game.lasers[i].size, 4, ST7735_RED);
                            ST7735_FillRect(game.lasers[i].x, game.lasers[i].y, game.lasers[i].size, 2, ST7735_Color565(255, 100, 100)); // Brighter center
                            break;
                        case VERTICAL:
                            // Draw a wider vertical laser with a glowing effect
                            ST7735_FillRect(game.lasers[i].x - 1, game.lasers[i].y, 4, game.lasers[i].size, ST7735_RED);
                            ST7735_FillRect(game.lasers[i].x, game.lasers[i].y, 2, game.lasers[i].size, ST7735_Color565(255, 100, 100)); // Brighter center
                            break;
                        case DIAGONAL:
                            // Draw a thicker diagonal laser with enhanced glowing effect
                            for(int j = 0; j < game.lasers[i].size; j++) {
                                // Draw thicker diagonal laser with multiple pixels
                                // Main diagonal line (3 pixels thick)
                                ST7735_DrawPixel(game.lasers[i].x + j, game.lasers[i].y + j, ST7735_Color565(255, 100, 100)); // Center bright pixel
                                ST7735_DrawPixel(game.lasers[i].x + j - 1, game.lasers[i].y + j, ST7735_RED); // Left
                                ST7735_DrawPixel(game.lasers[i].x + j + 1, game.lasers[i].y + j, ST7735_RED); // Right
                                ST7735_DrawPixel(game.lasers[i].x + j, game.lasers[i].y + j - 1, ST7735_RED); // Above
                                ST7735_DrawPixel(game.lasers[i].x + j, game.lasers[i].y + j + 1, ST7735_RED); // Below
                            }
                            break;
                    }
                }
            }
        }
    }
    
    // Coin generation and drawing logic ONLY - no collection functionality
    {
        // Increment coin spawn timer
        game.coinSpawnTimer++;
        
        // Try to spawn a new coin formation every 60-100 frames
        if(game.coinSpawnTimer >= (60 + Random(40))) {
            game.coinSpawnTimer = 0;
            
            // Define playable area bounds (between ceiling and floor)
            const int ceilingY = 55;  // Top boundary (ceiling)
            const int floorY = 120;   // Bottom boundary (floor)
            const int coinWidth = 6;  // Coin width
            const int coinHeight = 6; // Coin height
            const int spacing = 4;    // Spacing between coins in a formation
            
            // Count active coins
            int activeCoins = 0;
            for(int i = 0; i < MAX_COINS; i++) {
                if(game.coins[i].active) {
                    activeCoins++;
                }
            }
            
            // Only spawn new coins if we have space
            if(activeCoins < MAX_COINS - 3) { // Ensure we have space for at least 3 coins
                // Randomly choose formation type (60% horizontal, 40% vertical)
                CoinFormation_t formation = (Random(100) < 60) ? HORIZONTAL_LINE : VERTICAL_LINE;
                
                // Generate unique formation ID for grouping coins
                int32_t formationId = Random32();
                
                // Choose formation length (3-5 coins)
                int formationLength = 3 + Random(3);
                if(formationLength > (MAX_COINS - activeCoins)) {
                    formationLength = MAX_COINS - activeCoins;
                }
                
                // Calculate formation starting position
                int startX = 128; // Start from right edge
                int startY;
                
                if(formation == HORIZONTAL_LINE) {
                    // For horizontal formation, pick a random Y in valid range
                    startY = ceilingY + 10 + Random(floorY - ceilingY - 20 - coinHeight);
                } else { // VERTICAL_LINE
                    // For vertical formation, ensure entire line fits within screen height
                    int verticalSpace = (formationLength * (coinHeight + spacing)) - spacing;
                    startY = ceilingY + 10 + Random(floorY - ceilingY - 20 - verticalSpace);
                }
                
                // Check if the formation would overlap with any lasers
                bool overlapsLaser = false;
                for(int i = 0; i < MAX_LASERS; i++) {
                    if(game.lasers[i].active) {
                        int laserX = game.lasers[i].x;
                        int laserY = game.lasers[i].y;
                        int laserWidth = 0;
                        int laserHeight = 0;
                        
                        // Set laser dimensions based on its type
                        switch(game.lasers[i].type) {
                            case HORIZONTAL:
                                laserWidth = game.lasers[i].size;
                                laserHeight = 4;
                                break;
                            case VERTICAL:
                                laserWidth = 4;
                                laserHeight = game.lasers[i].size;
                                break;
                            case DIAGONAL:
                                laserWidth = game.lasers[i].size * 0.7;
                                laserHeight = game.lasers[i].size * 0.7;
                                break;
                        }
                        
                        // Check if formation would overlap with this laser
                        if(formation == HORIZONTAL_LINE) {
                            int formationWidth = (formationLength * (coinWidth + spacing)) - spacing;
                            if(startX < laserX + laserWidth + 20 && // Add buffer zone
                               startX + formationWidth > laserX - 20 &&
                               startY < laserY + laserHeight + 5 &&
                               startY + coinHeight > laserY - 5) {
                                overlapsLaser = true;
                                break;
                            }
                        } else { // VERTICAL_LINE
                            int formationHeight = (formationLength * (coinHeight + spacing)) - spacing;
                            if(startX < laserX + laserWidth + 20 && // Add buffer zone
                               startX + coinWidth > laserX - 20 &&
                               startY < laserY + laserHeight + 5 &&
                               startY + formationHeight > laserY - 5) {
                                overlapsLaser = true;
                                break;
                            }
                        }
                    }
                }
                
                // If no overlap with lasers, create the formation
                if(!overlapsLaser) {
                    // Find free slots and create formation
                    int coinsPlaced = 0;
                    for(int i = 0; i < MAX_COINS && coinsPlaced < formationLength; i++) {
                        if(!game.coins[i].active) {
                            game.coins[i].active = true;
                            game.coins[i].formation = formation;
                            game.coins[i].formationId = formationId;
                            
                            if(formation == HORIZONTAL_LINE) {
                                // Place coins in horizontal line
                                game.coins[i].x = startX + (coinsPlaced * (coinWidth + spacing));
                                game.coins[i].y = startY;
                            } else { // VERTICAL_LINE
                                // Place coins in vertical line
                                game.coins[i].x = startX;
                                game.coins[i].y = startY + (coinsPlaced * (coinHeight + spacing));
                            }
                            
                            coinsPlaced++;
                        }
                    }
                }
            }
        }
        
        // Move and draw active coins
        for(int i = 0; i < MAX_COINS; i++) {
            if(game.coins[i].active) {
                // Move coin left with background
                game.coins[i].x -= 2;  // Same speed as background
                
                // If coin moves off screen, deactivate it
                if(game.coins[i].x < -6) {  // 6 is the width of coin0
                    game.coins[i].active = false;
                } else {
                    // Draw the coin using the coin0 bitmap (6x6 pixels)
                    ST7735_DrawBitmap(game.coins[i].x, game.coins[i].y, coin0, 6, 6);
                }
            }
        }
    }
    
{  // Scope block for Barry's movement and drawing
    // Define Barry's position and velocity
    static int barryY = 95;              // Barry's Y position - start in middle
    static float barryVelocity = 0;      // Barry's vertical velocity
    static int lastBarryY = -999;        // Track Barry's last position
    
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
    if(barryY < 70) {  // Changed from 69 to 55 to account for 37-pixel sprite
        barryY = 70;  // Top boundary (below ceiling)
        barryVelocity = 0; // Stop velocity when hitting ceiling
    }
    if(barryY > 120) {
        barryY = 120; // Bottom boundary (above floor)
        barryVelocity = 0; // Stop velocity when hitting floor
    }
    
    // Disable interrupts during drawing to prevent tearing
    __disable_irq();
    
    // Only update if Barry moved or background scrolled
    if(barryY != lastBarryY || game.backgroundX % 2 == 0) {
        // Draw the scrolling background
        ST7735_DrawBitmap(game.backgroundX, 121, bg_spaceship_2, 128, 82);
        if(game.backgroundX < 0) {
            ST7735_DrawBitmap(game.backgroundX + 128, 121, bg_spaceship_2, 128, 82);
        }
        
        // Draw Barry
        ST7735_DrawBitmap(15, barryY, barry3, 18, 30);
    }
    
    // Re-enable interrupts
    __enable_irq();
    
    // Update last position for next frame
    lastBarryY = barryY;
    
    // Display score and lives
    char scoreStr[20];
    char coinStr[20];
    if(game.language == English) {
        sprintf(scoreStr, "%dm", game.score);
        sprintf(coinStr, "Coins: %d", game.coinsCollected);
        ST7735_DrawString(0, 0, scoreStr, ST7735_WHITE);
        ST7735_DrawString(9, 0, coinStr, ST7735_WHITE);
    } else { // Spanish
        sprintf(scoreStr, "Puntos: %d", game.score);
        sprintf(coinStr, "Monedas: %d", game.coinsCollected);
        ST7735_DrawString(0, 0, scoreStr, ST7735_WHITE);
        ST7735_DrawString(0, 1, coinStr, ST7735_WHITE);
    }
    
    // Increment score every frame (you can adjust this as needed)
    game.score++;
    
    // Collision detection between Barry and lasers
    const int BARRY_X = 15;          // Barry's X position (fixed)
    const int BARRY_WIDTH = 18;      // Barry's width
    const int BARRY_HEIGHT = 30;     // Barry's height
    
    // Create a smaller hitbox for Barry (use a 60% of visual size)
    // This creates a more forgiving collision detection - accounting for sprite drawn from bottom left
    const int BARRY_HITBOX_X = BARRY_X + 3;          // Small inset from left
    const int BARRY_HITBOX_Y = barryY - BARRY_HEIGHT + 8;  // Bottom-left origin, so subtract height and add offset
    const int BARRY_HITBOX_WIDTH = BARRY_WIDTH - 6;  // Make hitbox slightly narrower
    const int BARRY_HITBOX_HEIGHT = BARRY_HEIGHT - 16; // Make hitbox much shorter to exclude jetpack
    
    // Debug collision - uncomment to visualize hitbox (for testing only)
    // ST7735_FillRect(BARRY_HITBOX_X, BARRY_HITBOX_Y, BARRY_HITBOX_WIDTH, BARRY_HITBOX_HEIGHT, ST7735_YELLOW);
    
    // Check collision with each active laser
    for(int i = 0; i < MAX_LASERS; i++) {
        if(game.lasers[i].active) {
            int laserX = game.lasers[i].x;
            int laserY = game.lasers[i].y;
            int laserWidth = 0;
            int laserHeight = 0;
            
            // Set laser hitbox dimensions based on its type
            switch(game.lasers[i].type) {
                case HORIZONTAL:
                    laserWidth = game.lasers[i].size;
                    laserHeight = 4;  // Horizontal lasers are 4 pixels high
                    break;
                case VERTICAL:
                    laserWidth = 4;   // Vertical lasers are 4 pixels wide
                    laserHeight = game.lasers[i].size;
                    break;
                case DIAGONAL:
                    // For diagonal lasers, use a smaller, more accurate hitbox
                    // This matches the visual representation better
                    laserWidth = game.lasers[i].size * 0.7;  // Reduce hitbox size to match visual
                    laserHeight = game.lasers[i].size * 0.7; // Reduce hitbox size to match visual
                    break;
            }
            
            // Check if Barry's hitbox collides with this laser
            if(CheckCollision(BARRY_HITBOX_X, BARRY_HITBOX_Y, BARRY_HITBOX_WIDTH, BARRY_HITBOX_HEIGHT, 
                             laserX, laserY, laserWidth, laserHeight)) {
                // Collision detected! Play sound and transition to game over
                Sound_Shoot();  // Play collision sound
                
                // Transition to game over state
                game.currentState = GAME_OVER;
                
                // No need to check other lasers
                break;
            }
        }
    }
    
    // Check collision with each active coin
    for(int i = 0; i < MAX_COINS; i++) {
        if(game.coins[i].active) {
            // Coin dimensions (6x6 pixels)
            const int COIN_WIDTH = 6;
            const int COIN_HEIGHT = 6;
            
            // Check if Barry's hitbox collides with this coin
            if(CheckCollision(BARRY_HITBOX_X, BARRY_HITBOX_Y, BARRY_HITBOX_WIDTH, BARRY_HITBOX_HEIGHT, 
                              game.coins[i].x, game.coins[i].y, COIN_WIDTH, COIN_HEIGHT)) {
                // Collision with coin! Add to coin counter and play sound
                game.coinsCollected++;  // Increment coin counter
                Sound_Coin();           // Play coin collection sound
                
                // Deactivate the coin
                game.coins[i].active = false;
            }
        }
    }
}
break;
                
case GAME_OVER:
    // Only clear screen when entering this state for the first time
    static bool firstTimeGameOver = true;
    if(firstTimeGameOver) {
        ST7735_FillScreen(ST7735_BLACK);
        firstTimeGameOver = false;
    }
    
    // Draw the game over screen with fancy red text
    if(game.language == English) {
        ST7735_DrawString(3, 3, (char*)"GAME OVER", ST7735_RED);
        
        // Display final score
        char scoreStr[20];
        sprintf(scoreStr, "SCORE: %d", game.score);
        ST7735_DrawString(3, 6, scoreStr, ST7735_WHITE);
        
        // Display coin count
        char coinStr[20];
        sprintf(coinStr, "COINS: %d", game.coinsCollected);
        ST7735_DrawString(3, 8, coinStr, ST7735_WHITE);
        
        // Display restart instruction
        ST7735_DrawString(1, 12, (char*)"PRESS RIGHT BUTTON", ST7735_WHITE);
        ST7735_DrawString(4, 13, (char*)"TO RESTART", ST7735_WHITE);
    } else { // Spanish
        ST7735_DrawString(3, 3, (char*)"FIN DEL JUEGO", ST7735_RED);
        
        // Display final score
        char scoreStr[20];
        sprintf(scoreStr, "PUNTOS: %d", game.score);
        ST7735_DrawString(3, 6, scoreStr, ST7735_WHITE);
        
        // Display coin count
        char coinStr[20];
        sprintf(coinStr, "MONEDAS: %d", game.coinsCollected);
        ST7735_DrawString(3, 8, coinStr, ST7735_WHITE);
        
        // Display restart instruction
        ST7735_DrawString(1, 12, (char*)"PRESIONA DERECHA", ST7735_WHITE);
        ST7735_DrawString(3, 13, (char*)"PARA REINICIAR", ST7735_WHITE);
    }
    
    // Check if right button is pressed to restart
    uint32_t rightState = (GPIOA->DIN31_0 & (1 << 17)) >> 17;
    if(rightState) {
        // Reset game state
        game.currentState = START_SCREEN;
        game.score = 0;
        game.coinsCollected = 0;  // Reset coin counter
        game.backgroundX = 0;
        
        // Reset all lasers
        for(int i = 0; i < MAX_LASERS; i++) {
            game.lasers[i].active = false;
        }
        
        // Reset all coins
        for(int i = 0; i < MAX_COINS; i++) {
            game.coins[i].active = false;
        }
        
        // Reset first time flags
        firstTimeGameOver = true;
        
        // Clear screen before transitioning
        ST7735_FillScreen(ST7735_BLACK);
        Clock_Delay1ms(100); // debounce
    }
    break;
}
}
}
// Test edit
}
