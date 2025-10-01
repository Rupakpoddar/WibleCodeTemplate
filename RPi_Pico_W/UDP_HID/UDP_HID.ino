/*
  * Project Information:
    - Developer: Rupak Poddar
    - Wible Code Template for Raspberry Pi Pico W
    - Tested on: RP2040, RP2350 
    - Wireless HID Example
  
  * NOTICE: Update WiFi Credentials
    - Before uploading this sketch, open "secrets.h" 
      and update the WiFi SSID and password with your network details.
  
  * Board Setup:
    1. Add Raspberry Pi Pico board definitions to the Arduino IDE:
      - Open Preferences in the Arduino IDE.
      - Paste the following URL into the "Additional Board Manager URLs":
        https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
      - Open the Boards Manager and install the "Raspberry Pi Pico" package by Earle F. Philhower.
      
    2. Programming the Pico W:
      - Hold the BOOTSEL button while connecting the Raspberry Pi Pico W to your computer.
      - In the Arduino IDE, select "Raspberry Pi Pico W" as the board.
      - Select "UF2_Board" for the port.
      - Upload the code.

    3. Post-Upload Steps:
      - Reselect the COM Port in the Arduino IDE after uploading the code.
      - Open the Serial Monitor at 115200 baud to view the board's IP address after a successful WiFi connection.
      - The default UDP port is 4210.

  * Instructions:
    - After the board connects to the internet, open the Wible app on your smartphone. 
    - Then select "Wireless HID" and enter the board's IP address and UDP port in the dialog box.
    - Select "Connect". Make sure the IP address is accessible from your smartphone.
*/

#include "Mouse.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include "secrets.h"
#include "keyboard_helper.hpp"

// Option to enable/disable verbose mode
bool verbose = true;

// UDP Setup
WiFiUDP udp;                // UDP server instance
const int udpPort = 4210;   // Port to listen for incoming packets
uint8_t incomingPacket[2];  // Buffer for incoming packets (2 bytes)

void setup() {
  if (verbose) {
    // Start Serial communication
    Serial.begin(115200);
    delay(1000);
  }

  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(1000);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    if (verbose) {
      delay(500);
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
    Serial.print("Listening on UDP port: ");
    Serial.println(udpPort);
    Serial.println();
  }

  // Start Mouse Control
  Mouse.begin();

  // Start Keyboard Control
  Keyboard.begin();
}

void loop() {
  // Check for incoming data
  int packetSize = udp.parsePacket();
  if (packetSize == 2) { // Expecting exactly 2 bytes
    udp.read(incomingPacket, 2);

    int16_t x = incomingPacket[0]; // First byte
    int16_t y = incomingPacket[1]; // Second byte

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
        else if (y == 1) {
          Mouse.release(MOUSE_LEFT);
          if (verbose) {
            Serial.println("MOUSE_LEFT_RELEASE");
          }
        }
        // Click
        else if (y == 4) {
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
        else if (y == 1) {
          Mouse.release(MOUSE_RIGHT);
          if (verbose) {
            Serial.println("MOUSE_RIGHT_RELEASE");
          }
        }
        // Click
        else if (y == 4) {
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
        if (verbose) {
          Serial.print("SCROLL = ");
          Serial.println(y);
        }
      }
    }

    // Keyboard
    else if (y > 100) {
      // Key Press
      if (y == (128+2)) {
        if (x != 0) {
          if (x < 75) {
            Keyboard.press(switchTable[x]);
          } else {
            Keyboard.consumerPress(consumerTable[x % 75]);
          }
          if (verbose) {
            Serial.print("PRESS SWITCH ID = ");
            Serial.println(x);
          }
        }
      }
      // Release
      else if (y == (128+1)) {
        if (x != 0) {
          if (x < 75) {
            Keyboard.release(switchTable[x]);
          } else {
            Keyboard.consumerRelease();
          }
          if (verbose) {
            Serial.print("RELEASE SWITCH ID = ");
            Serial.println(x);
          }
        }
      }
      // Release All
      else if (y == (128+4)) {
        Keyboard.releaseAll();
        Keyboard.consumerRelease();
        if (verbose) {
          Serial.println("KEYBOARD_RELEASE_ALL");
        }
      }
    }
  }
}
