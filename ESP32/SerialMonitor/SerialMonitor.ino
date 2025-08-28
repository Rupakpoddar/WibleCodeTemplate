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

// Check ESP32 board definition version
#if defined(ESP_ARDUINO_VERSION_MAJOR) && defined(ESP_ARDUINO_VERSION_MINOR) && defined(ESP_ARDUINO_VERSION_PATCH)
  #if (ESP_ARDUINO_VERSION_MAJOR > 3) || \
      (ESP_ARDUINO_VERSION_MAJOR == 3 && ESP_ARDUINO_VERSION_MINOR > 2) || \
      (ESP_ARDUINO_VERSION_MAJOR == 3 && ESP_ARDUINO_VERSION_MINOR == 2 && ESP_ARDUINO_VERSION_PATCH > 1)
    #error "ESP32 Arduino Core version is unsupported. Please downgrade to version 3.2.1."
  #endif
#endif

#ifdef _BLE_DEVICE_H_
  #error "Conflicting BLE library detected (possibly ArduinoBLE). Please remove it to proceed."
#endif

// UUIDs for the BLE service and characteristic
#define SERVICE_UUID           "66443771-D481-49B0-BE32-8CE24AC0F09C"
#define CHARACTERISTIC_UUID    "66443772-D481-49B0-BE32-8CE24AC0F09C"

// BLE server/characteristic pointers and connection state
BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
bool deviceConnected = false;

// Initialize counter
int counter = 0;

// Implement delay without blocking
unsigned long previousMillis = 0; // Stores the last time the event occurred
const long interval = 1000; // Interval for the delay in milliseconds

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Device connected.");
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    // Restart advertising after disconnecting
    pServer->getAdvertising()->start();
    Serial.println("Device disconnected. Re-advertising...");
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
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE |
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
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  
  Serial.println("Device initialized. Advertising...");
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
