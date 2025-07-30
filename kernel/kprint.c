#include <stdint.h>
#include "kprint.h"   // Include our own header for kprint function declaration
#include "kinput.h"   // Required for 'outb' function declaration (for hardware cursor control)

// VGA text mode buffer address and dimensions
#define VGA_ADDRESS 0xb8000
#define VGA_WIDTH   80
#define VGA_HEIGHT  25

// Pointer to the VGA text buffer
static uint16_t* vga_buffer = (uint16_t*) VGA_ADDRESS;

// Global variables to keep track of the current software cursor position
static int cursor_x = 0;
static int cursor_y = 0;

// --- Internal Helper Function: update_hardware_cursor ---
// Moves the physical blinking cursor on the screen to the current cursor_x, cursor_y.
// This interacts directly with the VGA controller's I/O ports.
static void update_hardware_cursor() {
    uint16_t cursor_pos = cursor_y * VGA_WIDTH + cursor_x;

    // Send the high byte of the cursor position to VGA controller register 0x0E
    outb(0x3D4, 0x0E); // Command port: select Cursor Location High Register
    outb(0x3D5, (uint8_t)(cursor_pos >> 8)); // Data port: send high byte

    // Send the low byte of the cursor position to VGA controller register 0x0F
    outb(0x3D4, 0x0F); // Command port: select Cursor Location Low Register
    outb(0x3D5, (uint8_t)(cursor_pos & 0xFF)); // Data port: send low byte
}

// --- Internal Helper Function: scroll_screen ---
// Scrolls the entire screen content up by one line.
// The top line disappears, and a new blank line appears at the bottom.
static void scroll_screen() {
    // Copy each line from (n+1) to n, effectively moving everything up.
    // This loop starts from the second line (y=1) and copies it to the first line (y=0),
    // then the third to the second, and so on.
    for (int y = 1; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            // Copy the 16-bit character (char + attribute)
            vga_buffer[(y - 1) * VGA_WIDTH + x] = vga_buffer[y * VGA_WIDTH + x];
        }
    }
    // Clear the last line (now the old second-to-last line) with spaces.
    // Use the default VGA_ATTRIB_WHITE_ON_BLACK attribute for the cleared line.
    for (int x = 0; x < VGA_WIDTH; x++) {
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = (VGA_ATTRIB_WHITE_ON_BLACK << 8) | ' ';
    }
}

// --- Public Function: kprint ---
// Prints a null-terminated string to the VGA text buffer at the current cursor position.
// Handles cursor movement, newlines, carriage returns, backspace, and scrolling.
// Parameters:
//   str: A pointer to the constant character string to print.
//   color_attribute: The attribute byte (foreground and background color).
void kprint(const char* str, uint8_t color_attribute) {
    int i = 0; // Index for iterating through the input string
    while (str[i]) { // Loop until the null terminator is found
        char c = str[i]; // Get the current character

        if (c == '\n') { // Handle newline character
            cursor_x = 0; // Move cursor to the beginning of the current line
            cursor_y++;   // Move cursor to the next line
        } else if (c == '\r') { // Handle carriage return character
            cursor_x = 0; // Move cursor to the beginning of the current line (without changing row)
        } else if (c == '\b') { // Handle backspace character
            if (cursor_x > 0) { // If not at the beginning of a line
                cursor_x--; // Move cursor back one position
                // Clear the character at the new cursor position by writing a space
                vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = (VGA_ATTRIB_WHITE_ON_BLACK << 8) | ' ';
            } else if (cursor_y > 0) { // If at beginning of line, move to end of previous line
                cursor_y--; // Move up one line
                cursor_x = VGA_WIDTH - 1; // Move to the last column
                vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = (VGA_ATTRIB_WHITE_ON_BLACK << 8) | ' ';
            }
        } else { // Handle regular printable characters
            // Write the character and its provided color attribute to the VGA buffer
            vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = (color_attribute << 8) | c;
            cursor_x++; // Move cursor to the next character position
        }

        // Check if the cursor has gone past the right edge of the screen
        if (cursor_x >= VGA_WIDTH) {
            cursor_x = 0; // Reset to the beginning of the line
            cursor_y++;   // Move to the next line
        }

        // Check if the cursor has gone past the bottom edge of the screen
        if (cursor_y >= VGA_HEIGHT) {
            scroll_screen(); // Scroll the entire screen content up
            cursor_y = VGA_HEIGHT - 1; // Keep the cursor on the last line
        }
        
        // Update the hardware cursor's position to match our software cursor
        update_hardware_cursor();

        i++; // Move to the next character in the input string
    }
}

// --- Public Function: kclear_screen ---
// Clears the entire VGA text buffer by filling it with spaces and resets the cursor to top-left.
void kclear_screen() {
    // Loop through all character positions on the screen
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            // Write a space character with the default VGA_ATTRIB_WHITE_ON_BLACK color attribute
            vga_buffer[y * VGA_WIDTH + x] = (VGA_ATTRIB_WHITE_ON_BLACK << 8) | ' ';
        }
    }
    cursor_x = 0; // Reset software cursor X to 0
    cursor_y = 0; // Reset software cursor Y to 0
    update_hardware_cursor(); // Update hardware cursor to top-left (0,0)
}

// --- Public Function: kset_cursor_pos ---
// Sets the software cursor position and updates the hardware cursor.
// This allows direct control over where the next character will be printed.
// Parameters:
//   x: The target column (0 to VGA_WIDTH - 1).
//   y: The target row (0 to VGA_HEIGHT - 1).
void kset_cursor_pos(int x, int y) {
    // Ensure coordinates are within bounds
    if (x < 0) x = 0;
    if (x >= VGA_WIDTH) x = VGA_WIDTH - 1;
    if (y < 0) y = 0;
    if (y >= VGA_HEIGHT) y = VGA_HEIGHT - 1;

    cursor_x = x; // Update software cursor X
    cursor_y = y; // Update software cursor Y
    update_hardware_cursor(); // Update the physical cursor on screen
}

// --- Public Function: kprint_at ---
// Prints a null-terminated string at a specific X, Y coordinate with a given color.
// This function temporarily moves the cursor, prints, and then restores the cursor
// to the position *after* printing the string.
// Parameters:
//   str: The string to print.
//   x: The column to start printing at.
//   y: The row to start printing at.
//   color_attribute: The attribute byte (foreground and background color).
void kprint_at(const char* str, int x, int y, uint8_t color_attribute) {
    // Save the current cursor position before changing it
    int original_x = cursor_x;
    int original_y = cursor_y;

    kset_cursor_pos(x, y); // Move the cursor to the desired (x, y)
    kprint(str, color_attribute); // Print the string using the color-aware kprint

    // After printing, restore the cursor to its original position
    // This is important if you mix kprint_at with regular kprint calls
    // and want the subsequent kprint calls to continue from where they left off.
    kset_cursor_pos(original_x, original_y);
}
