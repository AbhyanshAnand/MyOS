#ifndef KMATH_H // Standard header guard to prevent multiple inclusions
#define KMATH_H

#include <stdint.h> // Includes standard integer types like int

// --- Function Declarations for Basic Math Operations ---

// k_add_n: Performs integer addition for an array of numbers.
// Parameters:
//   numbers: A pointer to the first element of an array of integers.
//   count: The number of integers in the 'numbers' array.
// Returns:
//   The sum of all integers in the 'numbers' array.
//   Returns 0 if count is 0.
int k_add_n(const int* numbers, int count);

// k_subtract: Performs integer subtraction (binary operation).
// Parameters:
//   a: The first integer operand (minuend).
//   b: The second integer operand (subtrahend).
// Returns:
//   The result of 'a' minus 'b'.
int k_subtract(int a, int b);

// k_multiply_n: Performs integer multiplication for an array of numbers.
// Parameters:
//   numbers: A pointer to the first element of an array of integers.
//   count: The number of integers in the 'numbers' array.
// Returns:
//   The product of all integers in the 'numbers' array.
//   Returns 1 if count is 0 (multiplicative identity).
int k_multiply_n(const int* numbers, int count);

// k_divide: Performs integer division (binary operation).
// Parameters:
//   numerator: The integer to be divided.
//   denominator: The integer to divide by.
// Returns:
//   The result of 'numerator' divided by 'denominator'.
//   Returns 0 if 'denominator' is 0 to prevent a division-by-zero error,
//   and prints a warning.
int k_divide(int numerator, int denominator);

#endif // KMATH_H
