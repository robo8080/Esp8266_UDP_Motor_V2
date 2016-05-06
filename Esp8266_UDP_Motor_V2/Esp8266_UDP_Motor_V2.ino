/*
 * RCWController &
 * ESP-WROOM-2 & Adafruit Motor Shield V2 for Arduino
 * 2016/5/6 by robo8080
 * RCWController
 *  http://rcwcontroller.micutil.com
 * Adafruit Motor Shield V2 for Arduino 
 *  http://akizukidenshi.com/catalog/g/gK-07748/
 *  https://learn.adafruit.com/adafruit-motor-shield-v2-for-arduino
 */
 
#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char ssid[] = "ESP-Motor";  //  your network SSID (name)
const char pass[] = "esp8266ap";  // your network password

WiFiUDP udp;
unsigned int localPort = 10000;
const int PACKET_SIZE = 256;
char packetBuffer[PACKET_SIZE];
int status = WL_IDLE_STATUS;
int prev_S=0;

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
Adafruit_DCMotor *left_track = AFMS.getMotor(1);
Adafruit_DCMotor *right_track = AFMS.getMotor(2);

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  pinMode(13, OUTPUT);
  AFMS.begin();  // create with the default frequency 1.6KHz
  // Set the speed to start, from 0 (off) to 255 (max speed)
  left_track->setSpeed(prev_S);
  right_track->setSpeed(prev_S);
  left_track->run(FORWARD);
  right_track->run(FORWARD);
  // turn on motor
  left_track->run(RELEASE);
  right_track->run(RELEASE);

  WiFi.softAP(ssid, pass);
  IPAddress myIP = WiFi.softAPIP();

  Serial.print("\nAP IP address: ");
  Serial.println(myIP);

  Serial.println("Starting UDP");
  udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());
}

void loop() {
  int rlen, val_V=0,val_S=0;

  while (1) {
    rlen = udp.parsePacket();
    if(rlen<10) {
      delay(1);
      continue;
    }

    udp.read(packetBuffer, (rlen > PACKET_SIZE) ? PACKET_SIZE : rlen);

    val_S=packetBuffer[5];
    if(val_S!=prev_S) {
      //Right analogue y-axis
      left_track->setSpeed(val_S);
      right_track->setSpeed(val_S);
      analogWrite(13,val_S);
      prev_S=val_S;
    }
    
    val_V=packetBuffer[1];
    if(val_V) {
      switch (val_V) {
        case 1://Left cross up
            left_track->run(FORWARD);
            right_track->run(FORWARD);
          break;
        case 2://Left cross down
            left_track->run(BACKWARD);
            right_track->run(BACKWARD);
          break;
        case 4://Left cross right
            left_track->run(BACKWARD);
            right_track->run(FORWARD);
          break;
        case 8://Left cross right
            left_track->run(FORWARD);
            right_track->run(BACKWARD);
          break;
      }      
    } else {
      //Release button
      prev_S = 0;
      left_track->setSpeed(prev_S);
      right_track->setSpeed(prev_S);
      left_track->run(RELEASE);
      right_track->run(RELEASE);
    }
    /*
    for(int i=0;i<rlen;i++) {
      Serial.printf("%d",packetBuffer[i]);
    }
    Serial.printf("\n");
    */
    //delay(10);
  }
}
