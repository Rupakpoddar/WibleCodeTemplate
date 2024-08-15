/*
  * Developer: Rupak Poddar
  * Wible Code Template for Arduino
  * Serial Monitor
*/

#include <ArduinoBLE.h>

#ifndef _WIBLE_ENABLED_
  #error "Please install the modified ArduinoBLE library from: linkToTheLibrary.com"
#endif

/*
  * Create BLE service & characteristic to allow remote device to read, write, and notify
*/
BLEService TX_RX_Service("19B10010-E8F2-537E-4F6C-D104768A1214");
BLECharacteristic TX_RX_Characteristic("19B10011-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWriteWithoutResponse | BLENotify, 20);

/*
  * Initialize counter
*/
int counter = 0;

/*
  * Implement delay without blocking
*/
unsigned long previousMillis = 0; // Stores the last time the event occurred
const long interval = 1000; // Interval for the delay in milliseconds

void setup() {
  Serial.begin(9600);
  delay(100);
  Serial.println("\n\nWible Arduino Serial Monitor\n");

  pinMode(LED_BUILTIN, OUTPUT);

  // Initialize BLE module
  if (!BLE.begin()) {
    Serial.println("BLE Initialization failed!");

    while (1);
  }

  // Set the local and device name this peripheral advertises (default: Arduino)
  BLE.setLocalName("Wible");
  BLE.setDeviceName("Wible");

  // Set the UUID for the service this peripheral advertises
  BLE.setAdvertisedService(TX_RX_Service);

  // Add the characteristics to the service
  TX_RX_Service.addCharacteristic(TX_RX_Characteristic);

  // Add the service
  BLE.addService(TX_RX_Service);

  // Start advertising
  BLE.advertise();

  Serial.println("BLE device active, waiting for connections...\n");
}

void loop() {
  /*
    * Poll for BLE events
  */
  BLE.poll();

  /*
    * Check if BLE device is connected
  */
  if (BLE.connected()) {
    /*
      * Check if there is any data available from BLE device
    */
    if (TX_RX_Characteristic.written()) {
      // Get the data as a pointer to uint8_t (byte array)
      const uint8_t* value = TX_RX_Characteristic.value();

      // Convert the byte array to a String
      String incomingData = String((char*)value);

      // Print the received data to the Serial Monitor
      Serial.print("Received: ");
      Serial.println(incomingData);
    }

    unsigned long currentMillis = millis(); // Get the current time

    /*
      * Check if the interval has passed
    */
    if (currentMillis - previousMillis >= interval) {
      // Save the last time the event occurred
      previousMillis = currentMillis;

      /*
        * Write the counter value to remote device
      */
      TX_RX_Characteristic.writeValue(("Counter value: " + String(counter) + "\n").c_str());
      
      // Increment the counter
      counter++;
    }
  } else {
    /*
      * Blink the builtin LED while the BLE device is waiting for connection
    */
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
  }
}
