/*
  * Project Information:
    - Developer: Rupak Poddar
    - Wible Code Template for Arduino Uno R4 WiFi
    - Wireless HID Example
  
  * NOTICE: Update WiFi Credentials
    - Before uploading this sketch, open "secrets.h" 
      and update the WiFi SSID and password with your network details.
    - After a successful connection, the board's IP address will be 
      displayed on the built-in LED matrix. The default UDP port is 4210.

  * Instructions:
    - After the board connects to the internet, open the Wible app on your smartphone. 
    - Then select "Wireless HID" and enter the board's IP address and UDP port in the dialog box.
    - Select "Connect". Make sure the IP address is accessible from your smartphone.
  
  * To Reprogram The Board:
    - Double-press the reset button until the built-in LED starts fading in and out.
    - Reselect the port in the Arduino IDE.
    - Upload the new code.

  * Required Library:
    - ArduinoGraphics by Arduino (install via Arduino IDE Library Manager)
      https://docs.arduino.cc/libraries/arduinographics/
      or https://github.com/arduino-libraries/ArduinoGraphics
*/

#include "Mouse.h"
#include <WiFiS3.h>
#include <WiFiUdp.h>
#include "secrets.h"
#include "ArduinoGraphics.h"
#include "keyboard_helper.hpp"
#include "Arduino_LED_Matrix.h"

// Option to enable/disable verbose mode
bool verbose = false;

// UDP Setup
WiFiUDP udp;                // UDP server instance
const int udpPort = 4210;   // Port to listen for incoming packets
uint8_t incomingPacket[2];  // Buffer for incoming packets (2 bytes)

// Built-in LED matrix to display the IP address
ArduinoLEDMatrix matrix;

// ConsumerKeyboard class instance for media control 
ConsumerKeyboard mediaControl;

void setup() {
  if (verbose) {
    // Start Serial communication
    Serial.begin(115200);
    delay(1000);
  }

  // Connect to Wi-Fi
  WiFi.disconnect();
  delay(1000);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    if (verbose) {
      delay(500);
      Serial.print(".");
    }
  }

  // Wait for a valid IP via DHCP
  IPAddress ip;
  do {
    ip = WiFi.localIP();
    delay(100);
  } while (ip == IPAddress(0, 0, 0, 0));

  if (verbose) {
    Serial.println("\nWiFi connected.");
    Serial.print("Arduino IP address: ");
    Serial.println(ip);
  }

  // Start UDP server
  udp.begin(udpPort);
  if (verbose) {
    Serial.print("Listening on UDP port: ");
    Serial.println(udpPort);
    Serial.println();
  }

  // Display the IP address on LED Matrix
  matrix.begin();
  matrix.beginDraw();
  matrix.stroke(0xFFFFFFFF);
  matrix.textScrollSpeed(50);
  String paddedLocalIP = "    " + ip.toString() + "    ";
  matrix.textFont(Font_5x7);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.println(paddedLocalIP);
  matrix.endText(SCROLL_LEFT);
  matrix.endDraw();

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
            mediaControl.press(consumerTable[x % 75]);
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
            mediaControl.release();
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
        mediaControl.release();
        if (verbose) {
          Serial.println("KEYBOARD_RELEASE_ALL");
        }
      }
    }
  }
}
