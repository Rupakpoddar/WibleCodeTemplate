#include <WiFiS3.h>
#include <WiFiUdp.h>
#include "Mouse.h"

// Wi-Fi Credentials
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASS";

// UDP setup
WiFiUDP udp;            // UDP server instance
const int udpPort = 4210; // Port to listen for incoming packets
uint8_t incomingPacket[2]; // Buffer for incoming packets (2 bytes)

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
  if (packetSize == 2) { // Expecting exactly 2 bytes
    udp.read(incomingPacket, 2);

    uint8_t x = static_cast<int8_t>(incomingPacket[0]); // First byte
    uint8_t y = static_cast<int8_t>(incomingPacket[1]); // Second byte

    // Trackpad
    if (x <= 100 && y <= 100) {
      // Handle the offsets
      x -= 50;
      y -= 50;

      // Move the mouse based on x and y values
      Mouse.move(x, y, 0);

      if (verbose) {
        Serial.print("X = ");
        Serial.print(x);
        Serial.print(", Y = ");
        Serial.println(y);
      }
    }
    // Mouse Buttons
    else if (x > 100 && y < 100) {
      // Left Button
      if (x == (128+2)) {
        // Press
        if (y == 2) {
          Mouse.press(MOUSE_LEFT);
          if (verbose) {
            Serial.println("MOUSE_LEFT_PRESS");
          }
        }

        // Release
        if (y == 1) {
          Mouse.release(MOUSE_LEFT);
          if (verbose) {
            Serial.println("MOUSE_LEFT_RELEASE");
          }
        }

        // Click
        if (y == 4) {
          Mouse.click(MOUSE_LEFT);
          if (verbose) {
            Serial.println("MOUSE_LEFT_CLICK");
          }
        }
      }
      // Right Button
      else if (x == (128+1)) {
        // Press
        if (y == 2) {
          Mouse.press(MOUSE_RIGHT);
          if (verbose) {
            Serial.println("MOUSE_RIGHT_PRESS");
          }
        }

        // Release
        if (y == 1) {
          Mouse.release(MOUSE_RIGHT);
          if (verbose) {
            Serial.println("MOUSE_RIGHT_RELEASE");
          }
        }

        // Click
        if (y == 4) {
          Mouse.click(MOUSE_RIGHT);
          if (verbose) {
            Serial.println("MOUSE_RIGHT_CLICK");
          }
        }
      }
      // Scroll
      else if (x == (128+4)) {
        // Handle offset
        y -= 50;

        // Scroll based on y values
        Mouse.move(0, 0, y);
      }
    }
  }
}
