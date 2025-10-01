/*
  * Project Information:
    - Developer: Rupak Poddar
    - Wible Code Template for Arduino (BLE-supported boards)
    - Tested on: Arduino Uno R4 WiFi
    - Steer Freely Example

  * Instructions:
    - After flashing this code, open the Wible app on your smartphone.
    - Then select "Steer Freely" and look for the listing called "Wible".

  * Required Library:
    - ArduinoBLE by Arduino (install via Arduino IDE Library Manager)
      https://docs.arduino.cc/libraries/arduinoble/
      or https://github.com/arduino-libraries/ArduinoBLE
*/

#include <ArduinoBLE.h>

// Create UART BLE service with separate RX and TX characteristics
BLEService uartService("00000001-0000-FEED-0000-000000000000");
BLECharacteristic rxCharacteristic("00000002-0000-FEED-0000-000000000000",
                                   BLEWriteWithoutResponse, 96);
BLECharacteristic txCharacteristic("00000003-0000-FEED-0000-000000000000",
                                   BLERead | BLENotify, 96);

// Set verbose flag to enable print statements.
bool verbose = true;

// Motor driver pins
#define M1A 5
#define M1B 6
#define M2A 10
#define M2B 11

// Robot control variables
String receivedString = "";
unsigned char speed = 100;
String command = "STOP";
bool ignitionState = false;

// New flag to track BLE connection status changes.
bool deviceConnected = false;

void setup() {
  if (verbose) {
    Serial.begin(9600);
    delay(100);
    Serial.println("\n\nWible Arduino Steer Freely\n");
  }

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(M1A, OUTPUT);
  pinMode(M1B, OUTPUT);
  pinMode(M2A, OUTPUT);
  pinMode(M2B, OUTPUT);

  _STOP();

  // Initialize BLE module
  if (!BLE.begin()) {
    if (verbose) {
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

  if (verbose) {
    Serial.println("Device initialized. Advertising...");
  }
}

void loop() {
  // Poll for BLE events
  BLE.poll();
  
  // Get the central device, if any central is connected this will be non-null.
  BLEDevice central = BLE.central();

  if (central) {
    // A central is connected now.
    if (!deviceConnected) {
      if (verbose) {
        Serial.println("Device connected.");
      }
      deviceConnected = true;
    }

    // Check if central has written new data to the RX characteristic
    if (rxCharacteristic.written()) {
      const uint8_t* value = rxCharacteristic.value();
      int length = rxCharacteristic.valueLength();
      String receivedString = String((char*)value).substring(0, length);

      if (verbose) {
        Serial.print("\n----- Received String: ");
        Serial.print(receivedString);
        Serial.println(" -----\n");
      }

      // Split string values: first 3 characters denote speed, next 4 denote command.
      speed = receivedString.substring(0, 3).toInt();
      command = receivedString.substring(3, 7);

      if (verbose) {
        Serial.print("Speed: ");
        Serial.println(speed);
        Serial.print("Command: ");
        Serial.println(command);
      }

      // Process ignition commands
      if (command == "IGON") {
        ignitionState = true;
      } else if (command == "IGOF") {
        ignitionState = false;
        _STOP();
      }

      // Process STOP and LED commands
      if (command == "STOP") {
        _STOP();
      }
      if (command == "LTON") {
        digitalWrite(LED_BUILTIN, HIGH);
      }
      if (command == "LTOF") {
        digitalWrite(LED_BUILTIN, LOW);
      }

      /*
        * Utilize the Auxiliary buttons from below
      */
      // if (command == "AUX1"){}
      // if (command == "AUX2"){}
      // if (command == "AUX3"){}
      // if (command == "AUX4"){}
      // if (command == "AUX5"){}
      // if (command == "AUX6"){}

      /*
        * Uncomment the following line to disable ignition button
      */
      // ignitionState = true;

      // Process motor commands if ignition is enabled
      if (ignitionState) {
        // FORWARD
        if (command == "FWRD") {
          analogWrite(M1A, speed);
          analogWrite(M1B, 0);
          analogWrite(M2A, speed);
          analogWrite(M2B, 0);
        }
        // BACKWARD
        if (command == "BKWD") {
          analogWrite(M1A, 0);
          analogWrite(M1B, speed);
          analogWrite(M2A, 0);
          analogWrite(M2B, speed);
        }
        // LEFT
        if (command == "LEFT") {
          analogWrite(M1A, 0);
          analogWrite(M1B, speed);
          analogWrite(M2A, speed);
          analogWrite(M2B, 0);
        }
        // RIGHT
        if (command == "RGHT") {
          analogWrite(M1A, speed);
          analogWrite(M1B, 0);
          analogWrite(M2A, 0);
          analogWrite(M2B, speed);
        }

        /*
          * Utilize the multi-touch capability of the app from below
        */
        // if (command == "FRLT") {} // FORWARD + LEFT
        // if (command == "FRRT") {} // FORWARD + RIGHT
        // if (command == "BKLT") {} // BACKWARD + LEFT
        // if (command == "BKRT") {} // BACKWARD + RIGHT
      } else {
        if (verbose) {
          Serial.println("+------------------------+");
          Serial.println("|                        |");
          Serial.println("|   IGNITION STATE OFF   |");
          Serial.println("|                        |");
          Serial.println("+------------------------+");
        }
      }
    }
  } else {
    // No central is connected. If we were previously connected, clear ignition state, stop motors and restart advertising.
    if (deviceConnected) {
      if (verbose) {
        Serial.println("Device disconnected. Re-advertising...");
      }
      deviceConnected = false;
      ignitionState = false;
      _STOP();
      BLE.advertise();
    }
    
    // Blink LED to indicate waiting for connection.
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
  }
}

void _STOP() {
  analogWrite(M1A, 0);
  analogWrite(M1B, 0);
  analogWrite(M2A, 0);
  analogWrite(M2B, 0);
}
