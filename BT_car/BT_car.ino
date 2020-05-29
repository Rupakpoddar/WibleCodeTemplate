//  Written by Rupak Poddar
//  www.youtube.com/RupakPoddar

#include<SoftwareSerial.h>

SoftwareSerial BTmodule(2,3); //Connect the RX pin of Bluetooth module to pin 3
                              //Connect the TX pin of Bluetooth module to pin 2
                              //Connect the STATE pin of Bluetooth module to pin 4

#define light 13  //Connect the light to pin 13
#define a1 11     //Connect the motor driver input a1 to pin 11
#define a2 10     //connect the motor driver input a2 to pin 10
#define b1 9      //connect the motor driver input b1 to pin 9
#define b2 6      //connect the motor driver inout b2 to pin 6

#define state 4

void setup() {
Serial.begin(9600);
BTmodule.begin(9600);
BTmodule.setTimeout(10);
pinMode(a1, OUTPUT);
pinMode(a2, OUTPUT); 
pinMode(b1, OUTPUT);
pinMode(b2, OUTPUT);
pinMode(light, OUTPUT);
pinMode(state, INPUT);
}

void loop() {
if(digitalRead(state) == 1)//BT 5 ->0, BT 4->1
{
  String readString;
  
while (BTmodule.available()) {
readString = BTmodule.readString();
Serial.println(readString);
}

if (readString == "f")  //Forward
{
  digitalWrite(a1, HIGH);
  digitalWrite(b1, HIGH);
}

if (readString == "b")  //Backward
{
  digitalWrite(a2, HIGH);
  digitalWrite(b2, HIGH);
}

if (readString == "l")  //Left
{
  digitalWrite(a2, HIGH);
  digitalWrite(b1, HIGH);
}

if (readString == "r")  //Right
{
  digitalWrite(a1, HIGH);
  digitalWrite(b2, HIGH);
}

if (readString == "s")  //Stop
{
  digitalWrite(a1, LOW);
  digitalWrite(a2, LOW);
  digitalWrite(b1, LOW);
  digitalWrite(b2, LOW);
}

if (readString == "LED_ON")
{
  digitalWrite(light, HIGH);
}

if (readString == "LED_OFF")
{
  digitalWrite(light, LOW);
}

/*
//------------------------------------------------------//
//  For camera control(can be used as optional buttons) //
if (readString == "save"){
  //Do nothing!
}

if (readString == "reload"){
  //Do nothing!
}
//------------------------------------------------------//
*/
}

else{
  digitalWrite(a1, LOW);
  digitalWrite(a2, LOW);
  digitalWrite(b1, LOW);
  digitalWrite(b2, LOW);
  digitalWrite(light, HIGH);
  delay(100);
  digitalWrite(light, LOW);
  delay(100);
}
} //void loop ends
