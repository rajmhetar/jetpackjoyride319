/* My first C Program
 Date: Spring 2025
 Author: Ramesh Yerraballi
 The purpose of this program is to get you started in C programming
 by walking you through the basics:
 1. Directives, Variables, Constants
 2. Assignment, Conditional, Iterative Loops (For/While)
 3. Functions: 3 Is: Interface(Prototype), Implementation (Declaration) and Invocation(Call)
 4. Sources in multiple files with header files to connect
*/
/**** Pre-processor Directives Section ****/
//    The # character in front is indicative of
//    of pre-processor directives
// a. #define is used to declare Literals or Macros
 // Format to declare Literals: 
 //    #define symbol value
 //    1. They do not take space
 //    2. The compiler does a "literal"
 //       substitution of value wherever the
 //       symbol is used in code
#define TRUE 1
#define FALSE 0
#define Cutoff 70
#define Greeting "Howdy"
#define CLASS_SIZE 10
 //  Format to declare Macros:
 //  #define Macro_Name(arguments) expression
 //  Macros are like functions which take inputs and
 //  may produce outputs.  
#define Max(a,b) ((a>b)?(a):(b)) // Computes the Max of the two inputs
     // **Parenthesis are important in Macros!!
// b. #include is used to import information from
 //    other files (called header files)
 // Format:
 //    #include <name_of_system_file.h>
 //    or
 //    #include "name_of_user_file.h"
#include <ti/devices/msp/msp.h> // Imports MSPM0 Device Register Values
#include <stdint.h>    // Imports C99 data-type definitions
#include "myfun.h"     // This is a user defined module we will build

/**** Variables+Constants Declaration Section ****/
// This is where we declare things that take up memory
// Constants (in ROM) and Variables (in RAM) that are globally
// accessible; Constants and Variables can be inter-mixed 
// a. Variables can be single elements or a collection of elements (arrays)
//    Format:
//       Data_Type VariableName = Initial_Value;
//                              ^^^^^^^^^^^^^^^
//                                   optional
//       Data_Tpe ArrayName[Length] = {Initial values comma-separated};
//                                  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//                                               optional
// ** Multiple variables of the same time can be declared together.
// Examples
uint8_t Score, AScore, QScore, Error=FALSE;
char Grade;
uint8_t Scores[CLASS_SIZE];
char Grades[CLASS_SIZE];
int8_t Nums1[5] = {-4, 6, -12, 3, -6}; // Arrays are initialized by listing
                                       // values in curly braces
//  b. Constants are declared exactly like Variables with two exceptions:
//      i. The declaration must have the keyword "const" in front
//      ii. The Initial value(s) are not optional
// Examples
const uint16_t magic = 0x1234;
const uint32_t Primes[4] = {3, 11, 17, 31};
const int8_t Nums2[8] = {-4, 6, -12, 3, -6, -3, 4, 1};
/**** Function Interfaces(Prototypes) Declaration Section ****/
// This is where we declare the prototypes of functions that will be called
// in main or in other functions
// We do this for two reasons:
//   1. The C compiler is a single-pass process (unlike an assembler which is two-passes). 
//      If you call a function and the the C compiler has not seen it yet then
//      it has no way of checking if the function exists
//   2. The C compiler checks every function call for correctness, i.e., whether
//      the call is passing to the function what it is expecting and using the
//      result (if any) correctly.
// *** Ideally you should put comments around the prototype to
//     explain "what" the function does, its interface that is.
// Format:
//         Data_Type FunctionName(Comma-separated list of variable/constant declarations)
// ** The Data_Type can be "void" if nothing is returned by the function
// ** The list can be empty if the functions expects NO inputs.
// Examples:
char ComputeGrade(uint8_t); // Takes a 8-bit unsigned int score and computes
                            // and returns a letter grade 'P' or 'F'
uint32_t FSum(uint8_t);     // Takes a 8-bit unsigned int N as input and returns
                            // a 32-bit unsigned int sum: 1+2+3+...+N 
int32_t NumsSum(int8_t[], uint8_t); // Takes an array of  8-bit unsigned 
                            // integers and its size as input and returns
                            // a 32-bit unsigned int sum of the array elements
/**** Function Implementations  Section ****/
// This is where we write the actual implementations of main and the functions
// whose prototypes we declared in the previous section
int main(){
	// Statements inside a C Function are of 4 types: 
	//          Declaration, Assignment, Conditional, Iterative
	//   a. Declaration statements are similar in format to Variable/Constant/prototype
	//      declarations that are outside the functions. However, declaring
	//      them inside a function makes them "local" thus restricting their scope.
  // When a local variable is set to a value but never read the compiler complains
  // that you are doing something pointless. Use valatile to suppress this!	
  volatile uint32_t SumF;
	volatile int32_t nsum, nFact;
	volatile char Final;
	//   b. Assignment Statements
	//      Format:
	//             Variable = Expression;
	//      ** The variable can be simple variable or an array member.
	//      ** The expression can be a mix of arithmetic/logic/shift oeprations
	//         with calls to functions embedded.
	//      ** The espression value and the destination variable must be compatible

	AScore = 94;
	Scores[2] = 75;
	Scores[3] = 60;
  QScore = Scores[2] + 10;
	Score = Max(AScore, QScore);  // We are using the Max macro here defined above
	                              // Be careful about using macros. The C compiler
																// will not do any type checking!!
	//   c. Condition Statements
	//      Format:
	//            if (condition){
	//               // True Block code goes here
  //            } else {
	//               // False Block code goes here
	//            }
	//  ** The condition can be any expression that evaluates to True or False
	//     Typically comparison statements (a combination possibly)
	//  ** The else part if optional
	// Examples:
	if (AScore > Cutoff){
		Grade = 'P';
	} else {
		Grade = 'F';
	}
	// Here we are using a combination of conditions &&: AND; ||: OR
	if ((AScore >= Cutoff)&&(QScore >= Cutoff)){
		Final = 'P';
	} else {
		Final = 'F';
	}
	//   d. Iterative Statements - Loops
  //     	Format:
  //             while (condition){
	//                // Body of code to repeat when the
	//                // codition is true
	//             }
	// Example: Lets compute the Sum 1+2+3+4+5
	uint32_t i, Sum;
	Sum = 0;
	i = 0;
	while (i <= 5){
		Sum = Sum + i;   // Sum += i does the same
		i = i+1;         // i+=1 or i++ do the same
	}
	// We could have also done Sum as 5+4+3+2+1
	Sum = 0;
	i = 5;
	while (i > 0){
		Sum += i;
		i--;
	}
	// A for loop is more natural when while loops fit a certain pattern:
	//      initialize; // maybe a variable
	//      while (condition){
	//         Body
	//         update; // maybe a variable
    //      }
	// It can be written as a for loop like so:
	//      for (initialize; condition; update){
	//         Body
	//      }
	// The above Sum computation can be rewritten using a for loop as:
	for (i = 0; i <= 5; i++){
	   Sum +=i;
    }
	// Invocation/Call a Function
	SumF = FSum(10); // Call FSum with N=10; SumF will be 10*11/2 = 55
	SumF = FSum(13); // Call FSum with N=13; SumF will be 13*14/2 = 91
	// Call another function that expects an Array and length
	nsum = NumsSum(Nums1, 5); // What would the result be if the second
	                          // input to the function was 4?
	// Here is another call: Uncomment it to see what happens!!
    //nsum = NumsSum(Nums2, 8); // Compiler will give you an error on this line:
	                          // (some C compilers may just warn you)
	                          // You are passing/sending an
	                          // array of constants when the function is
	                          // is expecting an array of elements that 
	                          // can be changed; Either fix the Function
	                          // implementation or what you are passing!
	
	// Usually, we write code in multiple files. Each file is
  // a module in your system. As an example we
  // have a module called myfun for which we have two
  // files:
  // 	1. A C source file (myfun.c) which has the implementations 
	//     of the functions that make up our module
	//  2. A C header file (myfun.h) which has the interface(prototypes) 
	//     of the module functions
  // A user who want's to use this module only needs to look at
	// the interface to know what functionality the module provides
  // and how to invoke them - She does this by
	//  1. Doing a #include "myfun.h"
	//  2. Calling the functions
	// Here we are going to call two functions that the module provides
	nFact = Factorial(5); // Compute 5!
	// We will call the two different ways of calulating 1+2+3+..10
	// and compare them. Note our comparison is still of two
	// numbers but they come from calling two functions
	if (SumFormula(10) != FSum(10)){
	   Error = TRUE;
	}
	
  // Embedded Systems never stop running! 
  // The following while loop runs forever 
	while(TRUE){
	}
	
}

/* FSum
   A Function to Compute the Sum for a generic value N
   Input: N is a unsigned 8-bit number
   Output: Return the sum 1+2+3+...+N
*/
uint32_t FSum(uint8_t N){
	// Variables declared inside a function are local to the functions
	// Which means their scope is within the function
	//  i, Sum are local to FSum
	uint8_t i;
	uint32_t Sum=0;
	for (i=0; i <= N; i++){
		Sum += i;
	}
	return Sum;
}

/* NumsSum Computes the Sum of the elements of an array
Inputs: 1. An array of 8-bit signed numbers
        2. How many elements of the Array to add
Output: The sum of Len elements in the input array
Note: Len is simply how many elements of the array
      you want added, which in this case be any value 
      between 0 to the Size of the array;
      If Len = Size of array then all elements are added
*/
int32_t NumsSum(int8_t Nums[], uint8_t Len){
	int8_t i;
	int32_t Sum=0;
	
	for(i=0; i < Len; i++){
		Sum += Nums[i]; // Access elements using [] operator with index inside
	}
	return Sum;
}
