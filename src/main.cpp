/*
  ==============================================================================
  Project      : ESP32-S3 BOOT Interrupt BLE HID
  Firmware     : boot_interrupt_ble_hid_wifi_ntp_stable_text_v5.9.1
  Target MCU   : ESP32-S3
  Framework    : Arduino on ESP32-S3
  HID Library  : HijelHID_BLEKeyboard
  HID Device   : BLE HID Keyboard
  BLE Name     : ESP32 Login HID Test

  ------------------------------------------------------------------------------
  Purpose
  ------------------------------------------------------------------------------
  Use the BOOT button on GPIO 0 as an interrupt source to trigger BLE HID output.

  On startup:
    - Connect to Wi-Fi
    - Synchronize date/time using NTP
    - Start BLE HID keyboard
    - Arm BOOT interrupt

  On valid BOOT press:
    - Send first configured text
    - Send second configured text
    - Send current date/time from NTP
    - Send fixed line ending

  ------------------------------------------------------------------------------
  HID Output Example
  ------------------------------------------------------------------------------
  1234567890 Marcel Levinspuhl Segundo texto 2026-06-28 20:30:45

  ------------------------------------------------------------------------------
  Stability Notes
  ------------------------------------------------------------------------------
  This version does not send TAB.
  This version does not send a separate field separator.
  Space between first and second text is embedded in HID_FIRST_TEXT_TO_TYPE.
  Space between second text and date/time is embedded in the date/time format.
  Line ending is constant and is not modified after sending.

  ==============================================================================
*/

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <Adafruit_NeoPixel.h>
#include <HijelHID_BLEKeyboard.h>

// ==============================================================================
// 1) Wi-Fi Configuration
// ==============================================================================

const char* ssid = "SKYZHE1K";
const char* password = "rHGMRSNqyFjY";

#define WIFI_CONNECT_TIMEOUT_MS 15000
#define NTP_SYNC_TIMEOUT_MS 15000

#define LOCAL_TIMEZONE "GMT0IST,M3.5.0/1,M10.5.0/2"

#define NTP_SERVER_1 "pool.ntp.org"
#define NTP_SERVER_2 "time.google.com"

// ==============================================================================
// 2) HID Text Configuration
// ==============================================================================

#define HID_FIRST_TEXT_TO_TYPE  "1234567890 Marcel Levinspuhl "
#define HID_SECOND_TEXT_TO_TYPE "Segundo texto"

#define HID_LINE_ENDING "\n"

#define HID_CHAR_DELAY_MS 20
#define HID_ENTER_DELAY_MS 40

// ==============================================================================
// 3) BOOT / Interrupt Configuration
// ==============================================================================

#define PRESS_CONFIRM_MS 30
#define STARTUP_ARM_STABLE_MS 1000
#define RELEASE_STABLE_TIME_MS 50

// ==============================================================================
// 4) Pin Assignment
// ==============================================================================

#define BOOT_BUTTON_PIN 0

#define RGB_LED_PIN 48
#define NUM_LEDS 1

// ==============================================================================
// 5) BLE HID Identity
// ==============================================================================

#define BLE_HID_DEVICE_NAME "ESP32 Login HID Test"
#define BLE_HID_MANUFACTURER "Marcel"
#define BLE_HID_BATTERY_LEVEL 100

// ==============================================================================
// 6) RGB Configuration
// ==============================================================================

#define RGB_WHITE_LEVEL 8
#define RGB_RED_LEVEL 8
#define RGB_GREEN_LEVEL 8
#define RGB_BLUE_LEVEL 8

#define DISCONNECTED_LED_PULSE_MS 50
#define STATUS_LED_PULSE_MS 200

// ==============================================================================
// 7) Task Configuration
// ==============================================================================

#define HID_TASK_STACK_SIZE 4096
#define HID_TASK_PRIORITY 2

// ==============================================================================
// 8) Objects
// ==============================================================================

Adafruit_NeoPixel rgb(NUM_LEDS, RGB_LED_PIN, NEO_GRB + NEO_KHZ800);

HijelHID_BLEKeyboard keyboard(
  BLE_HID_DEVICE_NAME,
  BLE_HID_MANUFACTURER,
  BLE_HID_BATTERY_LEVEL
);

// ==============================================================================
// 9) RTOS / Interrupt State
// ==============================================================================

TaskHandle_t hidTaskHandle = nullptr;

portMUX_TYPE isrMux = portMUX_INITIALIZER_UNLOCKED;

volatile bool systemReady = false;
volatile bool triggerArmed = false;

bool wifiConnected = false;
bool timeSynchronized = false;

// ==============================================================================
// 10) Timing Helper
// ==============================================================================

void taskPauseMs(uint32_t delayMs) {
  if (delayMs == 0) {
    taskYIELD();
    return;
  }

  vTaskDelay(pdMS_TO_TICKS(delayMs));
}

// ==============================================================================
// 11) RGB Functions
// ==============================================================================

void setRGB(uint8_t r, uint8_t g, uint8_t b) {
  rgb.setPixelColor(0, rgb.Color(r, g, b));
  rgb.show();
}

void rgbOff() {
  setRGB(0, 0, 0);
}

void rgbWhite() {
  setRGB(RGB_WHITE_LEVEL, RGB_WHITE_LEVEL, RGB_WHITE_LEVEL);
}

void rgbRed() {
  setRGB(RGB_RED_LEVEL, 0, 0);
}

void rgbGreen() {
  setRGB(0, RGB_GREEN_LEVEL, 0);
}

void rgbBlue() {
  setRGB(0, 0, RGB_BLUE_LEVEL);
}

void rgbDisconnectedPulse() {
  rgbRed();
  taskPauseMs(DISCONNECTED_LED_PULSE_MS);
  rgbOff();
}

void rgbSetupOkPulse() {
  rgbGreen();
  taskPauseMs(STATUS_LED_PULSE_MS);
  rgbOff();
}

void rgbSetupFailPulse() {
  rgbRed();
  taskPauseMs(STATUS_LED_PULSE_MS);
  rgbOff();
}

// ==============================================================================
// 12) Wi-Fi / NTP Functions
// ==============================================================================

bool connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  unsigned long startMs = millis();

  while (WiFi.status() != WL_CONNECTED) {
    if ((millis() - startMs) >= WIFI_CONNECT_TIMEOUT_MS) {
      return false;
    }

    taskPauseMs(100);
  }

  return true;
}

bool synchronizeTimeWithNtp() {
  configTzTime(
    LOCAL_TIMEZONE,
    NTP_SERVER_1,
    NTP_SERVER_2
  );

  struct tm timeInfo;
  unsigned long startMs = millis();

  while (!getLocalTime(&timeInfo, 250)) {
    if ((millis() - startMs) >= NTP_SYNC_TIMEOUT_MS) {
      return false;
    }

    taskPauseMs(100);
  }

  return true;
}

void setupWiFiAndTime() {
  rgbBlue();

  wifiConnected = connectWiFi();

  if (!wifiConnected) {
    timeSynchronized = false;
    rgbSetupFailPulse();
    return;
  }

  timeSynchronized = synchronizeTimeWithNtp();

  if (timeSynchronized) {
    rgbSetupOkPulse();
  } else {
    rgbSetupFailPulse();
  }
}

// ==============================================================================
// 13) Trigger Arm Control
// ==============================================================================

void enableSystemAndArmTrigger() {
  portENTER_CRITICAL(&isrMux);
  systemReady = true;
  triggerArmed = true;
  portEXIT_CRITICAL(&isrMux);
}

void armTrigger() {
  portENTER_CRITICAL(&isrMux);
  triggerArmed = true;
  portEXIT_CRITICAL(&isrMux);
}

// ==============================================================================
// 14) GPIO Interrupt Service Routine
// ==============================================================================

void IRAM_ATTR bootButtonIsr() {
  BaseType_t higherPriorityTaskWoken = pdFALSE;
  bool notifyTask = false;

  portENTER_CRITICAL_ISR(&isrMux);

  if (systemReady && triggerArmed) {
    triggerArmed = false;
    notifyTask = true;
  }

  portEXIT_CRITICAL_ISR(&isrMux);

  if (notifyTask && hidTaskHandle != nullptr) {
    vTaskNotifyGiveFromISR(hidTaskHandle, &higherPriorityTaskWoken);
  }

  if (higherPriorityTaskWoken == pdTRUE) {
    portYIELD_FROM_ISR();
  }
}

// ==============================================================================
// 15) BOOT State Qualification
// ==============================================================================

bool bootIsPressed() {
  return digitalRead(BOOT_BUTTON_PIN) == LOW;
}

bool bootIsReleased() {
  return digitalRead(BOOT_BUTTON_PIN) == HIGH;
}

bool confirmBootPressStable() {
  taskPauseMs(PRESS_CONFIRM_MS);

  if (bootIsPressed()) {
    return true;
  }

  return false;
}

void waitForBootReleasedStable(uint32_t stableTimeMs) {
  unsigned long highStartMs = 0;

  for (;;) {
    bool released = bootIsReleased();
    unsigned long now = millis();

    if (released) {
      if (highStartMs == 0) {
        highStartMs = now;
      }

      if ((now - highStartMs) >= stableTimeMs) {
        return;
      }
    } else {
      highStartMs = 0;
    }

    taskPauseMs(1);
  }
}

// ==============================================================================
// 16) HID Send Functions
// ==============================================================================

void sendOneCharacter(char characterToSend) {
  if (!keyboard.isConnected()) {
    return;
  }

  char buffer[2];
  buffer[0] = characterToSend;
  buffer[1] = '\0';

  keyboard.print(buffer);

  taskPauseMs(HID_CHAR_DELAY_MS);
}

void sendTextCharByChar(const char *textToSend) {
  const char *textPointer = textToSend;

  while (*textPointer != '\0') {
    if (!keyboard.isConnected()) {
      return;
    }

    sendOneCharacter(*textPointer);
    textPointer++;
  }
}

void sendCurrentDateTimeAction() {
  if (!keyboard.isConnected()) {
    return;
  }

  struct tm timeInfo;
  char dateTimeBuffer[32];

  if (getLocalTime(&timeInfo, 250)) {
    strftime(
      dateTimeBuffer,
      sizeof(dateTimeBuffer),
      " %Y-%m-%d %H:%M:%S",
      &timeInfo
    );

    sendTextCharByChar(dateTimeBuffer);
  } else {
    sendTextCharByChar(" TIME_NOT_SYNCED");
  }
}

void sendLineEndingAction() {
  if (!keyboard.isConnected()) {
    return;
  }

  taskPauseMs(HID_ENTER_DELAY_MS);

  keyboard.print(HID_LINE_ENDING);

  taskPauseMs(HID_ENTER_DELAY_MS);
}

void sendBootHidSequence() {
  sendTextCharByChar(HID_FIRST_TEXT_TO_TYPE);

  sendTextCharByChar(HID_SECOND_TEXT_TO_TYPE);

  sendCurrentDateTimeAction();

  sendLineEndingAction();
}

// ==============================================================================
// 17) HID Task
// ==============================================================================

void hidTask(void *parameter) {
  for (;;) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    if (!confirmBootPressStable()) {
      rgbOff();
      waitForBootReleasedStable(RELEASE_STABLE_TIME_MS);
      armTrigger();
      continue;
    }

    rgbWhite();

    if (keyboard.isConnected()) {
      sendBootHidSequence();
    } else {
      rgbDisconnectedPulse();
    }

    waitForBootReleasedStable(RELEASE_STABLE_TIME_MS);

    rgbOff();

    armTrigger();
  }
}

// ==============================================================================
// 18) Setup
// ==============================================================================

void setup() {
  pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);

  rgb.begin();
  rgb.clear();
  rgb.show();
  rgbOff();

  setupWiFiAndTime();

  keyboard.begin();

  xTaskCreate(
    hidTask,
    "hidTask",
    HID_TASK_STACK_SIZE,
    nullptr,
    HID_TASK_PRIORITY,
    &hidTaskHandle
  );

  waitForBootReleasedStable(STARTUP_ARM_STABLE_MS);

  attachInterrupt(
    digitalPinToInterrupt(BOOT_BUTTON_PIN),
    bootButtonIsr,
    FALLING
  );

  enableSystemAndArmTrigger();
}

// ==============================================================================
// 19) Arduino Loop
// ==============================================================================

void loop() {
  vTaskDelay(portMAX_DELAY);
}