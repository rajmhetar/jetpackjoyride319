/* This module implements some simple
   math functions shown to demonstrate
	 splitting your project into multiple files
*/
#include <stdint.h>
/* This function computes the Sum 1+2+3+...+N
Input: N is a 8-bit unsigned number
Output: N(N+1)/2
*/
uint32_t SumFormula(uint8_t N){
	uint32_t res;
  
	res = N*(N+1)/2;
	return res;
}

/* This function computes the factorial of N: N!
Input: N is a 8-bit unsigned number
Output: N!
*/
uint32_t Factorial(uint8_t N){
	uint8_t i;
	uint32_t prod=1;
	
	for (i=N; i > 0; i--){
		prod *= i;
	}
	return prod;
}

