#include "kmath.h"  // Include the header for our math functions' declarations
#include "kprint.h" // For kprint to display error messages (e.g., division by zero)

// --- Function: k_add_n ---
// Performs integer addition for an array of numbers.
// Parameters:
//   numbers: A pointer to the first element of an array of integers.
//   count: The number of integers in the 'numbers' array.
// Returns:
//   The sum of all integers in the 'numbers' array.
//   Returns 0 if count is 0 (additive identity).
int k_add_n(const int* numbers, int count) {
    int sum = 0; // Initialize sum to 0 (additive identity)
    // If no numbers are provided, return 0.
    if (count <= 0 || numbers == 0) {
        return 0;
    }
    // Loop through the array and add each number to the sum.
    for (int i = 0; i < count; i++) {
        sum += numbers[i];
    }
    return sum;
}

// --- Function: k_subtract ---
// Performs integer subtraction (binary operation).
// Parameters:
//   a: The first integer operand (minuend).
//   b: The second integer operand (subtrahend).
// Returns:
//   The result of 'a' minus 'b'.
int k_subtract(int a, int b) {
    return a - b;
}

// --- Function: k_multiply_n ---
// Performs integer multiplication for an array of numbers.
// Parameters:
//   numbers: A pointer to the first element of an array of integers.
//   count: The number of integers in the 'numbers' array.
// Returns:
//   The product of all integers in the 'numbers' array.
//   Returns 1 if count is 0 (multiplicative identity).
int k_multiply_n(const int* numbers, int count) {
    int product = 1; // Initialize product to 1 (multiplicative identity)
    // If no numbers are provided, return 1.
    if (count <= 0 || numbers == 0) {
        return 1;
    }
    // Loop through the array and multiply each number into the product.
    for (int i = 0; i < count; i++) {
        product *= numbers[i];
    }
    return product;
}

// --- Function: k_divide ---
// Performs integer division (binary operation).
// Parameters:
//   numerator: The integer to be divided.
//   denominator: The integer to divide by.
// Returns:
//   The result of 'numerator' divided by 'denominator'.
//   Returns 0 if 'denominator' is 0 and prints a warning message to the console.
//   In a more robust OS, division by zero would typically trigger a CPU exception
//   that would be handled by an interrupt service routine. For this basic kernel,
//   we simply prevent a crash by checking and returning a default value.
int k_divide(int numerator, int denominator) {
    if (denominator == 0) {
        // Print an error message to the screen using kprint.
        // Using VGA_ATTRIB_RED_ON_BLACK for error messages.
        kprint("Error: Division by zero!\n", VGA_ATTRIB_RED_ON_BLACK); 
        return 0; // Return 0 as a safe default value for division by zero.
    }
    return numerator / denominator; // Perform the actual integer division.
}
