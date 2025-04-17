// Example in Lecture 1/15 Yerraballi
/* date
what
why
*/
// Global Variables go in RAM
       .data
       .align 2 // all declared entities are 2^2 aligned
Score: .byte 95
.align
Sum:   .short -1025
.align
value:  .long 0x1E240 // number 123456
Xvalues: .long 234567, -123456, -65, 125, 0
Result: .space 2
// Constants and Code
       .text
       .thumb
       .align 2
Vowels: .byte 'a', 'e', 'i', 'o', 'u'
.align
Filter: .short 1096, 2025, 1905, -1
Name:   .string "John Doe"
       .global main
main: 
       // Increment Score by 2
       LDR R0, =Score
       LDRB R1, [R0]
       ADDS R1, #2
       STRB R1, [R0]
loop:
       // Functionality of the ES
       B loop

       .end