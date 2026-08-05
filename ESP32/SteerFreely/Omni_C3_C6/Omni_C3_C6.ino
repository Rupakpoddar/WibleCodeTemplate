/*
  * Project Information:
    - Developer: Rupak Poddar
    - Wible Code Template for ESP32
    - Tested on: ESP32-C3, ESP32-C6
    - Steer Freely Omnidirectional & Motion Control Example

  * Instructions:
    - After flashing this code, open the Wible app on your smartphone.
    - Then select "Steer Freely" and look for the listing called "Wible".
*/

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#ifdef _BLE_DEVICE_H_
  #error "Conflicting BLE library detected (possibly ArduinoBLE). Please remove it to proceed."
#endif

// UUIDs for the BLE service and characteristics
#define UART_SERVICE_UUID      "00000001-0000-FEED-0000-000000000000"
#define RX_CHARACTERISTIC_UUID "00000002-0000-FEED-0000-000000000000"
#define TX_CHARACTERISTIC_UUID "00000003-0000-FEED-0000-000000000000"

// Set verbose flag to enable print statements
#define VERBOSE true

// Motor speed limits and motion thresholds
#define MIN_SPEED 80
#define MAX_SPEED 255
#define MOTION_TIMEOUT_MS 250
#define MOTION_X_UPPER_BOUND (128+32)
#define MOTION_X_LOWER_BOUND (128-32)
#define MOTION_Y_UPPER_BOUND (128+32)
#define MOTION_Y_LOWER_BOUND (128-32)

//                    FRONT 
//                      ↑ 
//            M1 ○ ┌─────────┐ ○ M2 
//                 │  ROBOT  │ 
//            M4 ○ └─────────┘ ○ M3 
//                      ↓ 
//                    BACK 
// 
//          A = Forward | B = Backward 

// Motor driver and light GPIOs
#define M1A 18
#define M1B 19
#define M2A 20
#define M2B 21
#define M3A 2
#define M3B 3
#define M4A 6
#define M4B 7
#define LED 22

/*
  * ESP32-C3 and ESP32-C6 only provide 6 LEDC (PWM) channels, but this robot
    has 8 motor pins. Driving all 8 with analogWrite() silently fails on the
    last two pins. Instead, each motor owns a single LEDC channel that is moved
    between its A and B pin whenever the direction changes, since only one side
    of an H-bridge is ever driven at a time. The idle side is held LOW digitally.
*/
#define PWM_FREQ 1000
#define PWM_RESOLUTION 8

// Motor indices
#define M1 0
#define M2 1
#define M3 2
#define M4 3

struct Motor {
  uint8_t pinA;
  uint8_t pinB;
  uint8_t channel;
  int8_t driven;  // -1 = coasting, 0 = pinA attached, 1 = pinB attached
};

Motor motors[4] = {
  {M1A, M1B, 0, -1},
  {M2A, M2B, 1, -1},
  {M3A, M3B, 2, -1},
  {M4A, M4B, 3, -1}
};

// Forward declarations
void setMotor(uint8_t index, uint8_t speedA, uint8_t speedB);
void stopMotors();

// BLE server/characteristic pointers
BLEServer *pServer = NULL;
BLECharacteristic *pRxCharacteristic = NULL;
BLECharacteristic *pTxCharacteristic = NULL;

// Global variables
volatile bool deviceConnected = false;
volatile bool motionActive = false;
volatile unsigned long lastMotionCommandTime = 0;

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    if (VERBOSE) {
      Serial.println("Device connected.");
    }
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    // Restart advertising after disconnecting
    pServer->getAdvertising()->start();
    if (VERBOSE) {
      Serial.println("Device disconnected. Re-advertising...");
    }
  }
};

class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String rxValue = pCharacteristic->getValue();
    if (rxValue.length() > 0) {
      if (VERBOSE) {
        Serial.print("\n----- Received string: ");
        Serial.print(rxValue);
        Serial.println(" -----\n");
      }

      unsigned char speed = 0;
      String command = "";
      
      if (rxValue.length() == 7) {  // Button input
        // Parse rxValue in {SSS}{CCCC} format: SSS = speed, CCCC = command.
        speed = rxValue.substring(0, 3).toInt();
        command = rxValue.substring(3, 7);

        if (speed != 0) {
          speed = map(speed, 1, 100, MIN_SPEED, MAX_SPEED);
        }
      }
      else if (rxValue.length() == 8) { // Motion input
        lastMotionCommandTime = millis();
        motionActive = true;

        // Parse rxValue in X{AAA}Y{BBB} format: AAA = X-axis position from 0 to 255, BBB = Y-axis position from 0 to 255.
        unsigned char xValue = rxValue.substring(1, 4).toInt();
        unsigned char yValue = rxValue.substring(5, 8).toInt();

        // Phone position: Neutral
        if ((xValue <= MOTION_X_UPPER_BOUND)
            && (xValue >= MOTION_X_LOWER_BOUND)
            && (yValue <= MOTION_Y_UPPER_BOUND)
            && (yValue >= MOTION_Y_LOWER_BOUND)) { 
          stopMotors();
          return;
        }

        // Phone position: Forward
        if ((xValue > MOTION_X_UPPER_BOUND)
            && (yValue <= MOTION_Y_UPPER_BOUND)
            && (yValue >= MOTION_Y_LOWER_BOUND)) { 
          speed = map(xValue, MOTION_X_UPPER_BOUND, 255, MIN_SPEED, MAX_SPEED);
          command = "FWRD";
        }

        // Phone position: Backward
        if ((xValue < MOTION_X_LOWER_BOUND)
            && (yValue <= MOTION_Y_UPPER_BOUND)
            && (yValue >= MOTION_Y_LOWER_BOUND)) { 
          speed = map(MOTION_X_LOWER_BOUND-xValue, 0, MOTION_X_LOWER_BOUND, MIN_SPEED, MAX_SPEED);
          command = "BKWD";
        }

        // Phone position: Left
        if ((xValue <= MOTION_X_UPPER_BOUND)
            && (xValue >= MOTION_X_LOWER_BOUND)
            && (yValue < MOTION_Y_LOWER_BOUND)) { 
          speed = map(MOTION_Y_LOWER_BOUND-yValue, 0, MOTION_Y_LOWER_BOUND, MIN_SPEED, MAX_SPEED);
          command = "ROLT";
        }

        // Phone position: Right
        if ((xValue <= MOTION_X_UPPER_BOUND)
            && (xValue >= MOTION_X_LOWER_BOUND)
            && (yValue > MOTION_Y_UPPER_BOUND)) { 
          speed = map(yValue, MOTION_Y_UPPER_BOUND, 255, MIN_SPEED, MAX_SPEED);
          command = "RORT";
        }

        // Phone position: Front left
        if ((xValue > MOTION_X_UPPER_BOUND)
            && (yValue < MOTION_Y_LOWER_BOUND)) {
          speed = map(xValue, MOTION_X_UPPER_BOUND, 255, MIN_SPEED, MAX_SPEED);
          command = "FRRL";
        }

        // Phone position: Front right
        if ((xValue > MOTION_X_UPPER_BOUND)
            && (yValue > MOTION_Y_UPPER_BOUND)) {
          speed = map(xValue, MOTION_X_UPPER_BOUND, 255, MIN_SPEED, MAX_SPEED);
          command = "FRRR";
        }

        // Phone position: Back left
        if ((xValue < MOTION_X_LOWER_BOUND)
            && (yValue < MOTION_Y_LOWER_BOUND)) {
          speed = map(MOTION_X_LOWER_BOUND-xValue, 0, MOTION_X_LOWER_BOUND, MIN_SPEED, MAX_SPEED);
          command = "BKRL";
        }

        // Phone position: Back right
        if ((xValue < MOTION_X_LOWER_BOUND)
            && (yValue > MOTION_Y_UPPER_BOUND)) {
          speed = map(MOTION_X_LOWER_BOUND-xValue, 0, MOTION_X_LOWER_BOUND, MIN_SPEED, MAX_SPEED);
          command = "BKRR";
        }
      }
      else {
        stopMotors();
        if (VERBOSE) {
          Serial.println("Invalid input");
        }
        return;
      }

      if (VERBOSE) {
        Serial.print("Speed: ");
        Serial.println(speed);
        Serial.print("Command: ");
        Serial.println(command);
      }

      // STOP
      if (command == "STOP") {
        stopMotors();
      }

      // LED ON
      if (command == "LTON") {
        digitalWrite(LED, HIGH);
      }

      // LED OFF
      if (command == "LTOF") {
        digitalWrite(LED, LOW);
      }

      // FORWARD
      if (command == "FWRD") {
        setMotor(M1, speed, 0);
        setMotor(M2, speed, 0);
        setMotor(M3, speed, 0);
        setMotor(M4, speed, 0);
      }

      // BACKWARD
      if (command == "BKWD") {
        setMotor(M1, 0, speed);
        setMotor(M2, 0, speed);
        setMotor(M3, 0, speed);
        setMotor(M4, 0, speed);
      }

      // LEFT
      if (command == "LEFT") {
        setMotor(M1, 0, speed);
        setMotor(M2, speed, 0);
        setMotor(M3, 0, speed);
        setMotor(M4, speed, 0);
      }

      // RIGHT
      if (command == "RGHT") {
        setMotor(M1, speed, 0);
        setMotor(M2, 0, speed);
        setMotor(M3, speed, 0);
        setMotor(M4, 0, speed);
      }

      /*
        * Utilize additional control buttons and multi-touch commands from below
      */
      // ROTATE LEFT
      if (command == "ROLT") {
        setMotor(M1, 0, speed);
        setMotor(M2, speed, 0);
        setMotor(M3, speed, 0);
        setMotor(M4, 0, speed);
      }

      // ROTATE RIGHT
      if (command == "RORT") {
        setMotor(M1, speed, 0);
        setMotor(M2, 0, speed);
        setMotor(M3, 0, speed);
        setMotor(M4, speed, 0);
      }

      // FORWARD + LEFT
      if (command == "FRLT") {
        setMotor(M1, 0, 0);
        setMotor(M2, speed, 0);
        setMotor(M3, 0, 0);
        setMotor(M4, speed, 0);
      }

      // FORWARD + RIGHT
      if (command == "FRRT") {
        setMotor(M1, speed, 0);
        setMotor(M2, 0, 0);
        setMotor(M3, speed, 0);
        setMotor(M4, 0, 0);
      }

      // FORWARD + ROTATE LEFT
      if (command == "FRRL") {
        setMotor(M1, speed / 2, 0);
        setMotor(M2, speed, 0);
        setMotor(M3, speed, 0);
        setMotor(M4, speed / 2, 0);
      }

      // FORWARD + ROTATE RIGHT
      if (command == "FRRR") {
        setMotor(M1, speed, 0);
        setMotor(M2, speed / 2, 0);
        setMotor(M3, speed / 2, 0);
        setMotor(M4, speed, 0);
      }

      // BACKWARD + LEFT
      if (command == "BKLT") {
        setMotor(M1, 0, speed);
        setMotor(M2, 0, 0);
        setMotor(M3, 0, speed);
        setMotor(M4, 0, 0);
      }

      // BACKWARD + RIGHT
      if (command == "BKRT") {
        setMotor(M1, 0, 0);
        setMotor(M2, 0, speed);
        setMotor(M3, 0, 0);
        setMotor(M4, 0, speed);
      }

      // BACKWARD + ROTATE LEFT
      if (command == "BKRL") {
        setMotor(M1, 0, speed / 2);
        setMotor(M2, 0, speed);
        setMotor(M3, 0, speed);
        setMotor(M4, 0, speed / 2);
      }

      // BACKWARD + ROTATE RIGHT
      if (command == "BKRR") {
        setMotor(M1, 0, speed);
        setMotor(M2, 0, speed / 2);
        setMotor(M3, 0, speed / 2);
        setMotor(M4, 0, speed);
      }

      /*
        * Utilize additional function buttons from below
      */
      // FUNCTION 1
      // if (command == "FNC1") {}

      // FUNCTION 2
      // if (command == "FNC2") {}

      // FUNCTION 3
      // if (command == "FNC3") {}

      // FUNCTION 4
      // if (command == "FNC4") {}
    }
  }
};

void setup() {
  if (VERBOSE) {
    Serial.begin(115200);
    delay(100);
    Serial.println("\n\nWible ESP32 Steer Freely\n");
  }

  pinMode(LED, OUTPUT);

  for (uint8_t i = 0; i < 4; i++) {
    pinMode(motors[i].pinA, OUTPUT);
    pinMode(motors[i].pinB, OUTPUT);
    digitalWrite(motors[i].pinA, LOW);
    digitalWrite(motors[i].pinB, LOW);
  }

  stopMotors();

  // Initialize BLE
  BLEDevice::init("Wible");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(UART_SERVICE_UUID);

  // Create the RX Characteristic (write_nr)
  pRxCharacteristic = pService->createCharacteristic(
    RX_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_WRITE_NR
  );
  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // Create the TX Characteristic (read, notify)
  pTxCharacteristic = pService->createCharacteristic(
    TX_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_NOTIFY
  );

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(UART_SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  pAdvertising->setName("Wible");
  BLEDevice::startAdvertising();

  if (VERBOSE) {
    Serial.println("Device initialized. Advertising...");
  }
}

void loop() {
  if (!deviceConnected) {
    stopMotors();

    digitalWrite(LED, HIGH);
    delay(250);
    digitalWrite(LED, LOW);
    delay(250);
  }

  if (motionActive) {
    if (millis() - lastMotionCommandTime > MOTION_TIMEOUT_MS) {
      motionActive = false;
      stopMotors();
    }
  }
}

// Drives one motor. Only one of speedA/speedB should be non-zero; speedA wins
// if both are set. The motor's LEDC channel follows whichever pin is active.
void setMotor(uint8_t index, uint8_t speedA, uint8_t speedB) {
  Motor &motor = motors[index];

  int8_t wanted = -1;
  uint8_t duty = 0;

  if (speedA > 0) {
    wanted = 0;
    duty = speedA;
  } else if (speedB > 0) {
    wanted = 1;
    duty = speedB;
  }

  // Move the LEDC channel only when the direction actually changes
  if (motor.driven != wanted) {
    if (motor.driven >= 0) {
      uint8_t previous = (motor.driven == 0) ? motor.pinA : motor.pinB;
      ledcDetach(previous);
      pinMode(previous, OUTPUT);
      digitalWrite(previous, LOW);
    }

    if (wanted >= 0) {
      uint8_t active = (wanted == 0) ? motor.pinA : motor.pinB;
      if (!ledcAttachChannel(active, PWM_FREQ, PWM_RESOLUTION, motor.channel)) {
        if (VERBOSE) {
          Serial.print("Failed to attach PWM channel on GPIO ");
          Serial.println(active);
        }
        return;
      }
    }

    motor.driven = wanted;
  }

  if (wanted >= 0) {
    uint8_t active = (wanted == 0) ? motor.pinA : motor.pinB;
    uint8_t idle = (wanted == 0) ? motor.pinB : motor.pinA;
    digitalWrite(idle, LOW);
    ledcWrite(active, duty);
  } else {
    digitalWrite(motor.pinA, LOW);
    digitalWrite(motor.pinB, LOW);
  }
}

void stopMotors() {
  for (uint8_t i = 0; i < 4; i++) {
    setMotor(i, 0, 0);
  }
}
