#ifndef KUTILS_H // Standard header guard to prevent multiple inclusions
#define KUTILS_H

#include <stdint.h> // Includes standard integer types like int, uint8_t, etc.

// --- Function Declarations ---

// k_strlen: Calculates the length of a null-terminated string.
// Parameters:
//   s: A pointer to the constant character string.
// Returns:
//   The number of characters in the string, excluding the null terminator.
int k_strlen(const char* s); // Now declared here to be public

// k_atoi: Converts a null-terminated string (ASCII) to an integer.
// Parameters:
//   str: A pointer to the constant character string to convert.
// Returns:
//   The integer value represented by the string.
// Example: "123" -> 123, "-45" -> -45
int k_atoi(const char* str);

// k_itoa: Converts an integer 'value' to a null-terminated string 's' in a given 'base'.
// Parameters:
//   value: The integer to convert.
//   s: A pointer to a character array (buffer) where the resulting string will be stored.
//      This array must be large enough to hold the converted string, including the null terminator.
//   base: The numerical base for the conversion (e.g., 10 for decimal, 2 for binary, 16 for hexadecimal).
//         Supports bases from 2 to 36 (0-9, a-z).
// Returns:
//   A pointer to the converted string 's'.
// Example: k_itoa(123, buffer, 10) -> "123"
// Example: k_itoa(10, buffer, 2) -> "1010"
char* k_itoa(int value, char* s, int base);

// k_reverse: Reverses a null-terminated string in place.
// This is a helper function primarily used internally by k_itoa, as k_itoa generates digits in reverse order.
// Parameters:
//   s: A pointer to the character array (string) to be reversed.
void k_reverse(char s[]);

#endif // KUTILS_H
