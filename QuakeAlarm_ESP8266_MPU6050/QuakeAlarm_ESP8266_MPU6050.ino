// Internet de las Cosas / Internet of Things / www.internetdelascosas.cl
// Sketch to simulate a QuakeAlarm device using an ESP8266 and a MPU6050
// Version 1.0 ESP8266_MPU6050
// Authors: Jose Antonio Zorrilla @joniuz
// 2019-09-26 @joniuz
// 2021-01-24 @jvicu2001 Implemented MPU6050 Sensor readings

#include "MPU6050_util.h"
//  #include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>



// Network Configuration
#define WIFISSID "Vicuna APX" // Put here your Wi-Fi SSID
#define PASSWORD "pad37crash" // Put here your Wi-Fi

// QuakeAlarm Configuration
#define ID "ff21e0eaba401d343b3f3edb325ad7af" // This ID is unique for each device, ask for one to contact@iot.cl
#define SERVER "dcs.iot.cl" // This is the iot.cl server

// QuakeAlarm internal variables
int qaValue = 0; //  current acceleration value
int qaPreviousValue = 0; // QuakeAlarm previous value
int sensibility = 18; // QuakeAlarm sensibility
float difference = 0; // Diference average for the last 20 readings
long ping = 10000; // Initial interval for ping requests
int timeInterval = 10000; // Time interval between activation requests
long lastConnection = 0; // Last activation request
int lastStatus = 0; // Status of the last connection
long lastPing = 0; // Last ping request

ESP8266WiFiMulti WiFiMulti;

// Function to extract the interger parameters from the XML payload
int xmlTakeParam(String inStr,String needParam)
{ 
  if(inStr.indexOf("<"+needParam+">")>=0){
     int CountChar=needParam.length();
     
     int indexStart=inStr.indexOf("<"+needParam+">");
     int indexStop= inStr.indexOf("</"+needParam+">");  
     return inStr.substring(indexStart+CountChar+2, indexStop).toInt();
  }
  return 0;
}

// Function to send data to the server
void httpRequest(char action[200]) {

  if ((WiFiMulti.run() == WL_CONNECTED)) {
    Serial.println("[ESP8266] Connecting to the server...");
    Serial.print("[ESP8266] WiFiMulti.run() = ");
    Serial.println(WiFiMulti.run());

    char url[200];
    WiFiClient client;

    HTTPClient http;

    Serial.print("[HTTP] begin...\n");

    if (action == "ping") {
      sprintf(url, "http://%s/sensor.php?id=%s&sensor=quakealarm&valor=ping&value=%d", SERVER, ID, qaValue);
      lastPing = millis();
      Serial.println("[HTTP] Start a ping request");
    }
    else {
      sprintf(url, "http://%s/sensor.php?id=%s&sensor=quakealarm&valor=%d&diferencia=%.2f", SERVER, ID, qaValue, difference);
      lastConnection = millis();
      Serial.println("[HTTP] Start a activation request");
    }
    
    if (http.begin(client, url )) {  // HTTP

      Serial.print("[HTTP] GET...\n");
      // start connection and send HTTP header
      int httpCode = http.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // Process the payload sent by the server and print the status 
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          Serial.print("[QuakeAlarm] Payload     : ");
          Serial.println(payload);
          sensibility = xmlTakeParam(payload,"sensibilidad");
          Serial.print("[QuakeAlarm] Sensibility : ");
          Serial.println(sensibility);
          ping = xmlTakeParam(payload,"ping");
          Serial.print("[QuakeAlarm] Ping        : ");
          Serial.println(ping);
          Serial.print("[QuakeAlarm] qaValue     : ");
          Serial.println(qaValue);
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }

      http.end();
    } else {
      Serial.printf("[HTTP} Unable to connect\n");
    }
  }
  else {
    // Wifi connection problems
    Serial.print("[ESP8266] WiFiMulti.run() = ");
    Serial.println(WiFiMulti.run());
    // Wait one second
    delay(1000);
  }
}

//  Initialize Sensor
MPU6050 Sensor(false);       /* Parameter indicates if AD0 pin is connected to VCC (true) or to GND (false)
                              * true = I2C Address 0x69
                              * false = I2C Address 0x68
                              */

void setup() {
  pinMode(D5, OUTPUT);
  Wire.begin();
  Serial.begin(115200);
  // Serial.setDebugOutput(true);

  Sensor.activate(0x06);     /* Parameter indicates Output filtering level.
                              * Min: 0x00, no filtering
                              * Max: 0x06
                              */

  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[Setup] Wait %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(WIFISSID, PASSWORD);

}

void loop() {
  // Get the QuakeAlarm value
  Sensor.readData();
  qaValue = Sensor.getCurrentAcc();

  // Get the difference with the previous value
  difference = Sensor.getDiffAverage();

  
  /*  Testing values 
  Serial.print("VAxis:"); Serial.print(Sensor.getVAxis());
  Serial.print(", MinAccel:"); Serial.print(Sensor.getMinAcc());
  Serial.print(", MaxAccel:"); Serial.print(Sensor.getMaxAcc());
  Serial.print(", Diff:"); Serial.print(abs(Sensor.getAccDiff()));
  Serial.print(", 20SampleMaxDiff:"); Serial.print(Sensor.getBiggestDiff());
  Serial.print(", Diff:"); Serial.print(Sensor.getImmediateDiff());
  Serial.print(", 20SampleDiffAverage:"); Serial.println(difference);
  Serial.print(", Value:"); Serial.println(Sensor.getCurrentAcc());
  Serial.print(", millis:"); Serial.println(millis());
  Serial.print(", MinDiff:"); Serial.print(Sensor.getMinDiff());
  Serial.print(", MaxDiff:"); Serial.print(Sensor.getMaxDiff());
  Serial.print(", AccelSUM:"); Serial.println(Sensor.getCurrentAcc());*/
  
  // Send an activation request to the server
  if ((difference > 0) and (difference >= sensibility) and ((millis() - lastConnection) > timeInterval)) {
    httpRequest("activation");
  }

  // Send a ping request to the server
  if ((millis() - lastPing) > ping) {
    httpRequest("ping");
  }
  delay(0);
}
