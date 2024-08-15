/*
  * Developer: Rupak Poddar
  * Wible Code Template for ESP32
  * Serial Monitor
*/

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
bool deviceConnected = false;

// UUIDs for the BLE Service and Characteristic
#define SERVICE_UUID           "66443771-D481-49B0-BE32-8CE24AC0F09C"
#define CHARACTERISTIC_UUID    "66443772-D481-49B0-BE32-8CE24AC0F09C"

// Initialize counter
int counter = 0;

// Implement delay without blocking
unsigned long previousMillis = 0; // Stores the last time the event occurred
const long interval = 1000; // Interval for the delay in milliseconds

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String rxValue = pCharacteristic->getValue();
    if (rxValue.length() > 0) {
      Serial.print("Received: ");
      Serial.println(rxValue);
    }
  }
};

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n\nWible ESP32 Serial Monitor\n");

  // Initialize BLE
  BLEDevice::init("Wible");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create the BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_WRITE_NR |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  
  Serial.println("BLE device active, waiting for connections...\n");
}

void loop() {
  if (deviceConnected) {
    unsigned long currentMillis = millis(); // Get the current time

    // Check if the interval has passed
    if (currentMillis - previousMillis >= interval) {
      // Save the last time the event occurred
      previousMillis = currentMillis;

      // Prepare the value to send
      String counterString = "Counter value: " + String(counter) + "\n";

      // Write the counter value to remote device
      pCharacteristic->setValue(counterString.c_str());
      pCharacteristic->notify();
      
      // Increment the counter
      counter++;
    }
  }
}
