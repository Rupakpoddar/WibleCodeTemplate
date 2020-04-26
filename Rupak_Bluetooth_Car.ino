//Written by Rupak Poddar
//www.youtube.com/RupakPoddar

#include<SoftwareSerial.h>

SoftwareSerial BTmodule(2,3); //connect the Rx of Bluetooth module to pin 3
                              //connect the Tx of Bluetooth module to pin 2

#define light 13 //connect the light to pin 13
#define a1 11    //connect the motor driver input a1 to pin 11
#define a2 10    //connect the motor driver input a2 to pin 10
#define b1 9     //connect the motor driver input b1 to pin 9
#define b2 6     //connect the motor driver inout b2 to pin 6
#define speaker 8 //connect the speaker to pin 8
#define state 4  //connect the state pin of Bluetooth module to pin 4
String readString;//data received from bluetooth device
int speed = 255;  //initial speed of the car (0-min, 255-max)

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
while (BTmodule.available()) {
delay(3); 
char c = BTmodule.read();
readString += c; 
}
if (readString.length() >0) {
Serial.println(readString);

char temp[10];
readString.toCharArray(temp, 10);

if (readString == "f")//here "f" indicates the received data for forward motion 
{
  analogWrite(a1, speed);
  analogWrite(a2, speed);
}

if (readString == "b")//here "b" indicates the received data for backward motion
{
  analogWrite(b1, speed);
  analogWrite(b2, speed);
}

if (readString == "l")//here "l" indicates the received data to turn left
{
  analogWrite(a1, speed);
  analogWrite(b2, speed);
}

if (readString == "r")//here "r" indicates the received data to turn right
{
  analogWrite(a2, speed);
  analogWrite(b1, speed);
}

if (readString == "s")//here "s" indicates the received data to stop all the motors
{
  analogWrite(a1, 0);
  analogWrite(a2, 0);
  analogWrite(b1, 0);
  analogWrite(b2, 0);
}

if (readString == "ledon")//command to turn ON lights
{
  digitalWrite(light, HIGH);
}

if (readString == "ledoff")//command to turn OFF lights
{
  digitalWrite(light, LOW);
}

if (readString == "hornon")//command to produce horns sound from the speaker
{
  tone(8, 250);
}

if (readString == "hornoff")//command to turn OFF buzzer
{
  noTone(8);
}

if (readString == "blink")//command received when the emergency button on the app is pressed
{
// write here whatever you want the arduino to do when you press the emergency button
}

if(isDigit(temp[0]))
{
  speed = readString.toInt();
  Serial.print("speed: ");
  Serial.println(speed); 
}

readString="";
} 
}
if(digitalRead(state) == 0)//BT 5 ->1, BT 4 ->0
{
  analogWrite(a1, 0);
  analogWrite(a2, 0);
  analogWrite(b1, 0);
  analogWrite(b2, 0);
  digitalWrite(light, HIGH);
  delay(100);
  digitalWrite(light, LOW);
  delay(100);
}
}
