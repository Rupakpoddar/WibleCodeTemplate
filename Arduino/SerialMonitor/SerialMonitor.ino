/*
  * Developer: Rupak Poddar
  * Wible Code Template for Arduino
  * Serial Monitor
*/

#include <ArduinoBLE.h>

#ifndef _WIBLE_ENABLED_
  #error "Please install the modified ArduinoBLE library from: https://github.com/Rupakpoddar/WibleCodeTemplate/raw/master/Arduino/ArduinoBLE.zip"
#endif

// Create BLE service & characteristic to allow remote device to read, write, and notify
BLEService TX_RX_Service("19B10010-E8F2-537E-4F6C-D104768A1214");
BLECharacteristic TX_RX_Characteristic("19B10011-E8F2-537E-4F6C-D104768A1214", 
                                        BLERead | BLEWriteWithoutResponse | BLENotify, 20);

// Initialize counter and timing variables
int counter = 0;
unsigned long previousMillis = 0; // last time the event occurred
const long interval = 1000; // interval in milliseconds

// Flag to detect changes in connection status
bool deviceConnected = false;

void setup() {
  Serial.begin(9600);
  delay(100);
  Serial.println("\n\nWible Arduino Serial Monitor\n");

  pinMode(LED_BUILTIN, OUTPUT);

  // Initialize BLE module
  if (!BLE.begin()) {
    Serial.println("BLE initialization failed!");
    while (1);
  }

  // Configure BLE device parameters
  BLE.setLocalName("Wible");
  BLE.setDeviceName("Wible");
  BLE.setAdvertisedService(TX_RX_Service);

  // Add the characteristic to the service and add the service
  TX_RX_Service.addCharacteristic(TX_RX_Characteristic);
  BLE.addService(TX_RX_Service);

  // Start advertising BLE service
  BLE.advertise();
  Serial.println("Device initialized. Advertising...");
}

void loop() {
  // Get the central device object, if any central is connected it will be non-null
  BLEDevice central = BLE.central();

  if (central) {
    // A central is connected now. Print connection message once.
    if (!deviceConnected) {
      Serial.println("Device connected.");
      deviceConnected = true;
    }

    // If new data is written by the central, print it out.
    if (TX_RX_Characteristic.written()) {
      const uint8_t* value = TX_RX_Characteristic.value();
      String incomingData = String((char*)value);
      Serial.print("Received: ");
      Serial.println(incomingData);
    }

    // Check if it is time to send a counter update to the central.
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      String msg = "Counter value: " + String(counter) + "\n";
      // Write counter message to the central
      TX_RX_Characteristic.writeValue(msg.c_str());
      counter++;
    }
    // Optionally, signal connection via LED (steady ON)
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    // No central connected. If we were previously connected, note the disconnect and restart advertising.
    if (deviceConnected) {
      Serial.println("Device disconnected. Re-advertising...");
      deviceConnected = false;
      BLE.advertise();
    }
    // Blink LED while waiting for connection
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
  }
}
