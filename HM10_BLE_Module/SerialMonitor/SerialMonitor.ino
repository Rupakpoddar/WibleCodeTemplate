/*
  * Project Information:
    - Developer: Rupak Poddar
    - Wible Code Template for HM10 BLE Module
    - Tested on: Arduino Uno R3
    - Serial Monitor Example

  * Instructions:
    - After flashing this code, open the Wible app on your smartphone.
    - Then select "Serial Monitor" and look for the HM10 device listing, usually called "HMSoft".

  * NOTE:
    - A known drawback of the HM-10 BLE module is its inherent latency in data transmission.
    - Consider using alternatives like Raspberry Pi Pico W, ESP32, or Arduino Uno R4 WiFi for lower latency.
*/

#include <SoftwareSerial.h>

// Connect HM10 TX pin to Arduino pin 2
// Connect HM10 RX pin to Arduino pin 3
SoftwareSerial HM10(2, 3);

// Initialize counter
int counter = 0;

// Implement delay without blocking
unsigned long previousMillis = 0; // Stores the last time the event occurred
const long interval = 1000; // Interval for the delay in milliseconds

void setup() {
  Serial.begin(9600);
  delay(100);
  Serial.println("\n\nWible HM10 Serial Monitor\n");

  HM10.begin(9600);
}

void loop() {
  // Check if there is any data available from HM-10
  if (HM10.available() > 0) {
    // Read the incoming data from HM-10
    String incomingData = HM10.readString();

    // Print the received data to the Serial Monitor
    Serial.print("Received from HM-10: ");
    Serial.println(incomingData);
  }

  unsigned long currentMillis = millis(); // Get the current time

  // Check if the interval has passed
  if (currentMillis - previousMillis >= interval) {
    // Save the last time the event occurred
    previousMillis = currentMillis;

    // Write the counter value to HM10 
    HM10.println("Counter value: " + String(counter));
    
    // Increment the counter
    counter++;
  }
}
