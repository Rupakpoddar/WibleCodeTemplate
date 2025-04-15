/*
  * Developer: Rupak Poddar
  * Wible Code Template for Arduino
  * Steer Freely
*/

#include <ArduinoBLE.h>

#ifndef _WIBLE_ENABLED_
  #error "Please install the modified ArduinoBLE library from: https://github.com/Rupakpoddar/WibleCodeTemplate/raw/master/Arduino/ArduinoBLE.zip"
#endif

// Create BLE service & characteristic to allow remote device to read, write, and notify
BLEService TX_RX_Service("19B10010-E8F2-537E-4F6C-D104768A1214");
BLECharacteristic TX_RX_Characteristic("19B10011-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWriteWithoutResponse | BLENotify, 20);

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
  BLE.setAdvertisedService(TX_RX_Service);

  // Add the characteristic to the service and add the service
  TX_RX_Service.addCharacteristic(TX_RX_Characteristic);
  BLE.addService(TX_RX_Service);

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

    // Check if central has written new data to the TX_RX characteristic
    if (TX_RX_Characteristic.written()) {
      const uint8_t* value = TX_RX_Characteristic.value();
      String receivedString = String((char*)value);

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
