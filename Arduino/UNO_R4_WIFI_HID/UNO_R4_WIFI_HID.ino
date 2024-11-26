#include <WiFiS3.h>
#include <WiFiUdp.h>
#include "Mouse.h"

// Wi-Fi Credentials
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASS";

// UDP setup
WiFiUDP udp;            // UDP server instance
const int udpPort = 4210; // Port to listen for incoming packets
char incomingPacket[255]; // Buffer for incoming packets

// Option to enable/disable verbose mode
bool verbose = false; // Change this value to false to disable verbose logging

void setup() {
  if (verbose) {
    // Start Serial communication
    Serial.begin(115200);
    while (!Serial);
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (verbose) {
      Serial.print(".");
    }
  }
  if (verbose) {
    Serial.println("\nWiFi connected.");
    Serial.print("Arduino IP address: ");
    Serial.println(WiFi.localIP());
  }

  // Start UDP server
  udp.begin(udpPort);
  if (verbose) {
    Serial.print("Listening on UDP port ");
    Serial.println(udpPort);
  }

  // Start Mouse control
  Mouse.begin();
}

void loop() {
  // Check for incoming data
  int packetSize = udp.parsePacket();
  if (packetSize) {
    int len = udp.read(incomingPacket, 255);
    if (len > 0) {
      incomingPacket[len] = 0; // Null-terminate the packet
      String receivedData = String(incomingPacket);
      receivedData.trim(); // Removes leading and trailing spaces and newlines

      if (verbose) {
        Serial.print("Received: ");
        Serial.println(receivedData);
      }

      // Check for TAP command
      if (receivedData == "TAP") {
        Mouse.click(MOUSE_LEFT);
        if (verbose) {
          Serial.println("Mouse TAP Triggered");
        }
      } else {
        // Handle x:y movement data
        int x = 0, y = 0;
        if (parseMovement(receivedData, x, y)) {
          // Move the mouse based on x and y values
          Mouse.move(x, y, 0);
          if (verbose) {
            Serial.print("Mouse moved: X = ");
            Serial.print(x);
            Serial.print(", Y = ");
            Serial.println(y);
          }
        } else {
          if (verbose) {
            Serial.println("Invalid data format.");
          }
        }
      }
    }
  }
}

// Function to parse movement data (x:y) from the string
bool parseMovement(String data, int &x, int &y) {
  int colonIndex = data.indexOf(':');

  // Validate the format (must contain a colon)
  if (colonIndex == -1) {
    return false;
  }

  // Extract x and y values
  String xStr = data.substring(0, colonIndex);
  String yStr = data.substring(colonIndex + 1);

  // Convert to integers
  x = xStr.toInt();
  y = yStr.toInt();

  return true; // Parsing successful
}
