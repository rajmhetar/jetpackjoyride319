//****************** ECE319K_Lab1.s ***************
// Your solution to Lab 1 in assembly code
// Author: Your name
// Last Modified: Your date
// Spring 2025
        .data
        .align 2
// Declare global variables here if needed
// with the .space assembly directive

        .text
        .thumb
        .align 2
        .global EID
EID:    .string "JHL3263" // replace ZZZ123 with your EID here

        .global Phase
        .align 2
Phase:  .long 10
// Phase= 0 will display your objective and some of the test cases, 
// Phase= 1 to 5 will run one test case (the ones you have been given)
// Phase= 6 to 7 will run one test case (the inputs you have not been given)
// Phase=10 will run the grader (all cases 1 to 7)
        .global Lab1
// Input: R0 points to the list
// Return: R0 as specified in Lab 1 assignment and terminal window
// According to AAPCS, you must save/restore R4-R7
// If your function calls another function, you must save/restore LR
Lab1: PUSH {R4-R7,LR}
        // your solution goes here
LOOP:   
        LDR R1, [R0]
        CMP R1, #0
        BEQ NOMATCH
        LDR R2, =EID
        MOVS R5, #0
        BL COMPARE
        CMP R5, #1
        BEQ MATCH
        ADDS R0, #8
        B LOOP

MATCH:
        LDR R0, [R0, #4]
        B EXIT

NOMATCH:
        LDR R0, =0xFFFFFFFF
        B EXIT

COMPARE:
        LDRB R3, [R1]
        LDRB R4, [R2]
        CMP R4, #0
        BEQ EQUAL
        CMP R3, R4
        BNE RETURN
        ADDS R1, #1
        ADDS R2, #1
        B COMPARE

EQUAL:
        MOVS R5, #1
        B RETURN

RETURN:
        BX LR

EXIT:
        POP  {R4-R7,PC} // return


        .align 2
        .global myClass
myClass: .long pAB123  // pointer to EID
         .long 95      // Score
         .long pXYZ1   // pointer to EID
         .long 96      // Score
         .long pAB5549 // pointer to EID
         .long 94      // Score
         .long 0       // null pointer means end of list
         .long 0
pAB123:  .string "AB123"
pXYZ1:   .string "XYZ1"
pAB5549: .string "AB5549"
        .end
