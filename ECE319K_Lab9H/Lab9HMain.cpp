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
#include <string.h>

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
    DIFFICULTY_SELECT,
    START_SCREEN,
    GET_READY,
    PLAYING,
    GAME_OVER
} GameState_t;

// Define difficulty levels
typedef enum {
    EASY,
    MEDIUM,
    HARD
} Difficulty_t;

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
    Difficulty_t difficulty;   // New difficulty setting
    float gameSpeed;           // Speed multiplier based on difficulty
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

// Add a function to read directional buttons
// Returns a 4-bit value where bit 0=right, 1=left, 2=down, 3=up
uint32_t ReadDirectionalButtons(void) {
  uint32_t buttons = 0;
  
  // Read all button states - using direct GPIO access
  uint32_t rightState = (GPIOA->DIN31_0 & (1 << 17)) >> 17;  // Right button (PA17)
  uint32_t leftState = (GPIOA->DIN31_0 & (1 << 28)) >> 28;   // Left button (PA28)
  uint32_t downState = (GPIOA->DIN31_0 & (1 << 16)) >> 16;   // Down button (PA16)
  uint32_t upState = (GPIOB->DIN31_0 & (1 << 13)) >> 13;     // Up button (PB13)
  
  // Combine into a single 4-bit value
  buttons = (upState << 3) | (downState << 2) | (leftState << 1) | rightState;
  
  return buttons;
}

// Helper function to check if a specific button is pressed
bool IsButtonPressed(uint32_t buttonMask) {
  return (ReadDirectionalButtons() & buttonMask) != 0;
}

// Button masks for easy access
#define BUTTON_RIGHT 0x01
#define BUTTON_LEFT  0x02
#define BUTTON_DOWN  0x04
#define BUTTON_UP    0x08

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

// use main function to continuously play coin sound
int main4(void){ 
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  
  // Configure all directional buttons as input with pull-down (positive logic)
  IOMUX->SECCFG.PINCM[PA28INDEX] = 0x00050081; // Left button
  IOMUX->SECCFG.PINCM[PB13INDEX] = 0x00050081; // Up button
  IOMUX->SECCFG.PINCM[PA16INDEX] = 0x00050081; // Down button
  IOMUX->SECCFG.PINCM[PA17INDEX] = 0x00050081; // Right button
  
  Switch_Init(); // initialize switches
  LED_Init(); // initialize LED
  Sound_Init();  // initialize sound
  TExaS_Init(ADC0,6,0); // ADC1 channel 6 is PB20, TExaS scope
  __enable_irq();
  
  // Main loop - continuously play coin sound
  while(1){
    // Play the coin sound
    Sound_Coin();
    //Sound_Explosion();
    // Toggle LED to show we're alive
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27
    
    // Small delay between sounds to avoid overlap
    Clock_Delay1ms(500);
    
  }
}

// ALL ST7735 OUTPUT MUST OCCUR IN MAIN
int main(void){ // THE BEST GAME EVER!!!!
  __disable_irq(); 
  PLL_Init(); 
  LaunchPad_Init();

  IOMUX->SECCFG.PINCM[PA28INDEX] = 0x00050081; 
  IOMUX->SECCFG.PINCM[PB13INDEX] = 0x00050081; 
  IOMUX->SECCFG.PINCM[PA16INDEX] = 0x00050081; 
  IOMUX->SECCFG.PINCM[PA17INDEX] = 0x00050081; 

  ST7735_InitPrintf(INITR_BLACKTAB);
  ST7735_InvertDisplay(1);
    //colors were looking weird so i fixed them
    // ST7735_InitR(INITR_REDTAB); inside ST7735_InitPrintf()
  ST7735_FillScreen(ST7735_BLACK); // black background looks cool
  Sensor.Init(); // PB18 = ADC1 channel 5, slidepot for difficulty
  Switch_Init(); // turn on the switches
  LED_Init();    // turn on the LEDs
  Sound_Init();  // make game sounds work!!
  TExaS_Init(0,0,&TExaS_LaunchPadLogicPB27PB26); // idk what this does but it's important
  
  TimerG12_Init();  // timer hardware
  TimerG12_IntArm(2666666, 0);  // 80MHz/30Hz = 2,666,666, highest priority cuz game needs to be smooth
  
  // setup the game - everything starts at 0
  game.currentState = LANGUAGE_SELECT;
  game.score = 0; 
  game.level = 1; 
  game.backgroundX = 0;
  game.backgroundFrame = 0;
  game.language = English; 
  game.difficulty = MEDIUM;      
  game.gameSpeed = 1.0f;         
  game.laserSpawnTimer = 0;
  game.coinSpawnTimer = 0;
  game.coinsCollected = 0; 
  
  for(int i = 0; i < MAX_LASERS; i++) {
      game.lasers[i].active = false;
  }
  
  for(int i = 0; i < MAX_COINS; i++) {
      game.coins[i].active = false;
  }
  gameSemaphore = false; 
  
  __enable_irq(); // turn interrupts back on or nothing will work!

  while(1){ // infinite loop - game runs forever
    static int prevBarryY = 120; 
    if(gameSemaphore) {
        gameSemaphore = false;  
        
        uint32_t buttons = ReadDirectionalButtons(); 
        switch(game.currentState) {
            // LANGUAGE_SELECT state - first screen
            case LANGUAGE_SELECT:
                static bool firstTimeLanguage = true;
                if(firstTimeLanguage) {
                    ST7735_FillScreen(ST7735_BLACK); 
                    firstTimeLanguage = false; 
                }
                
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
                
                if(buttons & BUTTON_UP) { 
                    game.language = English; 
                    Clock_Delay1ms(100); 
                } else if(buttons & BUTTON_DOWN) { 
                    game.language = Spanish; 
                    Clock_Delay1ms(100); 
                }
                
                if(buttons & BUTTON_RIGHT) {
                    game.currentState = DIFFICULTY_SELECT; 
                    firstTimeLanguage = true; 
                    ST7735_FillScreen(ST7735_BLACK); 
                    Clock_Delay1ms(100); 
                }
                break;

            case DIFFICULTY_SELECT: // slide left=easy, right=hard
                {
                    static bool firstTimeDifficulty = true;
                    if(firstTimeDifficulty) {
                        ST7735_FillScreen(ST7735_BLACK);
                        firstTimeDifficulty = false;
                    }
                    
                    uint32_t slidePotValue = Sensor.In();
                    
                    if(slidePotValue < 1365) {
                        game.difficulty = EASY;
                        game.gameSpeed = 0.7f;  
                    } else if(slidePotValue < 2730) {
                        game.difficulty = MEDIUM;
                        game.gameSpeed = 1.0f;  
                    } else {
                        game.difficulty = HARD;
                        game.gameSpeed = 1.5f;  // EXTREME SPEED MODE!!!
                    }
                    
                    if(game.language == English) {
                        ST7735_DrawString(2, 2, (char*)"SELECT DIFFICULTY:", ST7735_WHITE);
                        ST7735_DrawString(2, 5, (char*)"EASY", (game.difficulty == EASY) ? ST7735_GREEN : ST7735_WHITE);
                        ST7735_DrawString(2, 7, (char*)"MEDIUM", (game.difficulty == MEDIUM) ? ST7735_YELLOW : ST7735_WHITE);
                        ST7735_DrawString(2, 9, (char*)"HARD", (game.difficulty == HARD) ? ST7735_RED : ST7735_WHITE);
                        ST7735_DrawString(2, 12, (char*)"USE SLIDEPOT", ST7735_WHITE);
                        ST7735_DrawString(2, 14, (char*)"RIGHT BTN TO SELECT", ST7735_WHITE);
                    } else { // Spanish
                        ST7735_DrawString(2, 2, (char*)"SELECCIONAR DIFICULTAD:", ST7735_WHITE);
                        ST7735_DrawString(2, 5, (char*)"FACIL", (game.difficulty == EASY) ? ST7735_GREEN : ST7735_WHITE);
                        ST7735_DrawString(2, 7, (char*)"MEDIO", (game.difficulty == MEDIUM) ? ST7735_YELLOW : ST7735_WHITE);
                        ST7735_DrawString(2, 9, (char*)"DIFICIL", (game.difficulty == HARD) ? ST7735_RED : ST7735_WHITE);
                        ST7735_DrawString(2, 12, (char*)"USAR DESLIZADOR", ST7735_WHITE);
                        ST7735_DrawString(2, 14, (char*)"DERECHA PARA SELECCIONAR", ST7735_WHITE);
                    }
                    
                    int sliderPos = slidePotValue * 100 / 4095;  
                    ST7735_FillRect(20, 110, 100, 10, ST7735_BLACK);
                    ST7735_FillRect(20, 110, 100, 10, ST7735_BLUE);
                    ST7735_FillRect(20 + sliderPos, 108, 5, 14, ST7735_WHITE);
                    
                    if(buttons & BUTTON_RIGHT) {
                        game.currentState = START_SCREEN;
                        firstTimeDifficulty = true; 
                        ST7735_FillScreen(ST7735_BLACK); 
                        Clock_Delay1ms(100); 
                    }
                }
                break;

            case START_SCREEN:
                static bool firstTimeStart = true;
                if(firstTimeStart) {
                    ST7735_FillScreen(ST7735_BLACK);
                    firstTimeStart = false;
                }
                
                if(game.language == English) {
                    ST7735_DrawString(2, 4, (char*)"JETPACK JOYRIDE", ST7735_WHITE);
                    ST7735_DrawString(2, 7, (char*)"RIGHT BTN TO START", ST7735_WHITE);
                } else { // Spanish
                    ST7735_DrawString(2, 4, (char*)"PASEO EN JETPACK", ST7735_WHITE);
                    ST7735_DrawString(2, 7, (char*)"DERECHA PARA JUGAR", ST7735_WHITE);
                }
                
                if(buttons & BUTTON_RIGHT) {
                    game.currentState = GET_READY;
                    firstTimeStart = true; 
                    ST7735_FillScreen(ST7735_BLACK); 
                    Clock_Delay1ms(100); 
                }
                break;
            case GET_READY: // 3...2...1...GO!
                {
                    static bool firstTimeGetReady = true;
                    static uint32_t countdownTimer = 0;  
                    static uint32_t countdownValue = 3;  
                    
                    if(firstTimeGetReady) {
                        ST7735_FillScreen(ST7735_BLACK);
                        firstTimeGetReady = false;
                        countdownTimer = 0;
                        countdownValue = 3;
                        
                        IOMUX->SECCFG.PINCM[PB16INDEX] = 0x00000081;  // Red LED = 3
                        IOMUX->SECCFG.PINCM[PB12INDEX] = 0x00000081;  // Yellow LED = 2
                        IOMUX->SECCFG.PINCM[PB17INDEX] = 0x00000081;  // Green LED = 1
                        
                        GPIOB->DOE31_0 |= (1<<16)|(1<<12)|(1<<17);
                        
                        GPIOB->DOUTCLR31_0 = (1<<16)|(1<<12)|(1<<17);
                        
                        const uint16_t darkGrayColors[5] = {
                            ST7735_Color565(41, 41, 41),   
                            ST7735_Color565(43, 43, 43),   
                            ST7735_Color565(44, 44, 44),   
                            ST7735_Color565(45, 45, 45),   
                            ST7735_Color565(46, 46, 46)    
                        };
                        
                        for(int y = 40; y < 55; y += 8) {
                            for(int x = 0; x < 128; x += 8) {
                                uint16_t randomColor = darkGrayColors[Random(5)];
                                ST7735_FillRect(x, y, 8, 8, randomColor);
                            }
                        }
                        
                        for(int y = 121; y < 135; y += 8) {
                            for(int x = 0; x < 128; x += 8) {
                                uint16_t randomColor = darkGrayColors[Random(5)];
                                ST7735_FillRect(x, y, 8, 8, randomColor);
                            }
                        }
                    }
                    
                    // scrolling background looks AWESOME deadass
                    ST7735_DrawBitmap(game.backgroundX, 121, bg_spaceship_2, 128, 82);
                    if(game.backgroundX < 0) {
                        ST7735_DrawBitmap(game.backgroundX + 128, 121, bg_spaceship_2, 128, 82);
                    }
                    
                    if(game.language == English) {
                        ST7735_FillRect(0, 0, 128, 38, ST7735_BLACK);
                        
                        ST7735_DrawString(4, 2, (char*)"GET READY!", ST7735_WHITE);
                    } else { // Spanish
                        ST7735_FillRect(0, 0, 128, 38, ST7735_BLACK);
                        
                        ST7735_DrawString(4, 2, (char*)"PREPARATE!", ST7735_WHITE);
                    }
                    
                    char countdownStr[2];
                    sprintf(countdownStr, "%d", countdownValue);
                    
                    uint16_t countdownColor;
                    if(countdownValue == 3) {
                        countdownColor = ST7735_RED;
                        GPIOB->DOUTSET31_0 = (1<<16);
                        GPIOB->DOUTCLR31_0 = (1<<12)|(1<<17);
                    } else if(countdownValue == 2) {
                        countdownColor = ST7735_YELLOW;
                        GPIOB->DOUTSET31_0 = (1<<12);
                        GPIOB->DOUTCLR31_0 = (1<<16)|(1<<17);
                    } else {
                        countdownColor = ST7735_GREEN;
                        GPIOB->DOUTSET31_0 = (1<<17);
                        GPIOB->DOUTCLR31_0 = (1<<16)|(1<<12);
                    }
                    
                    ST7735_FillRect(0, 136, 128, 24, ST7735_BLACK);
                    
                    ST7735_DrawString(6, 17, countdownStr, countdownColor);
                    
                    if(game.language == English) {
                        ST7735_DrawString(1, 13, (char*)"UP BUTTON = FLY", ST7735_WHITE);
                    } else { // Spanish
                        ST7735_DrawString(0, 13, (char*)"ARRIBA = VOLAR", ST7735_WHITE);
                    }
                    
                    countdownTimer++;
                    
                    if(countdownTimer >= 30) {
                        countdownTimer = 0;
                        countdownValue--;
                        
                        Sound_Coin();
                        
                        if(countdownValue == 0) {
                            game.currentState = PLAYING;
                            firstTimeGetReady = true;
                            
                            game.score = 0;
                            
                            GPIOB->DOUTCLR31_0 = (1<<16)|(1<<12)|(1<<17);
                            
                            ST7735_FillScreen(ST7735_BLACK);
                            
                            Sound_Shoot();
                        }
                    }
                }
                break;

            case PLAYING:
                game.backgroundX -= 2 * game.gameSpeed;  
                
                if(game.backgroundX < -128) {
                    game.backgroundX = 0;      
                }
                
                static bool rectanglesInitialized = false;
                if(!rectanglesInitialized) {
                    const uint16_t darkGrayColors[5] = {
                        ST7735_Color565(41, 41, 41),   
                        ST7735_Color565(43, 43, 43),   
                        ST7735_Color565(44, 44, 44),   
                        ST7735_Color565(45, 45, 45),   
                        ST7735_Color565(46, 46, 46)    
                    };
                    
                    for(int y = 0; y < 39; y += 8) {
                        for(int x = 0; x < 128; x += 8) {
                            uint16_t randomColor = darkGrayColors[Random(5)];
                            ST7735_FillRect(x, y, 8, 8, randomColor);
                        }
                    }
                    
                    for(int y = 121; y < 160; y += 8) {
                        for(int x = 0; x < 128; x += 8) {
                            uint16_t randomColor = darkGrayColors[Random(5)];
                            ST7735_FillRect(x, y, 8, 8, randomColor);
                        }
                    }
                    
                    rectanglesInitialized = true; // never do this again or FPS drops to like 2
                }
                
                // LASER STUFF!!!! This is the dangerous part
                {
                    game.laserSpawnTimer++;
                    
                    if(game.laserSpawnTimer >= (30 + Random(60))) {
                        game.laserSpawnTimer = 0;
                        
                        for(int i = 0; i < MAX_LASERS; i++) {
                            if(!game.lasers[i].active) {
                                game.lasers[i].active = true;
                                game.lasers[i].x = 128;  
                                
                                const int ceilingY = 55;  
                                const int floorY = 120;   
                                
                                game.lasers[i].type = static_cast<LaserType_t>(Random(3)); 
                                
                                game.lasers[i].size = Random(15) + 10; 
                                
                                switch(game.lasers[i].type) {
                                    case HORIZONTAL:
                                        game.lasers[i].y = ceilingY + 2 + Random(floorY - ceilingY - 4);
                                        break;
                                    case VERTICAL:
                                        game.lasers[i].y = ceilingY + Random(floorY - ceilingY - game.lasers[i].size);
                                        break;
                                    case DIAGONAL:
                                        game.lasers[i].y = ceilingY + Random(floorY - ceilingY - game.lasers[i].size);
                                        break;
                                }
                                
                                break;  
                            }
                        }
                    }
                    
                    for(int i = 0; i < MAX_LASERS; i++) {
                        if(game.lasers[i].active) {
                            game.lasers[i].x -= 2 * game.gameSpeed;  
                            
                            if(game.lasers[i].x < -game.lasers[i].size) {
                                game.lasers[i].active = false;
                            } else {
                                switch(game.lasers[i].type) {
                                    case HORIZONTAL:
                                        ST7735_FillRect(game.lasers[i].x, game.lasers[i].y - 1, game.lasers[i].size, 4, ST7735_RED);
                                        ST7735_FillRect(game.lasers[i].x, game.lasers[i].y, game.lasers[i].size, 2, ST7735_Color565(255, 100, 100)); 
                                        break;
                                    case VERTICAL:
                                        ST7735_FillRect(game.lasers[i].x - 1, game.lasers[i].y, 4, game.lasers[i].size, ST7735_RED);
                                        ST7735_FillRect(game.lasers[i].x, game.lasers[i].y, 2, game.lasers[i].size, ST7735_Color565(255, 100, 100)); 
                                        break;
                                    case DIAGONAL:
                                        for(int j = 0; j < game.lasers[i].size; j++) {
                                            ST7735_DrawPixel(game.lasers[i].x + j, game.lasers[i].y + j, ST7735_Color565(255, 100, 100)); 
                                            ST7735_DrawPixel(game.lasers[i].x + j - 1, game.lasers[i].y + j, ST7735_RED); 
                                            ST7735_DrawPixel(game.lasers[i].x + j + 1, game.lasers[i].y + j, ST7735_RED); 
                                            ST7735_DrawPixel(game.lasers[i].x + j, game.lasers[i].y + j - 1, ST7735_RED); 
                                            ST7735_DrawPixel(game.lasers[i].x + j, game.lasers[i].y + j + 1, ST7735_RED); 
                                        }
                                        break;
                                }
                            }
                        }
                    }
                }
                
                // COIN logic
                {
                    game.coinSpawnTimer++;
                    
                    if(game.coinSpawnTimer >= (60 + Random(40))) {
                        game.coinSpawnTimer = 0;
                        
                        const int ceilingY = 55;  
                        const int floorY = 120;   
                        const int coinWidth = 6;  
                        const int coinHeight = 6; 
                        const int spacing = 4;    
                        
                        int activeCoins = 0;
                        for(int i = 0; i < MAX_COINS; i++) {
                            if(game.coins[i].active) {
                                activeCoins++;
                            }
                        }
                        
                        if(activeCoins < MAX_COINS - 3) { 
                            CoinFormation_t formation = (Random(100) < 60) ? HORIZONTAL_LINE : VERTICAL_LINE;
                            
                            int32_t formationId = Random32();
                            
                            int formationLength = 3 + Random(3);
                            if(formationLength > (MAX_COINS - activeCoins)) {
                                formationLength = MAX_COINS - activeCoins; 
                            }
                            
                            int startX = 128; 
                            int startY;
                            
                            if(formation == HORIZONTAL_LINE) {
                                startY = ceilingY + 10 + Random(floorY - ceilingY - 20 - coinHeight);
                            } else { // VERTICAL_LINE
                                int verticalSpace = (formationLength * (coinHeight + spacing)) - spacing;
                                startY = ceilingY + 10 + Random(floorY - ceilingY - 20 - verticalSpace);
                            }
                            
                            bool overlapsLaser = false;
                            for(int i = 0; i < MAX_LASERS; i++) {
                                if(game.lasers[i].active) {
                                    int laserX = game.lasers[i].x;
                                    int laserY = game.lasers[i].y;
                                    int laserWidth = 0;
                                    int laserHeight = 0;
                                    
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
                                    
                                    if(formation == HORIZONTAL_LINE) {
                                        int formationWidth = (formationLength * (coinWidth + spacing)) - spacing;
                                        if(startX < laserX + laserWidth + 20 && 
                                           startX + formationWidth > laserX - 20 &&
                                           startY < laserY + laserHeight + 5 &&
                                           startY + coinHeight > laserY - 5) {
                                            overlapsLaser = true; 
                                            break;
                                        }
                                    } else { // VERTICAL_LINE
                                        int formationHeight = (formationLength * (coinHeight + spacing)) - spacing;
                                        if(startX < laserX + laserWidth + 20 && 
                                           startX + coinWidth > laserX - 20 &&
                                           startY < laserY + laserHeight + 5 &&
                                           startY + formationHeight > laserY - 5) {
                                            overlapsLaser = true; 
                                            break;
                                        }
                                    }
                                }
                            }
                            
                            if(!overlapsLaser) {
                                int coinsPlaced = 0;
                                for(int i = 0; i < MAX_COINS && coinsPlaced < formationLength; i++) {
                                    if(!game.coins[i].active) {
                                        game.coins[i].active = true;
                                        game.coins[i].formation = formation;
                                        game.coins[i].formationId = formationId;
                                        
                                        if(formation == HORIZONTAL_LINE) {
                                            game.coins[i].x = startX + (coinsPlaced * (coinWidth + spacing));
                                            game.coins[i].y = startY;
                                        } else { // VERTICAL_LINE
                                            game.coins[i].x = startX;
                                            game.coins[i].y = startY + (coinsPlaced * (coinHeight + spacing));
                                        }
                                        
                                        coinsPlaced++;
                                    }
                                }
                            }
                        }
                    }
                    
                    for(int i = 0; i < MAX_COINS; i++) {
                        if(game.coins[i].active) {
                            game.coins[i].x -= 2 * game.gameSpeed;
                            
                            if(game.coins[i].x < -6) {  
                                game.coins[i].active = false;
                            } else {
                                ST7735_DrawBitmap(game.coins[i].x, game.coins[i].y, coin0, 6, 6);
                            }
                        }
                    }
                }
                
            {  // Barry's movement block
                static int barryY = 95;              
                static float barryVelocity = 0;       
                static int lastBarryY = -999;         
                
                static const float GRAVITY = 0.3;     
                static const float THRUST = -0.6;     
                static const float TERMINAL_VEL = 4;  
                static const float MAX_THRUST = -3;   
                
                if(buttons & BUTTON_UP) {
                    barryVelocity += THRUST;  // JETPACK GOES BRRRRRR!!!
                    if(barryVelocity < MAX_THRUST) {
                        barryVelocity = MAX_THRUST;
                    }
                } else {
                    barryVelocity += GRAVITY;
                    
                    if(barryVelocity > TERMINAL_VEL) {
                        barryVelocity = TERMINAL_VEL;
                    }
                }
                
                barryY += (int)barryVelocity;
                
                if(barryY < 70) {  
                    barryY = 70;  
                    barryVelocity = 0; 
                }
                if(barryY > 120) {
                    barryY = 120; 
                    barryVelocity = 0; 
                }
                
                __disable_irq();
                
                if(barryY != lastBarryY || game.backgroundX % 2 == 0) {
                    ST7735_DrawBitmap(game.backgroundX, 121, bg_spaceship_2, 128, 82);
                    if(game.backgroundX < 0) {
                        ST7735_DrawBitmap(game.backgroundX + 128, 121, bg_spaceship_2, 128, 82);
                    }
                    
                    ST7735_DrawBitmap(15, barryY, barry3, 18, 30);
                }
                
                __enable_irq();
                
                lastBarryY = barryY;
                
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
                
                game.score += (uint32_t)(1 * game.gameSpeed);
                
                const int BARRY_X = 15;          
                const int BARRY_WIDTH = 18;      
                const int BARRY_HEIGHT = 30;     
                
                const int BARRY_HITBOX_X = BARRY_X + 3;           
                const int BARRY_HITBOX_Y = barryY - BARRY_HEIGHT + 8;  
                const int BARRY_HITBOX_WIDTH = BARRY_WIDTH - 6;   
                const int BARRY_HITBOX_HEIGHT = BARRY_HEIGHT - 16; 
                
                for(int i = 0; i < MAX_LASERS; i++) {
                    if(game.lasers[i].active) {
                        int laserX = game.lasers[i].x;
                        int laserY = game.lasers[i].y;
                        int laserWidth = 0;
                        int laserHeight = 0;
                        
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
                        
                        if(CheckCollision(BARRY_HITBOX_X, BARRY_HITBOX_Y, BARRY_HITBOX_WIDTH, BARRY_HITBOX_HEIGHT, 
                                         laserX, laserY, laserWidth, laserHeight)) {
                            Sound_Shoot();  
                            
                            game.currentState = GAME_OVER;
                            
                            break;
                        }
                    }
                }
                
                for(int i = 0; i < MAX_COINS; i++) {
                    if(game.coins[i].active) {
                        const int COIN_WIDTH = 6;
                        const int COIN_HEIGHT = 6;
                        
                        if(CheckCollision(BARRY_HITBOX_X, BARRY_HITBOX_Y, BARRY_HITBOX_WIDTH, BARRY_HITBOX_HEIGHT, 
                                          game.coins[i].x, game.coins[i].y, COIN_WIDTH, COIN_HEIGHT)) {
                            game.coinsCollected++;  
                            Sound_Coin();           // make that sweet coin sound!!
                            
                            game.coins[i].active = false;
                        }
                    }
                }
            }
            break;
                
            case GAME_OVER: // GAME OVER SCREEN 
                static bool firstTimeGameOver = true;
                if(firstTimeGameOver) {
                    ST7735_FillScreen(ST7735_BLACK); 
                    firstTimeGameOver = false;
                }
                
                if(game.language == English) {
                    ST7735_DrawString(3, 3, (char*)"GAME OVER", ST7735_RED); 
                    
                    char scoreStr[20];
                    sprintf(scoreStr, "SCORE: %d", game.score);
                    ST7735_DrawString(3, 6, scoreStr, ST7735_WHITE);
                    
                    char coinStr[20];
                    sprintf(coinStr, "COINS: %d", game.coinsCollected);
                    ST7735_DrawString(3, 8, coinStr, ST7735_WHITE);
                    
                    char difficultyStr[20] = "DIFFICULTY: ";
                    switch(game.difficulty) {
                        case EASY:
                            strcat(difficultyStr, "EASY"); 
                            ST7735_DrawString(1, 10, difficultyStr, ST7735_GREEN); 
                            break;
                        case MEDIUM:
                            strcat(difficultyStr, "MEDIUM");
                            ST7735_DrawString(1, 10, difficultyStr, ST7735_YELLOW); 
                            break;
                        case HARD:
                            strcat(difficultyStr, "HARD");
                            ST7735_DrawString(1, 10, difficultyStr, ST7735_RED); 
                            break;
                    }
                    
                    ST7735_DrawString(1, 13, (char*)"PRESS RIGHT BUTTON", ST7735_WHITE);
                    ST7735_DrawString(2, 14, (char*)"FOR DIFFICULTY", ST7735_WHITE);
                } else { // Spanish
                    ST7735_DrawString(3, 3, (char*)"FIN DEL JUEGO", ST7735_RED); 
                    
                    char scoreStr[20];
                    sprintf(scoreStr, "PUNTOS: %d", game.score);
                    ST7735_DrawString(3, 6, scoreStr, ST7735_WHITE);
                    
                    char coinStr[20];
                    sprintf(coinStr, "MONEDAS: %d", game.coinsCollected);
                    ST7735_DrawString(3, 8, coinStr, ST7735_WHITE);
                    
                    char difficultyStr[20] = "DIFICULTAD: ";
                    switch(game.difficulty) {
                        case EASY:
                            strcat(difficultyStr, "FACIL"); 
                            ST7735_DrawString(1, 10, difficultyStr, ST7735_GREEN);
                            break;
                        case MEDIUM:
                            strcat(difficultyStr, "MEDIO"); 
                            ST7735_DrawString(1, 10, difficultyStr, ST7735_YELLOW);
                            break;
                        case HARD:
                            strcat(difficultyStr, "DIFICIL"); 
                            ST7735_DrawString(1, 10, difficultyStr, ST7735_RED);
                            break;
                    }
                    
                    ST7735_DrawString(1, 13, (char*)"PRESIONA DERECHA", ST7735_WHITE);
                    ST7735_DrawString(2, 14, (char*)"PARA DIFICULTAD", ST7735_WHITE);
                }
                
                if(buttons & BUTTON_RIGHT) {
                    game.currentState = DIFFICULTY_SELECT;  
                    game.score = 0; 
                    game.coinsCollected = 0;  
                    game.backgroundX = 0; 
                    
                    for(int i = 0; i < MAX_LASERS; i++) {
                        game.lasers[i].active = false;
                    }
                    
                    for(int i = 0; i < MAX_COINS; i++) {
                        game.coins[i].active = false;
                    }
                    
                    firstTimeGameOver = true;
                    
                    ST7735_FillScreen(ST7735_BLACK);
                    Clock_Delay1ms(100); 
                }
                break;
            }
        }
    }
}
