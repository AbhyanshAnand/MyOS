#include <stdint.h>   // For standard integer types like uint8_t, uint16_t
#include "kinput.h"   // Include our own header for kgetc and outb declarations
#include "kprint.h"   // Required for kprint to echo characters back to the screen (now with color support)

// --- PS/2 Keyboard Controller I/O Ports ---
// These are standard I/O port addresses for the PS/2 keyboard controller.
#define KBD_DATA_PORT   0x60 // Data port: used to read scan codes from the keyboard.
#define KBD_STATUS_PORT 0x64 // Status/Command port: used to check keyboard status or send commands.

// --- Scan Code to ASCII Mapping ---
// This is a highly simplified lookup table to convert raw keyboard scan codes
// (which are hardware-specific codes sent by the keyboard) into ASCII characters.
// This table is for a US QWERTY keyboard, non-shifted, and covers only basic keys.
// A full keyboard driver would be much more complex, handling:
// - Shift states (uppercase letters, symbols like !, @, #)
// - Caps Lock, Num Lock, Scroll Lock
// - Special keys (F-keys, arrow keys, Home, End, Page Up/Down, Insert, Delete)
// - Different keyboard layouts (e.g., QWERTZ, AZERTY)
// - Key release events (scan code with MSB set, e.g., 0x80 + key_scan_code)
static const unsigned char kbd_us[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', /* 0x0E: Backspace */
    '\t', /* 0x0F: Tab */
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',     /* 0x1C: Enter key */
    0, /* 0x1D: Left Control */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0, /* 0x2A: Left Shift */
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0, /* 0x36: Right Shift */
    '*', /* 0x37: Keypad * */
    0,  /* 0x38: Left Alt */
    ' ',  /* 0x39: Space bar */
    0,  /* 0x3A: Caps lock */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x3B-0x44: F1-F10 */
    0,  /* 0x45: Num lock*/
    0,  /* 0x46: Scroll Lock */
    0,  /* 0x47: Home key */
    0,  /* 0x48: Up Arrow */
    0,  /* 0x49: Page Up */
    '-', /* 0x4A: Keypad - */
    0,  /* 0x4B: Left Arrow */
    0,  /* 0x4C: Keypad 5 */
    0,  /* 0x4D: Right Arrow */
    '+', /* 0x4E: Keypad + */
    0,  /* 0x4F: End key */
    0,  /* 0x50: Down Arrow */
    0,  /* 0x51: Page Down */
    0,  /* 0x52: Insert Key */
    0,  /* 0x53: Delete Key */
    0, 0, 0,
    0,  /* 0x57: F11 Key */
    0,  /* 0x58: F12 Key */
    0,  /* All other keys are undefined or special */
};

// --- Public Function: kgetc ---
// Reads a single character from the keyboard. This function uses polling,
// meaning it continuously checks the keyboard status until a key press is detected.
// Parameters: None.
// Returns:
//   The ASCII character corresponding to the pressed key.
//   Returns 0 if the scan code is not mapped in the kbd_us table.
char kgetc() {
    uint8_t status;      // Variable to store the keyboard status register value
    uint8_t scan_code;   // Variable to store the raw scan code from the keyboard

    // Loop indefinitely until data is available in the keyboard buffer.
    while (1) {
        // 1. Read the status register of the keyboard controller.
        status = inb(KBD_STATUS_PORT);

        // 2. Check if the output buffer (bit 0 of the status register) is full.
        //    If bit 0 is set (0x01), it means there's data ready to be read from the data port.
        if (status & 0x01) {
            // 3. Read the scan code from the keyboard data port.
            scan_code = inb(KBD_DATA_PORT);

            // 4. Check for key release events.
            //    Key release scan codes have the most significant bit (MSB, 0x80) set.
            //    We only care about key press events, so we check if MSB is NOT set.
            if (!(scan_code & 0x80)) {
                // It's a key press. Convert the scan code to an ASCII character
                // using our lookup table and return it.
                // Ensure the scan_code is within the bounds of our kbd_us array.
                if (scan_code < 128) {
                    return kbd_us[scan_code];
                }
            }
        }
    }
}

// --- Public Function: kgets ---
// Reads a string from the keyboard into a provided buffer.
// It reads characters one by one using kgetc, echoes them to the screen,
// and handles backspace and the Enter key.
// Parameters:
//   buffer: A pointer to a character array where the input string will be stored.
//   max_len: The maximum number of characters to read (including space for null terminator).
void kgets(char* buffer, int max_len) {
    int i = 0;   // Index for the buffer
    char c;      // Variable to store the character read from keyboard

    // Loop until the buffer is almost full (leaving space for the null terminator)
    // or the Enter key is pressed.
    while (i < max_len - 1) {
        c = kgetc(); // Get a single character from the keyboard

        // Handle Enter key (newline or carriage return)
        if (c == '\n' || c == '\r') {
            break; // Exit the loop if Enter is pressed
        }

        // Handle Backspace key
        if (c == '\b') {
            if (i > 0) { // Only backspace if there are characters already in the buffer
                i--; // Decrement buffer index
                // Erase character from screen:
                // Move cursor back, print a space to overwrite the character, then move cursor back again.
                // Now passing the default color attribute.
                kprint("\b \b", VGA_ATTRIB_WHITE_ON_BLACK); 
            }
            continue; // Don't add backspace to the buffer; continue to the next key press
        }

        // Only process valid characters (non-zero, as 0 indicates unmapped scan codes in kbd_us)
        if (c != 0) {
            buffer[i++] = c; // Store the character in the buffer and then increment the index
            
            // Echo character to screen:
            // Create a temporary string with the single character and print it
            // so the user can see what they are typing.
            // Now passing the default color attribute.
            char temp_str[2];
            temp_str[0] = c;
            temp_str[1] = '\0'; // Null-terminate the temporary string
            kprint(temp_str, VGA_ATTRIB_WHITE_ON_BLACK);
        }
    }
    buffer[i] = '\0'; // Null-terminate the string after input is complete
}
