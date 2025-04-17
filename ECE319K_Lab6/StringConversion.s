// StringConversion.s
// Student names: change this to your names or look very silly
// Last modification date: change this to the last modification date or look very silly
// Runs on any Cortex M0
// ECE319K lab 6 number to string conversion
//
// You write udivby10 and Dec2String
     .data
     .align 2
// no globals allowed for Lab 6
    .global OutChar    // virtual output device
    .global OutDec     // your Lab 6 function
    .global Test_udivby10

    .text
    .align 2
// **test of udivby10**
// since udivby10 is not AAPCS compliant, we must test it in assembly
Test_udivby10:
    PUSH {LR}

    MOVS R0,#123
    BL   udivby10
// put a breakpoint here
// R0 should equal 12 (0x0C)
// R1 should equal 3

    LDR R0,=12345
    BL   udivby10
// put a breakpoint here
// R0 should equal 1234 (0x4D2)
// R1 should equal 5

    MOVS R0,#0
    BL   udivby10
// put a breakpoint here
// R0 should equal 0
// R1 should equal 0
    POP {PC}

//****************************************************
// divisor=10
// Inputs: R0 is 16-bit dividend
// quotient*10 + remainder = dividend
// Output: R0 is 16-bit quotient=dividend/10
//         R1 is 16-bit remainder=dividend%10 (modulus)
// not AAPCS compliant because it returns two values
udivby10:
   PUSH {LR}
   PUSH {R2-R6}
// write this
      MOVS R2, #0 // Total Quotient
      MOVS R3, R0 //working remainder
      MOVS R4, #10 //Divisor = 10
      MOVS R5, #0 //Shift Counter

ShiftLoop: //shift divisor until it is greater than the dividend and count how many times
        CMP     R3, R4
        BLT     ShiftDone 
        LSLS     R4, R4, #1 
        ADDS     R5, R5, #1  
        B       ShiftLoop

ShiftDone: //Shift right one more time to adjust
        LSRS     R4, R4, #1    
        SUBS     R5, R5, #1   
DivLoop:
        CMP     R5, #0
        BLT     DivisionDone  
        CMP     R3, R4         
        BLT     SkipSubtract   
        SUBS     R3, R3, R4    
        MOVS     R6, #1
        LSLS     R6, R6, R5   
        ORRS     R2, R2, R6   
SkipSubtract:
        LSRS     R4, R4, #1    
        SUBS    R5, R5, #1    
        BGE     DivLoop  

DivisionDone:
        MOV     R0, R2       
        MOV     R1, R3    

   POP {R2-R6}
   POP  {PC}

  
//-----------------------OutDec-----------------------
// Convert a 16-bit number into unsigned decimal format
// Call the function OutChar to output each character
// You will call OutChar 1 to 5 times
// OutChar does not do actual output, OutChar does virtual output used by the grader
// Input: R0 (call by value) 16-bit unsigned number
// Output: none
// Invariables: This function must not permanently modify registers R4 to R11
OutDec2:
   PUSH {LR}
// write this

   POP  {PC}
//* * * * * * * * End of OutDec * * * * * * * *

// ECE319H recursive version
// Call the function OutChar to output each character
// You will call OutChar 1 to 5 times
// Input: R0 (call by value) 16-bit unsigned number
// Output: none
// Invariables: This function must not permanently modify registers R4 to R11

.equ quotient, 0      // Binding: quotient is at SP+0

OutDec:
   PUSH {LR}
   PUSH {R4-R6}         
   
   CMP   R0, #10 
   BLT   BaseCase      

   MOVS  R4, R0      
   MOVS  R5, #0 //quotient

DivLoopR: //compute quotient
   CMP   R4, #10      
   BLT   DivDone
   SUBS  R4, R4, #10      
   ADDS  R5, R5, #1 
   B     DivLoopR 

DivDone:
   MOV   R6, R4  
   SUB   SP, SP, #8            // Allocation
   STR   R5, [SP, #quotient]    // Binding and access
   LDR   R0, [SP, #quotient]    // Access
   ADD   SP, SP, #8            // Deallocation
   BL    OutDec                // Recursion

   ADDS  R6, R6, #48 //output character        
   MOVS  R0, R6             
   BL    OutChar               

   B     End      

BaseCase:
   ADDS  R0, R0, #48         
   BL    OutChar   

End:
   POP {R4-R6}
   POP {PC}            
   .end

