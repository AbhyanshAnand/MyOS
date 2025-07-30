#include "kutils.h" // Include the header for our utility functions' declarations
#include "kprint.h" // For kprint if we want to print debug messages from here (optional)

// --- Function: k_strlen --- (No longer static)
// Calculates the length of a null-terminated string.
// Parameters:
//   s: A pointer to the constant character string.
// Returns:
//   The number of characters in the string, excluding the null terminator.
int k_strlen(const char* s) { // Removed 'static' keyword
    int len = 0; // Initialize a counter for the length
    // Loop through the string until the null terminator ('\0') is found
    while (s[len] != '\0') {
        len++; // Increment the counter for each character
    }
    return len; // Return the total count
}

// --- Helper Function: k_reverse ---
// Reverses a null-terminated string in place (modifies the original string).
// Parameters:
//   s: A pointer to the character array (string) to be reversed.
void k_reverse(char s[]) {
    int i, j;   // Loop indices: i for the start, j for the end
    char c;     // Temporary character for swapping

    // Initialize i to the beginning (0) and j to the end of the string (length - 1)
    // Loop as long as i is less than j (meaning we haven't reached the middle or crossed over)
    for (i = 0, j = k_strlen(s) - 1; i < j; i++, j--) {
        c = s[i];     // Store the character at the left index (i)
        s[i] = s[j];  // Move the character from the right index (j) to the left index (i)
        s[j] = c;     // Move the stored character (original s[i]) to the right index (j)
    }
}

// --- Function: k_atoi (ASCII to Integer) ---
// Converts a null-terminated string representation of a number to its integer value.
// Handles leading whitespace, optional sign ('+' or '-'), and digits.
// Stops conversion at the first non-digit character.
// Parameters:
//   str: A pointer to the constant character string to convert.
// Returns:
//   The integer value represented by the string.
int k_atoi(const char* str) {
    int res = 0;    // Variable to accumulate the resulting integer value
    int sign = 1;   // Variable to store the sign of the number (1 for positive, -1 for negative)
    int i = 0;      // Index to iterate through the input string

    // 1. Skip leading whitespace characters
    // Loop as long as the current character is a space, tab, newline, or carriage return
    while (str[i] == ' ' || str[i] == '\t' || str[i] == '\n' || str[i] == '\r') {
        i++; // Move to the next character
    }

    // 2. Handle optional sign ('-' or '+')
    if (str[i] == '-') {
        sign = -1; // If it's a minus sign, set sign to negative
        i++;       // Move past the sign character
    } else if (str[i] == '+') {
        i++;       // If it's a plus sign, just move past it (positive is default)
    }

    // 3. Convert digits to integer
    // Loop as long as the current character is a digit ('0' through '9')
    while (str[i] >= '0' && str[i] <= '9') {
        // Core conversion logic:
        // Multiply the current result by 10 (shifts digits left, e.g., 12 becomes 120)
        // Add the integer value of the current digit (e.g., '5' - '0' = 5).
        res = res * 10 + (str[i] - '0');
        i++; // Move to the next character
    }

    // 4. Apply the determined sign to the final result
    return sign * res;
}

// --- Function: k_itoa (Integer to ASCII) ---
// Converts an integer 'value' to a null-terminated string 's' in a given 'base'.
// Parameters:
//   value: The integer number to convert.
//   s: A pointer to a character array (buffer) where the resulting string will be stored.
//      This buffer must be large enough to hold the converted number.
//   base: The base for conversion (e.g., 10 for decimal, 2 for binary, 16 for hexadecimal).
//         Supported bases are from 2 to 36 (digits 0-9, then 'a'-'z' for 10-35).
// Returns:
//   A pointer to the converted string 's'.
char* k_itoa(int value, char* s, int base) {
    int i = 0;          // Index for building the string 's' (starts at 0)
    int is_negative = 0; // Flag to track if the original number was negative

    // 1. Validate the base
    if (base < 2 || base > 36) {
        s[0] = '\0'; // If base is invalid, return an empty string
        return s;
    }

    // 2. Handle the special case for value 0
    if (value == 0) {
        s[i++] = '0'; // Put '0' into the string
        s[i] = '\0';  // Null-terminate the string
        return s;     // Return the string
    }

    // 3. Handle negative numbers
    // If the value is negative AND the base is 10 (decimal),
    // set the negative flag and convert the value to positive for easier digit extraction.
    if (value < 0 && base == 10) {
        is_negative = 1;
        value = -value; // Make value positive
    } else if (value < 0) {
        // For non-base 10, we'll just convert the absolute value for simplicity.
        // A more complex itoa might handle two's complement for other bases.
        value = -value; // Convert to positive for conversion
    }

    // 4. Convert digits and store them in reverse order
    // Loop as long as the value is greater than 0
    while (value > 0) {
        int rem = value % base; // Get the remainder (this is the next digit from right to left)
        
        // Convert the remainder (digit) to its ASCII character representation:
        // If remainder is > 9 (e.g., 10, 11 for hex), use 'a' for 10, 'b' for 11, etc.
        // Otherwise (0-9), convert it by adding '0'.
        s[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        
        value = value / base; // Divide the value by the base to process the next digit
    }

    // 5. Append the negative sign if the original number was negative
    if (is_negative) {
        s[i++] = '-'; // Add the '-' character at the end (it will be reversed later)
    }

    s[i] = '\0'; // Null-terminate the string (important for C strings)

    // 6. Reverse the string
    // Since digits were generated in reverse order (e.g., 123 -> 3, 2, 1),
    // we need to reverse the string to get the correct order ("123").
    k_reverse(s);

    return s; // Return a pointer to the now correctly formatted string
}
