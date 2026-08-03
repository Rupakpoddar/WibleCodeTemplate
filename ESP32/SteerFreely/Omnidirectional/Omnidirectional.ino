/*
  * Project Information:
    - Developer: Rupak Poddar
    - Wible Code Template for ESP32
    - Tested on: ESP32-C3, ESP32-C6, ESP32-S3
    - Steer Freely Omnidirectional & Motion Control Example

  * Instructions:
    - After flashing this code, open the Wible app on your smartphone.
    - Then select "Steer Freely" and look for the listing called "Wible".
*/

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#ifdef _BLE_DEVICE_H_
  #error "Conflicting BLE library detected (possibly ArduinoBLE). Please remove it to proceed."
#endif

// UUIDs for the BLE service and characteristics
#define UART_SERVICE_UUID      "00000001-0000-FEED-0000-000000000000"
#define RX_CHARACTERISTIC_UUID "00000002-0000-FEED-0000-000000000000"
#define TX_CHARACTERISTIC_UUID "00000003-0000-FEED-0000-000000000000"

// Set verbose flag to enable print statements
#define VERBOSE true

// Cap minimum and maximum motor speed
#define MIN_SPEED 80
#define MAX_SPEED 255

//                    FRONT 
//                      ↑ 
//            M1 ○ ┌─────────┐ ○ M2 
//                 │  ROBOT  │ 
//            M4 ○ └─────────┘ ○ M3 
//                      ↓ 
//                    BACK
// 
//          A = Forward | B = Backward

// Motor driver and light GPIOs
#define M1A 18
#define M1B 19
#define M2A 20
#define M2B 21
#define M3A 2
#define M3B 3
#define M4A 6
#define M4B 7
#define LED 22

// BLE server/characteristic pointers and connection state
BLEServer *pServer = NULL;
BLECharacteristic *pRxCharacteristic = NULL;
BLECharacteristic *pTxCharacteristic = NULL;
volatile bool deviceConnected = false;

// Forward declaration
void stopMotors();

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    if (VERBOSE) {
      Serial.println("Device connected.");
    }
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    // Restart advertising after disconnecting
    pServer->getAdvertising()->start();
    if (VERBOSE) {
      Serial.println("Device disconnected. Re-advertising...");
    }
  }
};

class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String rxValue = pCharacteristic->getValue();
    if (rxValue.length() > 0) {
      if (VERBOSE) {
        Serial.print("\n----- Received String: ");
        Serial.print(rxValue);
        Serial.println(" -----\n");
      }

      // Parse rxValue in XXXYYYY format: XXX = speed, YYYY = command.
      unsigned char speed = rxValue.substring(0, 3).toInt();
      String command = rxValue.substring(3, 7);

      if (speed != 0) {
        speed = map(speed, 1, 100, MIN_SPEED, MAX_SPEED);
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
        digitalWrite(LED, HIGH);
      }

      // LED OFF
      if (command == "LTOF") {
        digitalWrite(LED, LOW);
      }

      // FORWARD
      if (command == "FWRD") {
        analogWrite(M1A, speed);
        analogWrite(M2A, speed);
        analogWrite(M3A, speed);
        analogWrite(M4A, speed);
        analogWrite(M1B, 0);
        analogWrite(M2B, 0);
        analogWrite(M3B, 0);
        analogWrite(M4B, 0);
      }

      // BACKWARD
      if (command == "BKWD") {
        analogWrite(M1A, 0);
        analogWrite(M2A, 0);
        analogWrite(M3A, 0);
        analogWrite(M4A, 0);
        analogWrite(M1B, speed);
        analogWrite(M2B, speed);
        analogWrite(M3B, speed);
        analogWrite(M4B, speed);
      }

      // LEFT
      if (command == "LEFT") {
        analogWrite(M1A, 0);
        analogWrite(M2A, speed);
        analogWrite(M3A, 0);
        analogWrite(M4A, speed);
        analogWrite(M1B, speed);
        analogWrite(M2B, 0);
        analogWrite(M3B, speed);
        analogWrite(M4B, 0);
      }

      // RIGHT
      if (command == "RGHT") {
        analogWrite(M1A, speed);
        analogWrite(M2A, 0);
        analogWrite(M3A, speed);
        analogWrite(M4A, 0);
        analogWrite(M1B, 0);
        analogWrite(M2B, speed);
        analogWrite(M3B, 0);
        analogWrite(M4B, speed);
      }

      /*
        * Utilize additional control buttons and multi-touch commands from below
      */
      // ROTATE LEFT
      if (command == "ROLT") {
        analogWrite(M1A, 0);
        analogWrite(M2A, speed);
        analogWrite(M3A, speed);
        analogWrite(M4A, 0);
        analogWrite(M1B, speed);
        analogWrite(M2B, 0);
        analogWrite(M3B, 0);
        analogWrite(M4B, speed);
      }

      // ROTATE RIGHT
      if (command == "RORT") {
        analogWrite(M1A, speed);
        analogWrite(M2A, 0);
        analogWrite(M3A, 0);
        analogWrite(M4A, speed);
        analogWrite(M1B, 0);
        analogWrite(M2B, speed);
        analogWrite(M3B, speed);
        analogWrite(M4B, 0);
      }

      // FORWARD + LEFT
      if (command == "FRLT") {
        analogWrite(M1A, 0);
        analogWrite(M2A, speed);
        analogWrite(M3A, 0);
        analogWrite(M4A, speed);
        analogWrite(M1B, 0);
        analogWrite(M2B, 0);
        analogWrite(M3B, 0);
        analogWrite(M4B, 0);
      }

      // FORWARD + RIGHT
      if (command == "FRRT") {
        analogWrite(M1A, speed);
        analogWrite(M2A, 0);
        analogWrite(M3A, speed);
        analogWrite(M4A, 0);
        analogWrite(M1B, 0);
        analogWrite(M2B, 0);
        analogWrite(M3B, 0);
        analogWrite(M4B, 0);
      }

      // FORWARD + ROTATE LEFT
      if (command == "FRRL") {
        analogWrite(M1A, speed/2);
        analogWrite(M2A, speed);
        analogWrite(M3A, speed);
        analogWrite(M4A, speed/2);
        analogWrite(M1B, 0);
        analogWrite(M2B, 0);
        analogWrite(M3B, 0);
        analogWrite(M4B, 0);
      }

      // FORWARD + ROTATE RIGHT
      if (command == "FRRR") {
        analogWrite(M1A, speed);
        analogWrite(M2A, speed/2);
        analogWrite(M3A, speed/2);
        analogWrite(M4A, speed);
        analogWrite(M1B, 0);
        analogWrite(M2B, 0);
        analogWrite(M3B, 0);
        analogWrite(M4B, 0);
      }

      // BACKWARD + LEFT
      if (command == "BKLT") {
        analogWrite(M1A, 0);
        analogWrite(M2A, 0);
        analogWrite(M3A, 0);
        analogWrite(M4A, 0);
        analogWrite(M1B, speed);
        analogWrite(M2B, 0);
        analogWrite(M3B, speed);
        analogWrite(M4B, 0);
      }

      // BACKWARD + RIGHT
      if (command == "BKRT") {
        analogWrite(M1A, 0);
        analogWrite(M2A, 0);
        analogWrite(M3A, 0);
        analogWrite(M4A, 0);
        analogWrite(M1B, 0);
        analogWrite(M2B, speed);
        analogWrite(M3B, 0);
        analogWrite(M4B, speed);
      }

      // BACKWARD + ROTATE LEFT
      if (command == "BKRL") {
        analogWrite(M1A, 0);
        analogWrite(M2A, 0);
        analogWrite(M3A, 0);
        analogWrite(M4A, 0);
        analogWrite(M1B, speed/2);
        analogWrite(M2B, speed);
        analogWrite(M3B, speed);
        analogWrite(M4B, speed/2);
      }

      // BACKWARD + ROTATE RIGHT
      if (command == "BKRR") {
        analogWrite(M1A, 0);
        analogWrite(M2A, 0);
        analogWrite(M3A, 0);
        analogWrite(M4A, 0);
        analogWrite(M1B, speed);
        analogWrite(M2B, speed/2);
        analogWrite(M3B, speed/2);
        analogWrite(M4B, speed);
      }

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
  }
};

void setup() {
  if (VERBOSE) {
    Serial.begin(115200);
    delay(100);
    Serial.println("\n\nWible ESP32 Steer Freely\n");
  }

  pinMode(M1A, OUTPUT);
  pinMode(M1B, OUTPUT);
  pinMode(M2A, OUTPUT);
  pinMode(M2B, OUTPUT);
  pinMode(M3A, OUTPUT);
  pinMode(M3B, OUTPUT);
  pinMode(M4A, OUTPUT);
  pinMode(M4B, OUTPUT);
  pinMode(LED, OUTPUT);

  stopMotors();

  // Initialize BLE
  BLEDevice::init("Wible");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(UART_SERVICE_UUID);

  // Create the RX Characteristic (write_nr)
  pRxCharacteristic = pService->createCharacteristic(
    RX_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_WRITE_NR
  );
  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // Create the TX Characteristic (read, notify)
  pTxCharacteristic = pService->createCharacteristic(
    TX_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_NOTIFY
  );

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(UART_SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  pAdvertising->setName("Wible");
  BLEDevice::startAdvertising();

  if (VERBOSE) {
    Serial.println("Device initialized. Advertising...");
  }
}

void loop() {
  if (!deviceConnected) {
    stopMotors();

    digitalWrite(LED, HIGH);
    delay(250);
    digitalWrite(LED, LOW);
    delay(250);
  }
}

void stopMotors() {
  analogWrite(M1A, 0);
  analogWrite(M1B, 0);
  analogWrite(M2A, 0);
  analogWrite(M2B, 0);
  analogWrite(M3A, 0);
  analogWrite(M3B, 0);
  analogWrite(M4A, 0);
  analogWrite(M4B, 0);
}
