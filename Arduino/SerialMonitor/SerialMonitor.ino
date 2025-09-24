/*
  * Developer: Rupak Poddar
  * Wible Code Template for Arduino
  * Serial Monitor
*/

#include <ArduinoBLE.h>

// Create UART BLE service with separate RX and TX characteristics
BLEService uartService("00000001-0000-FEED-0000-000000000000");
BLECharacteristic rxCharacteristic("00000002-0000-FEED-0000-000000000000",
                                   BLEWriteWithoutResponse, 96); // BLEWriteWithoutResponse (No ACK --> faster) can be replaced with BLEWrite 
BLECharacteristic txCharacteristic("00000003-0000-FEED-0000-000000000000",
                                   BLERead | BLENotify, 96); // BLENotify (No ACK --> faster) can be replaced with BLEIndicate

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
  BLE.setAdvertisedService(uartService);

  // Add the characteristics to the service and add the service
  uartService.addCharacteristic(rxCharacteristic);
  uartService.addCharacteristic(txCharacteristic);
  BLE.addService(uartService);

  // Start advertising BLE service
  BLE.advertise();
  Serial.println("Device initialized. Advertising...");
}

void loop() {
  // Poll for BLE events
  BLE.poll();
  
  // Get the central device object, if any central is connected it will be non-null
  BLEDevice central = BLE.central();

  if (central) {
    // A central is connected now. Print connection message once.
    if (!deviceConnected) {
      Serial.println("Device connected.");
      deviceConnected = true;
    }

    // If new data is written to RX characteristic, print it out.
    if (rxCharacteristic.written()) {
      const uint8_t* value = rxCharacteristic.value();
      String incomingData = String((char*)value);
      Serial.print("Received: ");
      Serial.println(incomingData);
    }

    // Check if it is time to send a counter update to the central.
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      String msg = "Counter value: " + String(counter) + "\n";
      // Write counter message to TX characteristic
      txCharacteristic.writeValue(msg.c_str());
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
