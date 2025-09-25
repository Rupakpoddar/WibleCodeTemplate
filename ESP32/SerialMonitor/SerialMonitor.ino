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

#ifdef _BLE_DEVICE_H_
  #error "Conflicting BLE library detected (possibly ArduinoBLE). Please remove it to proceed."
#endif

// UUIDs for the BLE service and characteristics
#define UART_SERVICE_UUID      "00000001-0000-FEED-0000-000000000000"
#define RX_CHARACTERISTIC_UUID "00000002-0000-FEED-0000-000000000000"
#define TX_CHARACTERISTIC_UUID "00000003-0000-FEED-0000-000000000000"

// BLE server/characteristic pointers and connection state
BLEServer *pServer = NULL;
BLECharacteristic *pRxCharacteristic = NULL;
BLECharacteristic *pTxCharacteristic = NULL;
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
  pTxCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(UART_SERVICE_UUID);
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
      pTxCharacteristic->setValue(counterString.c_str());
      pTxCharacteristic->notify();
      
      // Increment the counter
      counter++;
    }
  }
}
