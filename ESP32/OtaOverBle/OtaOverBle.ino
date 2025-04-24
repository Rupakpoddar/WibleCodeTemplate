/*
  This base sketch enables OTA firmware updates via BLE on the ESP32.
  Flash this code first before attempting any over-the-air updates.

  To update the firmware:
  1. Make your code changes in this template or integrate it into your project.
  2. In Arduino IDE, go to Sketch --> Export Compiled Binary.
  3. A 'build' folder will be created in your project directory.
  4. Open the subfolder named after your selected MCU.
  5. Locate the file with the '.ino.bin' extension and move it to your phone.

  Updating via Wible App:
  1. Open the Wible app and choose OTA Update.
  2. Connect to your ESP32 device via BLE.
  3. Select the '.ino.bin' file you placed on your phone.
  4. Start the update process.

  Once the transfer is complete, the ESP32 will automatically reboot into the new firmware.
*/

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Update.h>

#ifdef _BLE_DEVICE_H_
  #error "Conflicting BLE library detected (possibly ArduinoBLE). Please remove it to proceed."
#endif

BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
bool deviceConnected = false;

// OTA Update Variables
bool otaInProgress = false;
uint32_t otaFileSize = 0;
uint32_t otaReceived = 0;
int lastProgressPercent = -1;

// UUIDs for BLE Service and Characteristic
#define SERVICE_UUID           "66443771-D481-49B0-BE32-8CE24AC0F09C"
#define CHARACTERISTIC_UUID    "66443772-D481-49B0-BE32-8CE24AC0F09C"

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
    otaInProgress = false;
    // Restart advertising after disconnecting
    pServer->getAdvertising()->start();
    Serial.println("[BLE] Device disconnected. Re-advertising...");
  }
};

class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    uint8_t* data = pCharacteristic->getData();
    size_t length = pCharacteristic->getLength();

    if (length == 0) return;

    // Check for OTA start command "OPEN"
    if (!otaInProgress && length == 4 && 
        memcmp(data, "\x4F\x50\x45\x4E", 4) == 0) {
      Serial.println("[OTA] Update started.");
      otaInProgress = true;
      otaFileSize = 0;
      otaReceived = 0;
      lastProgressPercent = -1;
      return;
    }

    // Check for OTA cancel command "HALT" during an OTA update
    if (otaInProgress && length == 4 && 
        memcmp(data, "\x48\x41\x4C\x54", 4) == 0) {
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
      if (length == 4 && memcmp(data, "\x44\x4F\x4E\x45", 4) == 0) {
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
    } else {
      // Handle regular input
      Serial.print("[Console] ");
      Serial.write(data, length);
      Serial.println();
    }
  }
};

void setup() {
  Serial.begin(115200);
  BLEDevice::init("Wible");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE |
    BLECharacteristic::PROPERTY_WRITE_NR |
    BLECharacteristic::PROPERTY_NOTIFY
  );

  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setCallbacks(new MyCallbacks());
  pService->start();
  BLEDevice::startAdvertising();
  Serial.println("[BLE] Device initialized. Advertising...");
}

void loop() {
  if (deviceConnected && !otaInProgress) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      String counterString = "Counter: " + String(counter++) + "\n";
      pCharacteristic->setValue(counterString.c_str());
      pCharacteristic->notify();
    }
  }
}
