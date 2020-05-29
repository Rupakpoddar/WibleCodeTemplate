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
int speed = 255;  //Initial speed(0-min, 255-max)

void setup() {
Serial.begin(9600);
BTmodule.begin(9600);
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
  char temp[10];
  
while (BTmodule.available()) {
readString = BTmodule.readString();
Serial.println(readString);
readString.toCharArray(temp, 10);
}

if (readString == "f")  //Forward
{
  analogWrite(a1, speed);
  analogWrite(b1, speed);
}

if (readString == "b")  //Backward
{
  analogWrite(a2, speed);
  analogWrite(b2, speed);
}

if (readString == "l")  //Left
{
  analogWrite(a2, speed);
  analogWrite(b1, speed);
}

if (readString == "r")  //Right
{
  analogWrite(a1, speed);
  analogWrite(b2, speed);
}

if (readString == "s")  //Stop
{
  analogWrite(a1, 0);
  analogWrite(a2, 0);
  analogWrite(b1, 0);
  analogWrite(b2, 0);
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

if(isDigit(temp[0]))
{
  speed = readString.toInt();
  Serial.print("speed: ");
  Serial.println(speed); 
}

}

else{
  analogWrite(a1, 0);
  analogWrite(a2, 0);
  analogWrite(b1, 0);
  analogWrite(b2, 0);
  digitalWrite(light, HIGH);
  delay(100);
  digitalWrite(light, LOW);
  delay(100);
}
} //void loop ends
