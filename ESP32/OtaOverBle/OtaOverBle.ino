/*
  * Project Information:
    - Developer: Rupak Poddar
    - Wible Code Template for ESP32
    - Tested on: ESP32-C3, ESP32-C6, ESP32-S3
    - Firmware Updates Over BLE Example

  * NOTE:
    - This base sketch enables OTA firmware updates via BLE on the ESP32.
    - Flash this code first before attempting any over-the-air updates.

  * Export Update File:
    - In the Arduino IDE, select the correct board from Tools --> Board.
    - Make changes in this template or integrate it into your project.
    - In Arduino IDE, go to Sketch --> Export Compiled Binary.
    - A 'build' folder will be created inside your project directory.  
    - Inside the 'build' folder, open the subfolder named after your selected MCU.  
    - Locate the file with the '.ino.bin' extension and move it to your phone.

  * Instructions:
    - Once you’ve completed the steps above, launch the Wible app on your smartphone.
    - Then select "OTA Update" and look for the listing called "Wible".
    - Select the '.ino.bin' file you placed on your phone.
    - Tap the "Start" button in the Wible app to begin the OTA update (file transfer).  
    - After the transfer is complete, the ESP32 will verify the firmware and automatically reboot into the new version.  
*/

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <Update.h>

#ifdef _BLE_DEVICE_H_
  #error "Conflicting BLE library detected (possibly ArduinoBLE). Please remove it to proceed."
#endif

// UUIDs for BLE services and characteristics
#define UART_SERVICE_UUID      "00000001-0000-FEED-0000-000000000000"
#define RX_CHARACTERISTIC_UUID "00000002-0000-FEED-0000-000000000000"
#define TX_CHARACTERISTIC_UUID "00000003-0000-FEED-0000-000000000000"

#define UPDATE_SERVICE_UUID       "00000001-0000-C0DE-0000-000000000000"
#define UPDATE_CHARACTERISTIC_UUID "00000002-0000-C0DE-0000-000000000000"

// BLE server/characteristic pointers and connection state
BLEServer *pServer = NULL;
BLECharacteristic *pRxCharacteristic = NULL;
BLECharacteristic *pTxCharacteristic = NULL;
BLECharacteristic *pUpdateCharacteristic = NULL;
bool deviceConnected = false;

// OTA update variables
bool otaInProgress = false;
uint32_t otaFileSize = 0;
uint32_t otaReceived = 0;
int lastProgressPercent = -1;

// Counter and timing variables
int counter = 0;
unsigned long previousMillis = 0;
const long interval = 1000;

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("[BLE] Device connected.");
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    // Cancel OTA update on disconnect
    if (otaInProgress) {
      Serial.println("[OTA] Update cancelled.");
      Update.end(false); // Abort the update process
      otaInProgress = false;
    }
    // Restart advertising after disconnecting
    pServer->getAdvertising()->start();
    Serial.println("[BLE] Device disconnected. Re-advertising...");
  }
};

class UartCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    uint8_t* data = pCharacteristic->getData();
    size_t length = pCharacteristic->getLength();

    if (length == 0) return;

    // Handle regular UART input
    Serial.print("[Console] ");
    Serial.write(data, length);
    Serial.println();
  }
};

class UpdateCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    uint8_t* data = pCharacteristic->getData();
    size_t length = pCharacteristic->getLength();

    if (length == 0) return;

    // Check for OTA start command "OPEN"
    if (!otaInProgress && length == 4 &&
        memcmp(data, "OPEN", 4) == 0) {
      Serial.println("[OTA] Update started.");
      otaInProgress = true;
      otaFileSize = 0;
      otaReceived = 0;
      lastProgressPercent = -1;
      return;
    }

    // Check for OTA cancel command "HALT" during an OTA update
    if (otaInProgress && length == 4 &&
        memcmp(data, "HALT", 4) == 0) {
      Serial.println("[OTA] Update cancelled.");
      Update.end(false); // Abort the update process
      otaInProgress = false;
      return;
    }

    if (otaInProgress) {
      // Handle file size reception
      if (otaFileSize == 0 && length == 4) {
        memcpy(&otaFileSize, data, 4);
        Serial.printf("[OTA] Update size: %u bytes.\n", otaFileSize);
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
          Serial.println("[OTA] ERROR: Unable to start update.");
          otaInProgress = false;
        }
        return;
      }

      // Check for OTA end command "DONE"
      if (length == 4 && memcmp(data, "DONE", 4) == 0) {
        Serial.println("[OTA] Finalizing update.");

        if (otaReceived != otaFileSize) {
          Serial.printf("[OTA] ERROR: Size mismatch (%u/%u bytes).\n", otaReceived, otaFileSize);
          Update.end(false); // Abort safely
        } else if (Update.end(true) && Update.isFinished()) {
          Serial.println("[OTA] Update successful.");
          Serial.println("[OTA] Rebooting...");
          ESP.restart();
        } else {
          Serial.println("[OTA] ERROR: Failed to finalize update.");
          Update.printError(Serial);
        }

        otaInProgress = false;
        return;
      }

      // Process binary data chunks
      if (otaReceived < otaFileSize) {
        size_t written = Update.write(data, length);
        if (written > 0) {
          otaReceived += written;
          int progress = (otaReceived * 100) / otaFileSize;
          // Print only if progress has advanced to a new integer value
          if (progress != lastProgressPercent) {
            lastProgressPercent = progress;
            Serial.printf("[OTA] Progress: %d%%\n", progress);
          }
        }
      }
    }
  }
};

void setup() {
  Serial.begin(115200);
  BLEDevice::init("Wible");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create UART Service
  BLEService *pUartService = pServer->createService(UART_SERVICE_UUID);

  // Create RX Characteristic (write_nr)
  pRxCharacteristic = pUartService->createCharacteristic(
    RX_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_WRITE_NR
  );
  pRxCharacteristic->setCallbacks(new UartCallbacks());

  // Create TX Characteristic (read, notify)
  pTxCharacteristic = pUartService->createCharacteristic(
    TX_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_NOTIFY
  );

  // Create Update Service
  BLEService *pUpdateService = pServer->createService(UPDATE_SERVICE_UUID);

  // Create Update Characteristic (write_nr)
  pUpdateCharacteristic = pUpdateService->createCharacteristic(
    UPDATE_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_WRITE_NR
  );
  pUpdateCharacteristic->setCallbacks(new UpdateCallbacks());

  // Start both services
  pUartService->start();
  pUpdateService->start();

  // Start advertising with both service UUIDs
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(UART_SERVICE_UUID);
  pAdvertising->addServiceUUID(UPDATE_SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  pAdvertising->setName("Wible");
  BLEDevice::startAdvertising();

  Serial.println("[BLE] Device initialized. Advertising...");
}

void loop() {
  if (deviceConnected && !otaInProgress) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      String counterString = "Counter: " + String(counter++) + "\n";
      pTxCharacteristic->setValue(counterString.c_str());
      pTxCharacteristic->notify();
    }
  }
}
