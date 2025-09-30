/*
  * Developer: Rupak Poddar
  * Wible Code Template for HM10 BLE Module
  * Steer Freely
*/

/* 
  Note: A known drawback of the HM-10 BLE module is its inherent latency in data transmission.
  Consider using alternatives like Raspberry Pi Pico W, ESP32, or Arduino Uno R4 WiFi for lower latency.
  The current code handles this delay in the best possible way by introducing the PRECISIONDELAY macro.
*/

#include <SoftwareSerial.h>

#define PRECISIONDELAY 100

/*
  * Connect HM10 TX pin to Arduino pin 2
  * Connect HM10 RX pin to Arduino pin 3
*/
SoftwareSerial HM10(2, 3);

/*
  * Set the following variable to false to disable print statements
*/
bool verbose = true;

/*
  * Utilize PWM pins for motor drivers
  * Connect motor driver 1 to Arduino pin 5, 6
  * Connect motor driver 2 to Arduino pin 10, 11
*/
#define M1A 5
#define M1B 6
#define M2A 10
#define M2B 11

/*
  * Initialize the robot with the following values
*/
String receivedString = "";
unsigned char speed = 100;
String command = "STOP";
bool ignitionState = false;

void setup() {
  if (verbose) {
    Serial.begin(9600);
    delay(100);
    Serial.println("\n\nWible HM10 Steer Freely\n");
  }

  HM10.begin(9600);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(M1A, OUTPUT);
  pinMode(M1B, OUTPUT);
  pinMode(M2A, OUTPUT);
  pinMode(M2B, OUTPUT);

  _STOP();
}

void loop() {
  if (HM10.available() > 0) {
    receivedString = HM10.readString();

    if (verbose) {
      Serial.print("----- Received String: ");
      Serial.print(receivedString);
      Serial.print(" -----\n\n");
    }

    unsigned char commandStart = 0;
    while (commandStart < receivedString.length()) {
      String currentCommand = receivedString.substring(commandStart, commandStart + 7);

      /*
        * Ignore the connection response from the module
      */
      if (currentCommand.substring(0, 3) != "OK+")
        speed = currentCommand.substring(0, 3).toInt();
      
      command = currentCommand.substring(3, 7);

      if (verbose) {
        Serial.print("Speed: ");
        Serial.println(speed);
        Serial.print("Command: ");
        Serial.print(command);
        Serial.print("\n\n");
      }

      /*
        * Handle Bluetooth connection lost event
      */
      if (command == "LOST") {
        ignitionState = false;
        _STOP();

        if (verbose) {
          Serial.println("+------------------------+");
          Serial.println("|                        |");
          Serial.println("|    CONNECTION LOST     |");
          Serial.println("|                        |");
          Serial.println("+------------------------+");
          Serial.print("\n\n");
        }
      }

      if (command == "IGON") {
        ignitionState = true;
      }

      if (command == "IGOF") {
        ignitionState = false;
        _STOP();
      }

      if (command == "STOP") {
        _STOP();
      }

      if (command == "LTON") {
        digitalWrite(LED_BUILTIN, HIGH);
      }

      if (command == "LTOF") {
        digitalWrite(LED_BUILTIN, LOW);
      }

      /*
        * Utilize the Auxiliary buttons from below
      */
      // if (command == "AUX1"){}
      // if (command == "AUX2"){}
      // if (command == "AUX3"){}
      // if (command == "AUX4"){}
      // if (command == "AUX5"){}
      // if (command == "AUX6"){}

      /*
        * Uncomment the following line to disable ignition button
      */
      // ignitionState = true;

      if (ignitionState) {
        /*
          * FORWARD
        */
        if (command == "FWRD") {
          analogWrite(M1A, speed);
          analogWrite(M1B, 0);
          analogWrite(M2A, speed);
          analogWrite(M2B, 0);
        }

        /*
          * BACKWARD
        */
        if (command == "BKWD") {
          analogWrite(M1A, 0);
          analogWrite(M1B, speed);
          analogWrite(M2A, 0);
          analogWrite(M2B, speed);
        }

        /*
          * LEFT
        */
        if (command == "LEFT") {
          analogWrite(M1A, 0);
          analogWrite(M1B, speed);
          analogWrite(M2A, speed);
          analogWrite(M2B, 0);
        }

        /*
          * RIGHT
        */
        if (command == "RGHT") {
          analogWrite(M1A, speed);
          analogWrite(M1B, 0);
          analogWrite(M2A, 0);
          analogWrite(M2B, speed);
        }

        /*
          * Utilize the multi-touch capability of the app from below
        */
        // if (command == "FRLT") {} // FORWARD + LEFT
        // if (command == "FRRT") {} // FORWARD + RIGHT
        // if (command == "BKLT") {} // BACKWARD + LEFT
        // if (command == "BKRT") {} // BACKWARD + RIGHT
      }

      commandStart += 7;
      delay(PRECISIONDELAY);
    }
    if (!ignitionState) {
      if (verbose) {
        Serial.println("+------------------------+");
        Serial.println("|                        |");
        Serial.println("|   IGNITION STATE OFF   |");
        Serial.println("|                        |");
        Serial.println("+------------------------+");
        Serial.println("\n\n");
      }
    }
  }
}

void _STOP() {
  analogWrite(M1A, 0);
  analogWrite(M1B, 0);
  analogWrite(M2A, 0);
  analogWrite(M2B, 0);
}
