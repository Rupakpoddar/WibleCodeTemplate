/* 
 *  Written by Rupak Poddar
 *  www.youtube.com/RupakPoddar
 *  www.github.com/RupakPoddar
*/

#include<SoftwareSerial.h>
#define LED 13              // Connect light to pin 13 (Optional)
#define BUZZER 12        // Connect buzzer to pin 12 (Optional)
#define MOTOR_1A 11   // Connect motor driver input 1 to pin 11
#define MOTOR_1B 10   // Connect motor driver input 2 to pin 10
#define MOTOR_2A 9     // Connect motor driver input 3 to pin 9
#define MOTOR_2B 6     // Connect motor driver input 4 to pin 6
#define TX 2                 // Connect Bluetooth module Tx to pin 2
#define RX 3                 // Connect Bluetooth module Rx to pin 3

SoftwareSerial BTmodule(TX, RX);
int speed = 255;

void setup() {
  Serial.begin(9600);
  BTmodule.begin(9600);
  BTmodule.setTimeout(10);
  pinMode(MOTOR_1A, OUTPUT);
  pinMode(MOTOR_1B, OUTPUT); 
  pinMode(MOTOR_2A, OUTPUT);
  pinMode(MOTOR_2B, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
}

void loop() {
  String readString;
  
  while(BTmodule.available()) {
    readString = BTmodule.readString();
    Serial.print("Received: ");
    Serial.println(readString);
  }
                                                                                       // FORWARD
  if(readString == "f"){
    analogWrite(MOTOR_1A, speed);
    analogWrite(MOTOR_2A, speed);
  }
                                                                                       // BACKWARD
  if(readString == "b"){
    analogWrite(MOTOR_1B, speed);
    analogWrite(MOTOR_2B, speed);
  }
                                                                                       // LEFT
  if(readString == "l"){
    analogWrite(MOTOR_1A, speed);
    analogWrite(MOTOR_2B, speed);
  }
                                                                                       // RIGHT
  if(readString == "r"){
    analogWrite(MOTOR_1B, speed);
    analogWrite(MOTOR_2A, speed);
  }
                                                                                       // STOP
  if(readString == "s"){
    analogWrite(MOTOR_1A, 0);
    analogWrite(MOTOR_1B, 0);
    analogWrite(MOTOR_2A, 0);
    analogWrite(MOTOR_2B, 0);
  }
                                                                                       // LED ON
  if(readString == "LED_ON"){
    digitalWrite(LED, HIGH);
  }
                                                                                       // LED OFF
  if(readString == "LED_OFF"){
    digitalWrite(LED, LOW);
  }
                                                                                       // SOUND
  if(readString == "SOUND"){
    digitalWrite(BUZZER, HIGH);
    delay(100);
    digitalWrite(BUZZER, LOW);
  }
                                                                                       // AUX
  if(readString.substring(0, 3) == "aux"){
    int auxVal = readString.substring(3).toInt();
    
    // Print the value of aux button pressed, ranging from 1-8
    Serial.print("Aux ");
    Serial.println(auxVal);
  }
                                                                                       // SPEED
   if(readString.substring(0, 6) == "Speed:"){
    String val = readString.substring(readString.length()-3);
    if(val.substring(0, 1) == "d"){
      speed = val.substring(2).toInt();
    }
    else if(val.substring(0, 1) == ":"){
      speed = val.substring(1).toInt();
    }
    else{
      speed = val.toInt();
    }
    Serial.print("Speed: ");
    Serial.println(speed);
  }
}
