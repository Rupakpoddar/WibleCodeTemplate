/*
  * Project Information:
    - Developer: Rupak Poddar
    - Wible Code Template for Arduino (BLE-supported boards)
    - Tested on: Arduino Uno R4 WiFi
    - Steer Freely Basic & Motion Control Example

  * Instructions:
    - After flashing this code, open the Wible app on your smartphone.
    - Then select "Steer Freely" and look for the listing called "Wible".

  * Required Library:
    - ArduinoBLE by Arduino (install via Arduino IDE Library Manager)
      https://docs.arduino.cc/libraries/arduinoble/
      or https://github.com/arduino-libraries/ArduinoBLE
*/

#include <ArduinoBLE.h>

#ifdef _WIBLE_ENABLED_
  #error "A modified ArduinoBLE library is installed. Please remove it and use the official ArduinoBLE library."
#endif

// UUIDs for the BLE service and characteristics
#define UART_SERVICE_UUID      "00000001-0000-FEED-0000-000000000000"
#define RX_CHARACTERISTIC_UUID "00000002-0000-FEED-0000-000000000000"
#define TX_CHARACTERISTIC_UUID "00000003-0000-FEED-0000-000000000000"

// Maximum payload size of the RX and TX characteristics
#define CHARACTERISTIC_SIZE 96

// Set verbose flag to enable print statements
#define VERBOSE true

// Motor speed limits and motion thresholds
#define MIN_SPEED 80
#define MAX_SPEED 255
#define MOTION_TIMEOUT_MS 250
#define MOTION_X_UPPER_BOUND (128+32)
#define MOTION_X_LOWER_BOUND (128-32)
#define MOTION_Y_UPPER_BOUND (128+32)
#define MOTION_Y_LOWER_BOUND (128-32)

//                    FRONT
//                      ↑
//               ○ ┌─────────┐ ○
//              M1 │  ROBOT  │ M2
//               ○ └─────────┘ ○
//                      ↓
//                    BACK
//
//          A = Forward | B = Backward

// Motor driver pins
// Note: All four motor pins must support analogWrite().
//       On the Arduino Uno R4 WiFi these are D3, D5, D6, D9, D10 and D11.
#define M1A 5
#define M1B 6
#define M2A 10
#define M2B 11

// Forward declarations
void handleCommand(const String &rxValue);
void stopMotors();

// Create UART BLE service with separate RX and TX characteristics
BLEService uartService(UART_SERVICE_UUID);
BLECharacteristic rxCharacteristic(RX_CHARACTERISTIC_UUID,
                                   BLEWriteWithoutResponse, CHARACTERISTIC_SIZE);
BLECharacteristic txCharacteristic(TX_CHARACTERISTIC_UUID,
                                   BLERead | BLENotify, CHARACTERISTIC_SIZE);

// Global variables
bool deviceConnected = false;
bool motionActive = false;
unsigned long lastMotionCommandTime = 0;

void setup() {
  if (VERBOSE) {
    Serial.begin(9600);
    delay(100);
    Serial.println("\n\nWible Arduino Steer Freely\n");
  }

  pinMode(M1A, OUTPUT);
  pinMode(M1B, OUTPUT);
  pinMode(M2A, OUTPUT);
  pinMode(M2B, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  stopMotors();

  // Initialize BLE module
  if (!BLE.begin()) {
    if (VERBOSE) {
      Serial.println("BLE initialization failed!");
    }
    while (1);
  }

  // Configure BLE parameters
  BLE.setLocalName("Wible");
  BLE.setDeviceName("Wible");
  BLE.setAdvertisedService(uartService);

  // Add the characteristics to the service and add the service
  uartService.addCharacteristic(rxCharacteristic);
  uartService.addCharacteristic(txCharacteristic);
  BLE.addService(uartService);

  // Start advertising BLE service
  BLE.advertise();

  if (VERBOSE) {
    Serial.println("Device initialized. Advertising...");
  }
}

void loop() {
  // Poll for BLE events
  BLE.poll();

  // Stop the motors if no motion command has been received recently
  if (motionActive) {
    if (millis() - lastMotionCommandTime > MOTION_TIMEOUT_MS) {
      motionActive = false;
      stopMotors();
    }
  }

  // Get the central device, if any central is connected this will be non-null
  BLEDevice central = BLE.central();

  if (central) {
    // A central is connected now
    if (!deviceConnected) {
      deviceConnected = true;
      if (VERBOSE) {
        Serial.println("Device connected.");
      }
    }

    // Check if the central has written new data to the RX characteristic
    if (rxCharacteristic.written()) {
      char buffer[CHARACTERISTIC_SIZE + 1];
      int length = rxCharacteristic.valueLength();
      if (length > CHARACTERISTIC_SIZE) {
        length = CHARACTERISTIC_SIZE;
      }
      memcpy(buffer, rxCharacteristic.value(), length);
      buffer[length] = '\0';

      handleCommand(String(buffer));
    }
  } else {
    // No central is connected. If we were previously connected, stop the motors and restart advertising.
    if (deviceConnected) {
      deviceConnected = false;
      stopMotors();
      BLE.advertise();
      if (VERBOSE) {
        Serial.println("Device disconnected. Re-advertising...");
      }
    }

    // Blink LED to indicate waiting for connection
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
  }
}

void handleCommand(const String &rxValue) {
  if (rxValue.length() == 0) {
    return;
  }

  if (VERBOSE) {
    Serial.print("\n----- Received string: ");
    Serial.print(rxValue);
    Serial.println(" -----\n");
  }

  unsigned char speed = 0;
  String command = "";

  if (rxValue.length() == 7) {  // Button input
    // Parse rxValue in {SSS}{CCCC} format: SSS = speed, CCCC = command.
    speed = rxValue.substring(0, 3).toInt();
    command = rxValue.substring(3, 7);

    if (speed != 0) {
      speed = map(speed, 1, 100, MIN_SPEED, MAX_SPEED);
    }
  }
  else if (rxValue.length() == 8) { // Motion input
    lastMotionCommandTime = millis();
    motionActive = true;

    // Parse rxValue in X{AAA}Y{BBB} format: AAA = X-axis position from 0 to 255, BBB = Y-axis position from 0 to 255.
    unsigned char xValue = rxValue.substring(1, 4).toInt();
    unsigned char yValue = rxValue.substring(5, 8).toInt();

    // Phone position: Neutral
    if ((xValue <= MOTION_X_UPPER_BOUND)
        && (xValue >= MOTION_X_LOWER_BOUND)
        && (yValue <= MOTION_Y_UPPER_BOUND)
        && (yValue >= MOTION_Y_LOWER_BOUND)) {
      stopMotors();
      return;
    }

    // Phone position: Forward
    if ((xValue > MOTION_X_UPPER_BOUND)
        && (yValue <= MOTION_Y_UPPER_BOUND)
        && (yValue >= MOTION_Y_LOWER_BOUND)) {
      speed = map(xValue, MOTION_X_UPPER_BOUND, 255, MIN_SPEED, MAX_SPEED);
      command = "FWRD";
    }

    // Phone position: Backward
    if ((xValue < MOTION_X_LOWER_BOUND)
        && (yValue <= MOTION_Y_UPPER_BOUND)
        && (yValue >= MOTION_Y_LOWER_BOUND)) {
      speed = map(MOTION_X_LOWER_BOUND-xValue, 0, MOTION_X_LOWER_BOUND, MIN_SPEED, MAX_SPEED);
      command = "BKWD";
    }

    // Phone position: Left
    if ((xValue <= MOTION_X_UPPER_BOUND)
        && (xValue >= MOTION_X_LOWER_BOUND)
        && (yValue < MOTION_Y_LOWER_BOUND)) {
      speed = map(MOTION_Y_LOWER_BOUND-yValue, 0, MOTION_Y_LOWER_BOUND, MIN_SPEED, MAX_SPEED);
      command = "LEFT";
    }

    // Phone position: Right
    if ((xValue <= MOTION_X_UPPER_BOUND)
        && (xValue >= MOTION_X_LOWER_BOUND)
        && (yValue > MOTION_Y_UPPER_BOUND)) {
      speed = map(yValue, MOTION_Y_UPPER_BOUND, 255, MIN_SPEED, MAX_SPEED);
      command = "RGHT";
    }

    // Phone position: Front left
    if ((xValue > MOTION_X_UPPER_BOUND)
        && (yValue < MOTION_Y_LOWER_BOUND)) {
      speed = map(xValue, MOTION_X_UPPER_BOUND, 255, MIN_SPEED, MAX_SPEED);
      command = "FRLT";
    }

    // Phone position: Front right
    if ((xValue > MOTION_X_UPPER_BOUND)
        && (yValue > MOTION_Y_UPPER_BOUND)) {
      speed = map(xValue, MOTION_X_UPPER_BOUND, 255, MIN_SPEED, MAX_SPEED);
      command = "FRRT";
    }

    // Phone position: Back left
    if ((xValue < MOTION_X_LOWER_BOUND)
        && (yValue < MOTION_Y_LOWER_BOUND)) {
      speed = map(MOTION_X_LOWER_BOUND-xValue, 0, MOTION_X_LOWER_BOUND, MIN_SPEED, MAX_SPEED);
      command = "BKLT";
    }

    // Phone position: Back right
    if ((xValue < MOTION_X_LOWER_BOUND)
        && (yValue > MOTION_Y_UPPER_BOUND)) {
      speed = map(MOTION_X_LOWER_BOUND-xValue, 0, MOTION_X_LOWER_BOUND, MIN_SPEED, MAX_SPEED);
      command = "BKRT";
    }
  }
  else {
    stopMotors();
    if (VERBOSE) {
      Serial.println("Invalid input");
    }
    return;
  }

  if (VERBOSE) {
    Serial.print("Speed: ");
    Serial.println(speed);
    Serial.print("Command: ");
    Serial.println(command);
  }

  // STOP
  if (command == "STOP") {
    stopMotors();
  }

  // LED ON
  if (command == "LTON") {
    digitalWrite(LED_BUILTIN, HIGH);
  }

  // LED OFF
  if (command == "LTOF") {
    digitalWrite(LED_BUILTIN, LOW);
  }

  // FORWARD
  if (command == "FWRD") {
    analogWrite(M1A, speed);
    analogWrite(M2A, speed);
    analogWrite(M1B, 0);
    analogWrite(M2B, 0);
  }

  // BACKWARD
  if (command == "BKWD") {
    analogWrite(M1A, 0);
    analogWrite(M2A, 0);
    analogWrite(M1B, speed);
    analogWrite(M2B, speed);
  }

  // LEFT
  if (command == "LEFT") {
    analogWrite(M1A, 0);
    analogWrite(M2A, speed);
    analogWrite(M1B, speed);
    analogWrite(M2B, 0);
  }

  // RIGHT
  if (command == "RGHT") {
    analogWrite(M1A, speed);
    analogWrite(M2A, 0);
    analogWrite(M1B, 0);
    analogWrite(M2B, speed);
  }

  /*
    * Utilize additional control buttons and multi-touch commands from below
  */
  // ROTATE LEFT
  // if (command == "ROLT") {}

  // ROTATE RIGHT
  // if (command == "RORT") {}

  // FORWARD + LEFT
  if (command == "FRLT") {
    analogWrite(M1A, speed/2);
    analogWrite(M2A, speed);
    analogWrite(M1B, 0);
    analogWrite(M2B, 0);
  }

  // FORWARD + RIGHT
  if (command == "FRRT") {
    analogWrite(M1A, speed);
    analogWrite(M2A, speed/2);
    analogWrite(M1B, 0);
    analogWrite(M2B, 0);
  }

  // FORWARD + ROTATE LEFT
  // if (command == "FRRL") {}

  // FORWARD + ROTATE RIGHT
  // if (command == "FRRR") {}

  // BACKWARD + LEFT
  if (command == "BKLT") {
    analogWrite(M1A, 0);
    analogWrite(M2A, 0);
    analogWrite(M1B, speed/2);
    analogWrite(M2B, speed);
  }

  // BACKWARD + RIGHT
  if (command == "BKRT") {
    analogWrite(M1A, 0);
    analogWrite(M2A, 0);
    analogWrite(M1B, speed);
    analogWrite(M2B, speed/2);
  }

  // BACKWARD + ROTATE LEFT
  // if (command == "BKRL") {}

  // BACKWARD + ROTATE RIGHT
  // if (command == "BKRR") {}

  /*
    * Utilize additional function buttons from below
  */
  // FUNCTION 1
  // if (command == "FNC1") {}

  // FUNCTION 2
  // if (command == "FNC2") {}

  // FUNCTION 3
  // if (command == "FNC3") {}

  // FUNCTION 4
  // if (command == "FNC4") {}
}

void stopMotors() {
  analogWrite(M1A, 0);
  analogWrite(M1B, 0);
  analogWrite(M2A, 0);
  analogWrite(M2B, 0);
}
