#ifndef PROJECT_CONFIG_H
#define PROJECT_CONFIG_H

// =============================================================================
// User Adjustable Configuration
// =============================================================================

// Main text block.
// Do not include "\n" or "\r" here. Enter/newline is handled separately.
#define HID_TEXT_TO_TYPE "1234567890 Marcel Levinspuhl"

// Enter control.
#define SEND_ENTER_AFTER_TEXT true

// Enter send modes.
#define ENTER_MODE_PRINT_NEWLINE            0
#define ENTER_MODE_KEY_RETURN_RELEASE       1
#define ENTER_MODE_KEY_RETURN_RELEASE_ALL   2

// Default engineering test mode.
// If compilation fails because the library does not implement releaseAll(), change to:
//   ENTER_MODE_KEY_RETURN_RELEASE
// If Enter key mode is unreliable, change to:
//   ENTER_MODE_PRINT_NEWLINE
#define ENTER_SEND_MODE ENTER_MODE_KEY_RETURN_RELEASE_ALL

// Used only when ENTER_SEND_MODE == ENTER_MODE_PRINT_NEWLINE.
#define HID_NEWLINE_TEXT "\n"

// Guard before Enter/newline.
// This does not slow the main text block. It only protects the final Enter.
#define ENTER_PRE_GUARD_MS 100

// Used only by HID key mode.
#define ENTER_KEY_HOLD_MS 20

// Optional guard after Enter/newline.
#define ENTER_POST_GUARD_MS 50

// Press qualification after GPIO interrupt.
// Rejects startup glitches and contact bounce.
#define PRESS_CONFIRM_MS 30

// BOOT must be released and stable at startup before trigger is enabled.
#define STARTUP_ARM_STABLE_MS 1000

// BOOT must be released and stable after activation before rearming.
#define RELEASE_STABLE_TIME_MS 50

// Red LED pulse time when BLE HID is not connected.
#define DISCONNECTED_LED_PULSE_MS 50

// =============================================================================
// Hardware Configuration
// =============================================================================

#define BOOT_BUTTON_PIN 0

#define RGB_LED_PIN 48
#define NUM_LEDS 1

#define RGB_WHITE_LEVEL 8
#define RGB_RED_LEVEL 8

// =============================================================================
// BLE HID Identity
// =============================================================================

#define BLE_HID_DEVICE_NAME "TVK"
#define BLE_HID_MANUFACTURER "Marcel"
#define BLE_HID_BATTERY_LEVEL 100

// =============================================================================
// Task Configuration
// =============================================================================

#define HID_TASK_STACK_SIZE 4096
#define HID_TASK_PRIORITY 2

#endif
