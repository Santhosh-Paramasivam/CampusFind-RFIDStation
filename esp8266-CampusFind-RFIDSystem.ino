#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <SPI.h>
#include <MFRC522.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <ArduinoJson.h>

#define SS_PIN 16
#define RST_PIN 0

MFRC522 rfid(SS_PIN, RST_PIN);

const char* ssid = "";
const char* password = "";
const char* server = "";
const String apiKey = "";
const String institution_id = "";
const String institution_api_key = "";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000); // UTC time

void setup() {
  Serial.begin(115200);
  SPI.begin();          
  rfid.PCD_Init();       
  timeClient.begin();

  pinMode(2, OUTPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("WiFi connected");

  digitalWrite(2, LOW);
}

void loop() {
  digitalWrite(2,LOW);
  if (!rfid.PICC_IsNewCardPresent()) {
    return;
  }

  if (!rfid.PICC_ReadCardSerial()) {
    return;
  }

  digitalWrite(2,HIGH);
  // Store UID in a string
  String uidStr = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    String hexByte = String(rfid.uid.uidByte[i], HEX);
    if (rfid.uid.uidByte[i] < 0x10) {
      hexByte = "0" + hexByte;  
    }
    hexByte.toUpperCase(); 
    uidStr += hexByte;

    if (i < rfid.uid.size - 1) {
      uidStr += ":";
    }
  }

  //Serial.print("RFID UID: ");
  //Serial.println(uidStr);

  rfid.PICC_HaltA();
  Serial.println(uidStr);
  updateUserLocation(uidStr);
}

void updateUserLocation(String uid)
{
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected to WiFi");

    WiFiClientSecure client;
    Serial.println(WiFi.macAddress());

    client.setInsecure();

    if (!client.connect(server, 443)) {
      Serial.println("Connection to server failed");
      delay(1000);
      return;
    }

    StaticJsonDocument<200> doc;
    doc["mac_address"] = WiFi.macAddress();
    doc["uid"] = uid;
    doc["api_key"] = institution_api_key;
    doc["institution_id"] = institution_id;
    doc["entry_time"] = getISOTime();

    String jsonString;
    serializeJson(doc, jsonString);

    //String postData = "{\"mac_address\":" + WiFi.macAddress() + "}"; 
    String url = "/update_user_location_secure";

    client.print(String("POST ") + url + " HTTP/1.1\r\n" +
                 "Host: " + server + "\r\n" +
                 "Content-Type: application/json\r\n" +
                 "x-api-key:" + apiKey + "\r\n" +
                 "Content-Length: " + jsonString.length() + "\r\n" +
                 "Connection: close\r\n\r\n" +
                 jsonString);

    //while (client.connected()) {
    //  String line = client.readStringUntil('\n');
    //  if (line == "\r") {
    //    break;
    //  }
    //}

    //String response = client.readString();

    //if (response.indexOf("200 OK") != -1) {
    //  Serial.println("Data sent successfully");
    //} else {
    //  Serial.println("Error sending data");
    //  Serial.println("Full server response: ");
    //  Serial.println(response);  // Print full response in case of an error
    //}

    client.stop();
  }
  else {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Reconnecting to WiFi...");
    }
  }
}

String getISOTime() {
  // Update NTP time
  timeClient.update();

  // Get current timestamp
  unsigned long epochTime = timeClient.getEpochTime();

  time_t rawTime = epochTime;
  struct tm * ptm = gmtime(&rawTime);

  char buffer[25];
  snprintf(buffer, sizeof(buffer), "%04d-%02d-%02dT%02d:%02d:%02dZ",
           ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday,
           ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

  return String(buffer);
}
