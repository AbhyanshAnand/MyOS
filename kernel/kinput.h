#ifndef KINPUT_H
#define KINPUT_H

// Declares an external assembly function to read a byte from an I/O port.
// This function will be defined in boot.asm.
extern uint8_t inb(uint16_t port);

// Declares an external assembly function to write a byte to an I/O port.
// This function will be defined in boot.asm.
extern void outb(uint16_t port, uint8_t data);

// Function to get a single character from the keyboard.
// It polls the keyboard controller until a key press is detected.
char kgetc();

// Function to read a string from the keyboard.
// It reads characters until Enter is pressed or max_len is reached.
// It also handles backspace and echoes characters to the screen.
void kgets(char* buffer, int max_len);

#endif // KINPUT_H
