#include <stdint.h>     // Standard integer types (e.g., int, uint8_t)
#include "kprint.h"     // Our custom printing functions (kprint, kclear_screen, kset_cursor_pos, kprint_at)
#include "kinput.h"     // Our custom keyboard input functions (kgets, kgetc)
#include "kutils.h"     // Our new utility functions (k_atoi, k_itoa, k_strlen)
#include "kmath.h"      // Our new math functions (k_add_n, k_subtract, k_multiply_n, k_divide)

// --- Menu Option Definitions ---
// Define the menu options as an array of constant strings.
const char* menu_options[] = {
    "1. Do Math",
    "2. About MyOS",
    "3. Reboot",
    "4. Shutdown",
    "5. Calculator" // New calculator option
};
// Calculate the number of options in the menu dynamically.
#define NUM_MENU_OPTIONS (sizeof(menu_options) / sizeof(menu_options[0]))

// --- Menu State Variables ---
static int selected_option = 0; // Index of the currently highlighted option (0-based)
static const int MENU_START_Y = 5; // Y-coordinate (row) where the menu will start printing

// --- Calculator State Variables ---
// These are global so they persist across calls to calculator functions
static int calculator_cursor_X = 0; // X-position of the highlighted button in the calculator grid
static int calculator_cursor_Y = 0; // Y-position of the highlighted button in the calculator grid

// Buffers for calculator display and input
static char calculator_display_buffer[VGA_WIDTH + 1]; // Main display, can show current number or result
static char calculator_input_buffer[32];              // Stores digits being typed for current number
static int calculator_input_buffer_idx = 0;           // Current index in calculator_input_buffer

// Calculator logic variables
static int calculator_operand1 = 0;       // First operand in a calculation
static char calculator_operator = '\0';   // Stored operator (+, -, *, /)
static int calculator_expecting_operand2 = 0; // Flag: 1 if we're expecting the second number, 0 otherwise
static int calculator_just_calculated = 0; // Flag: 1 if '=' was just pressed, clears display on next digit

// --- Helper Function: k_strcpy ---
// Simple string copy for internal use. Moved here to be defined before use.
static void k_strcpy(char* dest, const char* src) {
    int i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

// --- Helper Function: k_strcat ---
// Simple string concatenation for internal use. Moved here to be defined before use.
static void k_strcat(char* dest, const char* src) {
    int dest_len = k_strlen(dest); // k_strlen is now public from kutils.h
    int i = 0;
    while (src[i] != '\0') {
        dest[dest_len + i] = src[i];
        i++;
    }
    dest[dest_len + i] = '\0';
}

// --- Function Prototypes for Menu Actions ---
// These functions perform the actions associated with each menu item.
void do_math_action();
void about_myos_action();
void reboot_action();
void shutdown_action();
void run_calculator(); // Renamed from 'calculator' to 'run_calculator' for clarity

// --- Helper Function: delay ---
// Creates a simple busy-wait delay. Not accurate in real-time, but works for basic pauses.
// Parameters:
//   iterations: The number of times to loop. Higher value means longer delay.
void delay(uint32_t iterations) {
    for (uint32_t i = 0; i < iterations; i++) {
        __asm__ volatile ("nop"); // No operation - prevents compiler from optimizing out the loop entirely
    }
}

// --- Function: draw_menu ---
// Clears the menu area and redraws all menu options, highlighting the selected one.
void draw_menu() {
    // Clear the area where the menu will be displayed to remove old highlights.
    // We print 80 spaces (VGA_WIDTH) on each line with the default color.
    for (int i = 0; i < (int)NUM_MENU_OPTIONS + 2; i++) { // Cast NUM_MENU_OPTIONS to int
        kprint_at("                                                                                ", 0, MENU_START_Y + i, VGA_ATTRIB_WHITE_ON_BLACK);
    }

    // Print the menu title, centered and in yellow.
    kprint_at("--- Main Menu ---", (VGA_WIDTH - k_strlen("--- Main Menu ---")) / 2, MENU_START_Y - 2, VGA_ATTRIB_YELLOW_ON_BLACK);

    // Loop through each menu option to print it.
    for (int i = 0; i < (int)NUM_MENU_OPTIONS; i++) { // Cast NUM_MENU_OPTIONS to int
        int current_y = MENU_START_Y + i; // Calculate the Y position for this option.
        
        uint8_t color_attribute; // Variable to hold the color for the current option.
        if (i == selected_option) {
            // If this option is selected, use black text on a white background for highlighting.
            color_attribute = VGA_ATTRIB_BLACK_ON_WHITE; 
        } else {
            // Otherwise, use white text on a black background (default).
            color_attribute = VGA_ATTRIB_WHITE_ON_BLACK;
        }

        const char* option_str = menu_options[i]; // Get the string for the current option.
        
        // Calculate X position to center the option on the screen.
        int start_x = (VGA_WIDTH - k_strlen(option_str)) / 2;

        // Print the option string at the calculated position with the determined color.
        kprint_at(option_str, start_x, current_y, color_attribute);
    }
}

// --- Function: handle_menu_input ---
// Manages menu navigation based on user key presses ('w' for up, 's' for down, Enter to select).
// Returns:
//   The index of the selected option if Enter is pressed, otherwise -1.
int handle_menu_input() {
    char key = kgetc(); // Get a single character from the keyboard.

    if (key == 'w' || key == 'W') { // If 'w' or 'W' (Up arrow simulation)
        selected_option--; // Move selection up.
        if (selected_option < 0) {
            selected_option = (int)NUM_MENU_OPTIONS - 1; // Cast NUM_MENU_OPTIONS to int for wrap-around
        }
        draw_menu(); // Redraw the menu with the new highlight.
    } else if (key == 's' || key == 'S') { // If 's' or 'S' (Down arrow simulation)
        selected_option++; // Move selection down.
        if (selected_option >= (int)NUM_MENU_OPTIONS) { // Cast NUM_MENU_OPTIONS to int for comparison
            selected_option = 0; // Wrap around to the top if at the bottom.
        }
        draw_menu(); // Redraw the menu with the new highlight.
    } else if (key == '\n') { // If Enter key is pressed
        return selected_option; // Return the index of the currently selected option.
    }
    return -1; // Return -1 if no option was selected (i.e., just navigation keys were pressed).
}

// --- Calculator UI Layout ---
// Defines the text labels for each button on the calculator grid.
const char* calculator_layout[5][4] = {
    { "7", "8", "9", "/" },
    { "4", "5", "6", "*" },
    { "1", "2", "3", "-" },
    { "0", ".", "=", "+" },
    { "C", "Q", "", "" }  // C for Clear, Q for Quit (exit calculator)
};
// Dimensions of the calculator grid
#define CALC_GRID_ROWS 5
#define CALC_GRID_COLS 4
// Starting position for drawing the calculator grid on screen
#define CALC_START_X 20
#define CALC_START_Y 5
// Position for the calculator display area
#define CALC_DISPLAY_X 15
#define CALC_DISPLAY_Y 3

// --- Function: draw_calculator ---
// Draws the calculator interface, including buttons and the display.
// Parameters:
//   highlight_x: X-coordinate of the currently highlighted button.
//   highlight_y: Y-coordinate of the currently highlighted button.
void draw_calculator(int highlight_x, int highlight_y) {
    kclear_screen(); // Clear the screen for the calculator

    // Draw the display area
    kprint_at("-----------------------------------", CALC_DISPLAY_X, CALC_DISPLAY_Y - 1, VGA_ATTRIB_WHITE_ON_BLACK);
    kprint_at("|                                 |", CALC_DISPLAY_X, CALC_DISPLAY_Y, VGA_ATTRIB_WHITE_ON_BLACK);
    kprint_at("-----------------------------------", CALC_DISPLAY_X, CALC_DISPLAY_Y + 1, VGA_ATTRIB_WHITE_ON_BLACK);
    
    // Print the current content of the display buffer
    kprint_at(calculator_display_buffer, CALC_DISPLAY_X + 2, CALC_DISPLAY_Y, VGA_ATTRIB_YELLOW_ON_BLACK);

    // Draw the calculator buttons
    for (int y = 0; y < CALC_GRID_ROWS; y++) {
        for (int x = 0; x < CALC_GRID_COLS; x++) {
            const char* label = calculator_layout[y][x];

            // Skip drawing if there's no label (e.g., empty slots in the layout)
            if (label[0] == '\0') continue;

            // Determine button color: highlighted or normal
            uint8_t color = (x == highlight_x && y == highlight_y)
                ? VGA_ATTRIB_BLACK_ON_WHITE // Highlighted color
                : VGA_ATTRIB_WHITE_ON_BLACK; // Normal color

            // Create a padded string for the button label to ensure uniform width
            char padded[5]; // Max 3 chars for label + 1 space + null terminator
            k_strcpy(padded, "   "); // Fill with spaces
            padded[k_strlen(label)] = '\0'; // Null-terminate after label length
            k_strcpy(padded, label); // Copy label over spaces (e.g., " 7 " or " + ")

            // Draw button centered in 3 spaces (each button is 3 chars wide + 1 space between)
            kprint_at(padded, CALC_START_X + x * 4, CALC_START_Y + y, color);
        }
    }
}


// --- Function: calculate_result ---
// Performs the calculation based on stored operands and operator.
// Updates calculator_operand1 with the result.
void calculate_result() {
    if (calculator_operator == '\0' || calculator_input_buffer_idx == 0) {
        return; // Nothing to calculate yet
    }

    int operand2 = k_atoi(calculator_input_buffer);
    int result = 0;

    switch (calculator_operator) {
        case '+': result = k_add_n((const int[]){calculator_operand1, operand2}, 2); break;
        case '-': result = k_subtract(calculator_operand1, operand2); break;
        case '*': result = k_multiply_n((const int[]){calculator_operand1, operand2}, 2); break;
        case '/': result = k_divide(calculator_operand1, operand2); break;
    }

    calculator_operand1 = result; // Store result as the new first operand
    k_itoa(result, calculator_display_buffer, 10); // Update display with result
    calculator_input_buffer_idx = 0; // Clear current input buffer
    calculator_input_buffer[0] = '\0';
    calculator_operator = '\0'; // Clear operator
    calculator_expecting_operand2 = 0; // Reset flag
    calculator_just_calculated = 1; // Mark that a calculation just happened
}

// --- Function: run_calculator ---
// Main loop for the calculator application.
void run_calculator() {
    kclear_screen(); // Clear screen initially for calculator

    // Initialize calculator state
    k_strcpy(calculator_display_buffer, "0"); // Default display
    calculator_input_buffer[0] = '\0';
    calculator_input_buffer_idx = 0;
    calculator_operand1 = 0;
    calculator_operator = '\0';
    calculator_expecting_operand2 = 0;
    calculator_just_calculated = 0;
    calculator_cursor_X = 0; // Reset cursor position for calculator grid
    calculator_cursor_Y = 0;

    draw_calculator(calculator_cursor_X, calculator_cursor_Y); // Draw initial calculator UI

    while (1) {
        char key = kgetc(); // Get key input

        // --- Navigation ---
        if (key == 'w' || key == 'W') { // Up
            if (calculator_cursor_Y > 0) calculator_cursor_Y--;
        } else if (key == 's' || key == 'S') { // Down
            if (calculator_cursor_Y < CALC_GRID_ROWS - 1) calculator_cursor_Y++;
        } else if (key == 'a' || key == 'A') { // Left
            if (calculator_cursor_X > 0) calculator_cursor_X--;
        } else if (key == 'd' || key == 'D') { // Right
            if (calculator_cursor_X < CALC_GRID_COLS - 1) calculator_cursor_X++;
        } 
        // --- Action (Enter Key) ---
        else if (key == '\n') { // Enter key pressed
            const char* button_label = calculator_layout[calculator_cursor_Y][calculator_cursor_X];

            if (button_label[0] == '\0') { // Handle empty button slots
                // Do nothing for empty slots
            } else if (button_label[0] >= '0' && button_label[0] <= '9') { // Digit button
                if (calculator_just_calculated || (calculator_expecting_operand2 && calculator_input_buffer_idx == 0)) {
                    // If just calculated or expecting new operand, clear display/input
                    calculator_input_buffer_idx = 0;
                    calculator_input_buffer[0] = '\0';
                    k_strcpy(calculator_display_buffer, "0"); // Reset display
                    calculator_just_calculated = 0;
                }
                if (calculator_input_buffer_idx < (int)sizeof(calculator_input_buffer) - 1) { // Cast sizeof to int
                    calculator_input_buffer[calculator_input_buffer_idx++] = button_label[0];
                    calculator_input_buffer[calculator_input_buffer_idx] = '\0';
                    k_strcpy(calculator_display_buffer, calculator_input_buffer);
                }
            } else if (button_label[0] == '.') { // Decimal point (not fully supported for int math, but can be added)
                // For now, we only support integer math.
                // You would need to implement floating-point support for this.
            } else if (button_label[0] == 'C') { // Clear button
                calculator_input_buffer_idx = 0;
                calculator_input_buffer[0] = '\0';
                k_strcpy(calculator_display_buffer, "0");
                calculator_operand1 = 0;
                calculator_operator = '\0';
                calculator_expecting_operand2 = 0;
                calculator_just_calculated = 0;
            } else if (button_label[0] == 'Q') { // Quit button
                return; // Exit the calculator loop, return to main menu
            } else if (button_label[0] == '=') { // Equals button
                calculate_result();
            } else { // Operator button (+, -, *, /)
                if (calculator_input_buffer_idx > 0) { // If a number has been entered
                    if (calculator_operator != '\0') { // If there's a pending operation, calculate it first
                        calculate_result();
                    }
                    calculator_operand1 = k_atoi(calculator_input_buffer);
                } else if (calculator_just_calculated) {
                    // If we just calculated, the result is already in calculator_operand1
                    calculator_just_calculated = 0;
                }
                
                calculator_operator = button_label[0];
                calculator_expecting_operand2 = 1;
                calculator_input_buffer_idx = 0; // Clear input buffer for next operand
                calculator_input_buffer[0] = '\0';
                k_strcpy(calculator_display_buffer, button_label); // Show operator on display temporarily
            }
        }
        // Redraw calculator UI with updated position and display
        draw_calculator(calculator_cursor_X, calculator_cursor_Y);
    }
}

// --- Menu Action Function: do_math_action ---
// Handles the "Do Math" menu option. Prompts for two numbers, performs basic math, and displays results.
void do_math_action() {
    kclear_screen(); // Clear the screen for the math application.
    char input_buffer[32]; // Buffer for string input from user.
    int num1, num2;         // Variables to store the two integer inputs.
    char result_str[32];    // Buffer to store string representation of math results.
    int sum, difference, product, quotient; // Variables for math results.

    kprint("--- Do Math ---\n", VGA_ATTRIB_YELLOW_ON_BLACK); // Title for the math section.
    
    kprint("Enter first number: ", VGA_ATTRIB_WHITE_ON_BLACK);
    kgets(input_buffer, sizeof(input_buffer)); // Get string input.
    num1 = k_atoi(input_buffer);               // Convert string to integer.

    kprint("Enter second number: ", VGA_ATTRIB_WHITE_ON_BLACK);
    kgets(input_buffer, sizeof(input_buffer)); // Get string input.
    num2 = k_atoi(input_buffer);               // Convert string to integer.

    // Perform math operations using kmath functions.
    // Note: k_add_n and k_multiply_n take an array.
    sum = k_add_n((const int[]){num1, num2}, 2); // Example: add 2 numbers
    difference = k_subtract(num1, num2);
    product = k_multiply_n((const int[]){num1, num2}, 2); // Example: multiply 2 numbers
    quotient = k_divide(num1, num2); // k_divide handles division by zero internally.

    // Print results with different colors for clarity.
    kprint("Sum: ", VGA_ATTRIB_LIGHT_BLUE_ON_BLACK);
    k_itoa(sum, result_str, 10); // Convert sum to string.
    kprint(result_str, VGA_ATTRIB_WHITE_ON_BLACK); // Print sum.
    kprint("\n", VGA_ATTRIB_WHITE_ON_BLACK);

    kprint("Difference: ", VGA_ATTRIB_LIGHT_BLUE_ON_BLACK);
    k_itoa(difference, result_str, 10); // Convert difference to string.
    kprint(result_str, VGA_ATTRIB_WHITE_ON_BLACK); // Print difference.
    kprint("\n", VGA_ATTRIB_WHITE_ON_BLACK);

    kprint("Product: ", VGA_ATTRIB_LIGHT_BLUE_ON_BLACK);
    k_itoa(product, result_str, 10); // Convert product to string.
    kprint(result_str, VGA_ATTRIB_WHITE_ON_BLACK); // Print product.
    kprint("\n", VGA_ATTRIB_WHITE_ON_BLACK);

    kprint("Quotient: ", VGA_ATTRIB_LIGHT_BLUE_ON_BLACK);
    k_itoa(quotient, result_str, 10); // Convert quotient to string.
    kprint(result_str, VGA_ATTRIB_WHITE_ON_BLACK); // Print quotient.
    kprint("\n\n", VGA_ATTRIB_WHITE_ON_BLACK);

    kprint("Press any key to return to menu...\n", VGA_ATTRIB_DARK_GREY_ON_BLACK);
    kgetc(); // Wait for any key press before returning to the menu.
}

// --- Menu Action Function: about_myos_action ---
// Displays information about the OS.
void about_myos_action() {
    kclear_screen(); // Clear the screen.
    kprint("--- About MyOS ---\n", VGA_ATTRIB_YELLOW_ON_BLACK);
    kprint("MyOS is a simple 64-bit kernel built from scratch using assembly for boot and C for Kernel.\n", VGA_ATTRIB_WHITE_ON_BLACK);
    kprint("It offers basic VGA type display text output and keyboard input.\n", VGA_ATTRIB_WHITE_ON_BLACK);
    kprint("Developed by me as a learning project for OS development.\n", VGA_ATTRIB_WHITE_ON_BLACK);
    kprint("\nPress any key to return to menu...\n", VGA_ATTRIB_DARK_GREY_ON_BLACK);
    kgetc(); // Wait for a key press.
}

// --- Menu Action Function: reboot_action ---
// Attempts to reboot the system using the keyboard controller.
void reboot_action() {
    kclear_screen(); // Clear the screen.
    kprint("Rebooting system...\n", VGA_ATTRIB_RED_ON_BLACK);
    // This is a common way to trigger a system reset using the PS/2 keyboard controller (8042).
    // Sending 0xFE to port 0x64 (command port) often initiates a CPU reset.
    outb(0x64, 0xFE); 
    // If the reboot command doesn't work, enter an infinite halt loop.
    while(1) { __asm__ volatile("hlt"); }
}

// --- Menu Action Function: shutdown_action ---
// Halts the CPU, simulating a shutdown.
void shutdown_action() {
    kclear_screen(); // Clear the screen.
    kprint("Shutting down system...\n", VGA_ATTRIB_RED_ON_BLACK);
    // In a real OS, this would involve sending ACPI commands for a graceful shutdown.
    // For a simple kernel, halting the CPU indefinitely is the closest equivalent.
    while(1) { __asm__ volatile("hlt"); }
}

// --- Main Kernel Entry Point ---
// This is the first C function executed after the assembly bootstrap.
void kernel_main(void) {
    kclear_screen(); // Clear the screen to ensure a clean start.

    // --- Initial Welcome and Name Input ---
    kprint("Welcome to MyOS!\n", VGA_ATTRIB_LIGHT_CYAN_ON_BLACK);
    
    char name[256]; // Buffer for user's name.
    kprint("Please enter your name: ", VGA_ATTRIB_WHITE_ON_BLACK);
    kgets(name, sizeof(name)); 

    kprint("\nHello, ", VGA_ATTRIB_GREEN_ON_BLACK);
    kprint(name, VGA_ATTRIB_YELLOW_ON_BLACK); // Print name in yellow.
    kprint("!\n", VGA_ATTRIB_GREEN_ON_BLACK);

    // --- "Do you want to do math?" Prompt ---
    char math_choice_str[10]; // Buffer for user's yes/no input.
    kprint("\nDo you want to do math? (yes/no): ", VGA_ATTRIB_MAGENTA_ON_BLACK);
    kgets(math_choice_str, sizeof(math_choice_str));

    // Simple check for "yes" or "y". Case-insensitive for 'y'/'Y'.
    // A more robust check would compare the full string "yes" or use string comparison functions.
    if (math_choice_str[0] == 'y' || math_choice_str[0] == 'Y') {
        // --- Main Menu Loop ---
        int chosen_option = -1; // Variable to store the index of the selected menu item.
        while (1) {
            draw_menu(); // Draw the menu with the current selection highlighted.
            chosen_option = handle_menu_input(); // Get user input and handle navigation.

            // If an option was selected (Enter key pressed).
            if (chosen_option != -1) {
                // Use a switch statement to perform actions based on the selected option.
                switch (chosen_option) {
                    case 0: // "1. Do Math"
                        do_math_action();
                        break;
                    case 1: // "2. About MyOS"
                        about_myos_action();
                        break;
                    case 2: // "3. Reboot"
                        reboot_action();
                        break;
                    case 3: // "4. Shutdown"
                        shutdown_action();
                        break;
                    case 4: // "5. Calculator"
                        run_calculator(); // Call the new calculator function
                        break;
                    default:
                        kprint("Invalid option selected!\n", VGA_ATTRIB_RED_ON_BLACK);
                        break;
                }
                // After an action, clear the screen and prepare to return to the menu loop.
                kclear_screen();
                kprint("Returning to main menu...\n\n", VGA_ATTRIB_DARK_GREY_ON_BLACK);
                selected_option = 0; // Reset selection to the first option when returning.
            }
        }
    } else {
        // --- User chose "no" to math ---
        kclear_screen();
        kprint("Ok then, time ends...\n", VGA_ATTRIB_RED_ON_BLACK);
        
        // Simple 3-second delay (approximate)
        delay(30000000); // Adjust this value for longer/shorter delay

        kprint("CPU halting.\n", VGA_ATTRIB_RED_ON_BLACK);
        // Halt the CPU indefinitely.
        while (1) {
            __asm__ volatile ("hlt"); 
        }
    }
}
