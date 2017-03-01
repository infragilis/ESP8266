#include <ESP8266WiFi.h>
#include "DHT.h"



////////////////////DHT11
//Uncomment one of the lines below for whatever DHT sensor type you're using!
#define DHTTYPE DHT11   //DHT11
//#define DHTTYPE DHT21   //DHT21 (AM2301)
//#define DHTTYPE DHT22   //DHT22 (AM2302), AM2321

const int DHTPin = 4;
DHT dht(DHTPin, DHTTYPE);

float humidity, temperature;
////////////////////DHT11

////////////////////set roomname/tilename  
#define ROOM_NAME "outside"

////////////////////WiFi  Router
#define WLAN_SSID "xxx"
#define WLAN_PASS "xxx"
////////////////////WiFi  Router



////////////////////SSL and Initial State
#define INITIALSTATE_ACCESS_KEY "xxx"
#define INITIALSTATE_BUCKET_KEY "xxx"
#define INITIALSTATE_API "groker.initialstate.com"
#define INITIALSTATE_PORT 443
#define INITIALSTATE_ENDPOINT "/api/events"

WiFiClientSecure client;

//groker.initialstate.com SHA1 fingerprint
const char* fingerprint = "D4 2A 25 CC 50 60 64 2C D9 D1 DD 4B 26 0B 0E DE 57 FC 12 05";

const char* host = INITIALSTATE_API;
////////////////////SSL and Initial State



////////////////////JSON & HTTPS
String jsonBuf = "";
String httpRequestContent = "";
String httpRequestContent_Part1 = "";
////////////////////JSON & HTTPS



void setup() {
  //Preform the HTTP header
  httpRequestContent_Part1 += "POST ";
  httpRequestContent_Part1 += INITIALSTATE_ENDPOINT;
  httpRequestContent_Part1 += " HTTP/1.0\r\n";
  httpRequestContent_Part1 += "User-Agent: esp8266/2.3.0 \r\n";
  httpRequestContent_Part1 += "Host: ";
  httpRequestContent_Part1 += INITIALSTATE_API;
  httpRequestContent_Part1 += "\r\n";
  httpRequestContent_Part1 += "Accept: */*\r\n";
  httpRequestContent_Part1 += "Content-Type: application/json\r\n";
  httpRequestContent_Part1 += "X-IS-AccessKey: ";
  httpRequestContent_Part1 += INITIALSTATE_ACCESS_KEY;
  httpRequestContent_Part1 += "\r\n";
  httpRequestContent_Part1 += "X-IS-BucketKey: ";
  httpRequestContent_Part1 += INITIALSTATE_BUCKET_KEY;
  httpRequestContent_Part1 += "\r\n";
  httpRequestContent_Part1 += "Accept-Version: 0.0.4\r\n";
  httpRequestContent_Part1 += "Content-Length: ";

  //Initialize Serial
  Serial.begin(115200);
  delay(10);

  //Initialize DHT11
  dht.begin();
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output

  //Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.print(WLAN_SSID);
  Serial.print("...");
  delay(1000);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  delay(5000);

  Serial.println();
  Serial.println("WiFi LAN IP address: ");
  Serial.println(WiFi.localIP());
}


long loop_counter = 0;
void loop() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  loop_counter++;
  
  //Check if any reads failed and exit early (to try again).
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT11 sensor!");
    Serial.print("_________________________________________Iteration #");
    Serial.print(loop_counter);
    Serial.println(" Ended");
    delay(10000);
  } else { //DHT11 read successfully!
    Serial.print("Connecting to ");
    Serial.print(INITIALSTATE_API);
    Serial.print(":");
    Serial.print(INITIALSTATE_PORT);
    Serial.print("...");
    if (! client.connect(host, INITIALSTATE_PORT)) {
      Serial.println("Connection failed. Will retry...");
      Serial.print("_________________________________________Iteration #");
      Serial.print(loop_counter);
      Serial.println(" Ended.");
      client.stop();
    }
    if (client.verify(fingerprint, host)) {
      Serial.println("Connection secure. Uploading data using HTTPS...");

      jsonBuf = "";
      jsonBuf += "[";
      jsonBuf += "{";
      jsonBuf += " \"key\": \"Temperature-"ROOM_NAME"\",";
      jsonBuf += " \"value\": \"";
      jsonBuf += String(temperature);
      jsonBuf += "\"";
      jsonBuf += " },";
      jsonBuf += "{";
      jsonBuf += " \"key\": \"Humidity-"ROOM_NAME"\",";
      jsonBuf += " \"value\": \"";
      jsonBuf += String(humidity);
      jsonBuf += "\"";
      jsonBuf += " }";
      jsonBuf += "]";

      httpRequestContent = "";
      httpRequestContent += httpRequestContent_Part1;
      httpRequestContent += String(jsonBuf.length());
      httpRequestContent += "\r\n\r\n";
      httpRequestContent += jsonBuf;

      Serial.println(httpRequestContent);
      client.print(httpRequestContent);
      Serial.println("Done uploading, disconnecting...");
      Serial.print("_________________________________________Iteration #");
      Serial.print(loop_counter);
      Serial.println(" Ended");
      client.stop();
    } else {
      Serial.println("Connection insecure, disconnecting, will retry...");
      Serial.print("_________________________________________Iteration #");
      Serial.print(loop_counter);
      Serial.println(" Ended");
      client.stop();
    }
    digitalWrite(LED_BUILTIN, LOW);    
    delay(10000);
    digitalWrite(LED_BUILTIN, HIGH); 


    
  }
}
