/*
 *  This sketch sends ds18b20 sensor temperature data via HTTP POST request to easyIoT home automation server.
 *  It needs the following libraries to work (besides the esp8266 standard libraries supplied with the IDE):
 *
 *  - https://github.com/adamvr/arduino-base64
 *  - https://github.com/milesburton/Arduino-Temperature-Control-Library
 *
 *  designed to run directly on esp8266-01 module, to where it can be uploaded using this marvelous piece of software:
 *
 *  https://github.com/esp8266/Arduino
 *
 *  2015 Dennis Schulze
 *  licensed under GNU GPL
 */

#include <ESP8266WiFi.h>
#include <Base64.h>
#include <OneWire.h>
#include <DallasTemperature.h>


//WIFI credentials go here
const char* ssid     = "MYSSID";
const char* password = "myP@ssW0rd";

//easyIoT credentials go here
char uname[] = "admin:test"; //needs to be replaced with your user/pass
char unameenc[40];
const char* host = "192.168.1.23";  //replace this with your easyIoT server IP address
const int httpPort = 81; //custom easyIoT server http port
const char* nodeID = "N7S0"; //node ID of corresponding virtual node


//initialize 1-wire bus
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);


void setup() {
  Serial.begin(115200);
  delay(10);
  
  //generate base64 string from credentials, for http basic auth
  memset(unameenc,0,sizeof(unameenc));
  base64_encode(unameenc, uname, strlen(uname));
 
  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  delay(10000);
  float temptemp;
  String temperature;
  do {
  sensors.requestTemperatures(); // Send the command to get temperatures
  temptemp = sensors.getTempCByIndex(0);
  temperature = String(temptemp);
  Serial.print("Temperature: ");
  Serial.println(temperature);
  } while (temptemp == 85.0 || temptemp == (-127.0));

  
  Serial.print("connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    setup();
    
  }
  
  // We now create a URI for the request
  String url = "";
  url += "/Api/EasyIoT/Control/Module/Virtual/";
  url += nodeID;
  //url += "/ControlOn";  //use this for simple switch, i.e. DigitalOutput virtual node type
  url += "/ControlLevel/";  // use this for things with more possible values then just one and zero
  url += temperature;
  
  Serial.print("Requesting URL: ");
  Serial.println(url);
  
  // This will send the request to the server
  client.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n" + 
               "Authorization: Basic " + unameenc + " \r\n" + 
               "Content-Length: 0\r\n" + 
               "\r\n"
               );
  delay(10);
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  
  Serial.println();
  Serial.println("closing connection");
}
