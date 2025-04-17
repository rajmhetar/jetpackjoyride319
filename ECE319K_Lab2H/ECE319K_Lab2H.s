//****************** ECE319K_Lab2H.s ***************
// Your solution to Lab 2 in assembly code
// Author: Joshua Lee
// Last Modified: 2/5
// ECE319H Spring 2025 (ECE319K students do Lab2)

    .include "../inc/msp.s"

        .data
        .align 2
// Declare global variables here if needed
// with the .space assembly directive

        .text
        .thumb
        .align 2
        .global EID
EID:    .string "JHL3263" // replace ZZZ123 with your EID here
        .align 2
  .equ dot,8000000
  .equ dash,(3*dot)
  .equ shortgap,(2*dot)  // because it will send an interelement too
  .equ interelement,dot
  Morse:
  .long  dot,  dash,    0,    0, 0 // A
  .long  dash,  dot,  dot,  dot, 0 // B
  .long  dash,  dot, dash,  dot, 0 // C
  .long  dash,  dot,  dot,    0, 0 // D
  .long  dot,     0,    0,    0, 0 // E
  .long  dot,   dot, dash,  dot, 0 // F
  .long  dash, dash,  dot,    0, 0 // G
  .long  dot,   dot,  dot,  dot, 0 // H
  .long  dot,   dot,    0,    0, 0 // I
  .long  dot,  dash, dash, dash, 0 // J
  .long  dash,  dot, dash,    0, 0 // K
  .long  dot,  dash,  dot,  dot, 0 // L
  .long  dash, dash,    0,    0, 0 // M
  .long  dash,  dot,    0,    0, 0 // N
  .long  dash, dash, dash,    0, 0 // O
  .long  dot,  dash, dash,  dot, 0 // P
  .long  dash, dash,  dot, dash, 0 // Q
  .long  dot,  dash,  dot,    0, 0 // R
  .long  dot,   dot,  dot,    0, 0 // S
  .long  dash,    0,    0,    0, 0 // T
  .long  dot,   dot, dash,    0, 0 // U
  .long  dot,   dot,  dot, dash, 0 // V
  .long  dot,  dash, dash,    0, 0 // W
  .long  dash,  dot,  dot, dash, 0 // X
  .long  dash,  dot, dash, dash, 0 // Y
  .long  dash, dash,  dot,  dot, 0 // Z

  .align 2
  .global Lab2Grader
  .global Lab2
  .global Debug_Init // Lab3 programs
  .global Dump       // Lab3 programs

// Switch input: PB2 PB1 or PB0, depending on EID
// LED output:   PB18 PB17 or PB16, depending on EID
// logic analyzer pins PB18 PB17 PB16 PB2 PB1 PB0
// analog scope pin PB20
Lab2:
// Initially the main program will
//   set bus clock at 80 MHz,
//   reset and power enable both Port A and Port B
// Lab2Grader will
//   configure interrupts  on TIMERG0 for grader or TIMERG7 for TExaS
//   initialize ADC0 PB20 for scope,
//   initialize UART0 for grader or TExaS
    MOVS R0,#3 // 0 for info, 1 debug with logic analyzer, 2 debug with scope, 10 for grade
    //BL   Lab2Grader
    //BL   Lab2Init   // you initialize input pin and output pin
    BL Debug_Init
loop:

//Stage 2
// wait for the switch to be released

LetterO:
    BL Dash
    BL Dash
    BL Dash
    B loop
Release:
    BL SWITCH_IN
    CMP R0, #0
    BNE Release

// wait for the switch to be pressed
Press:
    BL SWITCH_IN
    CMP R0, #1
    BNE Press
    
    MOVS R0,#3      // TExaS in logic analyzer mode
    BL   Lab2Grader // bus clock to 80 MHz
// Grader will return R0 as a random character from ‘A’ to ‘Z’
// send that random one letter
    LDR R1, =Morse //Address of first letter
    LDR R3, =0x41 //
    SUBS R0, R0, R3 //R0 <- How many letters to increment

//Store the letter address into R1
LETTER_INC:
    CMP R0, #0
    BEQ PRINT_LETTER
    ADDS R1, R1, #20
    SUBS R0, R0, #1
    B LETTER_INC


PRINT_LETTER:
    PUSH {R0, R2}
    LDR R0, [R1] //check if we print a dot, dash, or finish
    CMP R0, #0
    BEQ PRINT_EXIT
    LDR R2, =dot
    CMP R0, R2
    BEQ DOTGO
    LDR R2, =dash
    CMP R0, R2
    BEQ DASHGO

//If we printed a symbol, check the next symbol/null
NEXT_SYM: 
    ADDS R1, R1, #4
    B PRINT_LETTER

//If null, letter is done.
PRINT_EXIT:
    POP {R0, R2}
      B loop

DOTGO:
    PUSH {R1}
    BL Dot
    POP {R1}
    B NEXT_SYM

DASHGO:
    PUSH {R1}
    BL Dash
    POP {R1}
    B NEXT_SYM

Dot:  
    PUSH {LR}
    BL LED_ON

    LDR R0, =dot
    BL DELAY

    BL LED_OFF

    LDR R0, =interelement
    BL DELAY

    POP {PC}

Dash: 
    PUSH {LR}

    BL LED_ON

    LDR R0, =dash
    BL DELAY

    BL LED_OFF

    LDR R0, =interelement
    BL DELAY

    POP {PC}

SWITCH_IN:

    //Put Switch State in R0
    LDR R0, =GPIOB_DIN31_0
    LDR R1, [R0]
    LDR R2, =0x08 //Read PB3 bit
    ANDS R1, R1, R2
    LSRS R0, R1, #3
    BX LR


DELAY: SUBS R0,R0,#2
D_LOOP: SUBS R0,R0,#4 
       NOP
       BHS  D_LOOP
       BX   LR
    
LED_ON:

    LDR R0, =GPIOB_DOUT31_0
    LDR R1, [R0]
    LDR R2, =0x20000
    ORRS R1, R1, R2
    STR R1, [R0]
    PUSH {LR}
    BL Dump
    POP {PC}
    BX LR

LED_OFF:

    LDR R1,=GPIOB_DOUTCLR31_0
    LDR R2, =0x20000
    STR R2,[R1]  // PB17=0
    PUSH {LR}
    BL Dump
    POP {PC}
    BX LR
       

// make switch an input, LED an output
// PortB is already reset and powered
// Set IOMUX for your input and output
// Set GPIOB_DOE31_0 for your output (be friendly)
Lab2Init:
// ***do not reset/power Port A or Port B, already done****
   
   // Input Setup
   LDR R0, =IOMUXPB3
   LDR R1, =0x40081 //Sets bit 18, 7, and 00001 for bits 5-0
   LDR R2, [R0]
   ORRS R2, R2, R1
   STR R2,[R0]
    
   //Output Setup
    LDR R0, =IOMUXPB17
    LDR R1, =0x81 //set bits 7 and 00001 for 5-0
    LDR R2, [R0]
    ORRS R2, R2, R1
    STR R2, [R0]

    LDR  R0, =GPIOB_DOE31_0
    LDR  R1,[R0]
    LDR R2, =0x20000 //Set PB17
    ORRS R1, R1, R2
    STR  R1,[R0] 


   BX   LR


   .end
