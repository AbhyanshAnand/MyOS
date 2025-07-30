#ifndef KPRINT_H
#define KPRINT_H

#include <stdint.h> // For uint16_t, uint8_t

// VGA Dimensions

#define VGA_WIDTH   80
#define VGA_HEIGHT  25

// --- VGA Color Constants ---
// These are standard VGA text mode color codes.
// Each color is represented by a 4-bit value (0-15).
// The attribute byte is (Background << 4) | Foreground.
#define VGA_COLOR_BLACK         0x0
#define VGA_COLOR_BLUE          0x1
#define VGA_COLOR_GREEN         0x2
#define VGA_COLOR_CYAN          0x3
#define VGA_COLOR_RED           0x4
#define VGA_COLOR_MAGENTA       0x5
#define VGA_COLOR_BROWN         0x6
#define VGA_COLOR_LIGHT_GREY    0x7
#define VGA_COLOR_DARK_GREY     0x8
#define VGA_COLOR_LIGHT_BLUE    0x9
#define VGA_COLOR_LIGHT_GREEN   0xA
#define VGA_COLOR_LIGHT_CYAN    0xB
#define VGA_COLOR_LIGHT_RED     0xC
#define VGA_COLOR_LIGHT_MAGENTA 0xD
#define VGA_COLOR_YELLOW        0xE
#define VGA_COLOR_WHITE         0xF

// --- Common Color Combinations ---
#define VGA_ATTRIB_WHITE_ON_BLACK (VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4))
#define VGA_ATTRIB_BLACK_ON_WHITE (VGA_COLOR_BLACK | (VGA_COLOR_WHITE << 4))
#define VGA_ATTRIB_RED_ON_BLACK   (VGA_COLOR_RED   | (VGA_COLOR_BLACK << 4))
#define VGA_ATTRIB_GREEN_ON_BLACK (VGA_COLOR_GREEN | (VGA_COLOR_BLACK << 4))
#define VGA_ATTRIB_BLUE_ON_BLACK  (VGA_COLOR_BLUE  | (VGA_COLOR_BLACK << 4))
#define VGA_ATTRIB_YELLOW_ON_BLACK (VGA_COLOR_YELLOW | (VGA_COLOR_BLACK << 4))
#define VGA_ATTRIB_DARK_GREY_ON_BLACK (VGA_COLOR_DARK_GREY | (VGA_COLOR_BLACK << 4))
#define VGA_ATTRIB_LIGHT_BLUE_ON_BLACK (VGA_COLOR_LIGHT_BLUE | (VGA_COLOR_BLACK << 4))
#define VGA_ATTRIB_MAGENTA_ON_BLACK (VGA_COLOR_MAGENTA | (VGA_COLOR_BLACK << 4))
#define VGA_ATTRIB_LIGHT_CYAN_ON_BLACK (VGA_COLOR_LIGHT_CYAN | (VGA_COLOR_BLACK << 4))
// --- Function Declarations ---

// kprint: Prints a null-terminated string to the VGA text buffer at the current cursor position.
// Now accepts a color_attribute for the text.
// Parameters:
//   str: The string to print.
//   color_attribute: The attribute byte (foreground and background color).
void kprint(const char* str, uint8_t color_attribute);

// kclear_screen: Clears the entire VGA text buffer and resets cursor.
void kclear_screen();

// kset_cursor_pos: Sets the hardware cursor position.
// Parameters:
//   x: The column (0-79).
//   y: The row (0-24).
void kset_cursor_pos(int x, int y);

// kprint_at: Prints a null-terminated string at a specific X, Y coordinate.
// Now accepts a color_attribute for the text.
// Parameters:
//   str: The string to print.
//   x: The column to start printing at.
//   y: The row to start printing at.
//   color_attribute: The attribute byte (foreground and background color).
void kprint_at(const char* str, int x, int y, uint8_t color_attribute);

#endif
