#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "./dht11.h"

// Sensor de Temperatura
#define DHT11PIN D6
dht11 DHT11;


// Wifi
const char* ssid = "WIFI_NAME";
const char* password = "WIFI_PASSWORD";

// Tiempo
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

String getCurrentTime() {
  timeClient.update();
  time_t epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime ((time_t *)&epochTime); 
  int currentMonth = ptm->tm_mon+1;
  int currentYear = ptm->tm_year+1900;
  int monthDay = ptm->tm_mday;
  String currentDate = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay);
  return currentDate + " " + timeClient.getFormattedTime();
}


// POST to API
void PostToAPI(int temperature) {
  const char* host = "io-tapi.vercel.app";
  const char* endpoint = "/setSensor";

  WiFiClientSecure client;
  client.setInsecure(); // Ignore SSL certificate verification

  String currentTime = getCurrentTime();

  // Create a JSON document with the data to send
  StaticJsonDocument<200> postData;
  postData["temperatura"] = temperature;
  postData["humedad"] = 0.0;
  postData["orientacion"] = 0;
  postData["luz"] = 0;
  postData["id_lugar"] = 0;
  postData["fecha_hora"] = currentTime;

  String postDataStr;
  serializeJson(postData, postDataStr);

  // Make an HTTPS POST request
  Serial.println("Starting HTTPS POST request...");

  if (client.connect(host, 443)) {
    String postRequest = "POST " + String(endpoint) + " HTTP/1.1\r\n" +
                    "Host: " + String(host) + "\r\n" +
                    "Content-Type: application/json\r\n" +
                    "Content-Length: " + String(postDataStr.length()) + "\r\n\r\n" +
                    postDataStr;
    client.print(postRequest);

    client.stop();
  } else {
    Serial.println("Connection failed");
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  bool firstPrint = false;

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);

    if(!firstPrint) {
      Serial.println("Connecting to WiFi...");
      firstPrint = false;
    }
  }

  Serial.println("Connected to WiFi");

  // Initialize the NTP client
  timeClient.begin();
  timeClient.setTimeOffset(-21600);  // UTC-6
}

void loop() {
  int chk = DHT11.read(DHT11PIN);
  float temperature = DHT11.temperature;
  
  Serial.println(temperature);

  PostToAPI(temperature);

  delay(5000);
}