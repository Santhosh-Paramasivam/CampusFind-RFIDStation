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

// Wifi Network login credentials
const char* ssid = "";
const char* password = "";

// Flask backend server base path
const char* server = "";

// Public credentials
const String public_api_key = "";

// Institution-specific credentials
// The request can only update records of that institution
const String institution_id = "";
const String institution_api_key = "";

// server's ssl fingerprint
const char* ssl_fingerprint = "";

void setup() {
  Serial.begin(115200);
  SPI.begin();          
  rfid.PCD_Init();       
  timeClient.begin();

  // Sets an on-board LED as an output
  pinMode(2, OUTPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("WiFi connected");

  // On-board LED turns on when WiFi connection is established
  digitalWrite(2, LOW);
}

/**
 * @brief Reads RFID card and extracts UID as a string.
 *        Calls updateUserLocation() to send the UID to the backend.
 */
void loop() {
  // Once a request is sent to the backend, the on-board LED turns back on
  digitalWrite(2,LOW);
  if (!rfid.PICC_IsNewCardPresent()) {
    return;
  }

  if (!rfid.PICC_ReadCardSerial()) {
    return;
  }

  // When a request is being sent to the backend, the on-board LED turns off
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

  // Uncomment to print RFID UID
  //Serial.print("RFID UID: ");
  //Serial.println(uidStr);

  rfid.PICC_HaltA();
  Serial.println(uidStr);
  updateUserLocation(uidStr);
}

/**
 * @brief Sends the user's scanned RFID UID, MAC address, institution_api_key, institution_id
 * and entry time to the backend.
 * 
 * @param uid - The RFID UID in a string format (e.g., "AB:CD:12:34")
 * 
 * @note This function ensures secure communication using SSL fingerprints.
 *       If the server's certificate changes, it may cause a connection failure.
 */
void updateUserLocation(String uid)
{
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected to WiFi");

    // Ensure that all communication is encrypted
    WiFiClientSecure client;

    // Uncomment to view esp8266's macAddress
    // Serial.println(WiFi.macAddress());

    // Verifies that the server is the correct one
    // Will fail if the server certificate changes
    // Use client.setTrustAnchors for more persistent connections
    client.setFingerprint(ssl_fingerprint);

    // Uncomment to connect to the server without verifying
    // the server's identity, this is unsafe
    // client.setInsecure();

    if (!client.connect(server, 443)) {
      Serial.println("Connection to server failed");
      delay(1000);
      return;
    }

    StaticJsonDocument<200> doc;
    // Mac address is used to identify the location (building/floor/room)
    doc["mac_address"] = WiFi.macAddress();
    // Uid is used to identify user
    doc["uid"] = uid;
    // Used to identify the institution
    doc["institution_id"] = institution_id;
    // Institution api key is used to authenticate the institution
    doc["api_key"] = institution_api_key;
    // room entry/exit time
    doc["entry_time"] = getISOTime();

    String jsonString;
    serializeJson(doc, jsonString);

    String url = "/update_user_location_secure";

    client.print(String("POST ") + url + " HTTP/1.1\r\n" +
                 "Host: " + server + "\r\n" +
                 "Content-Type: application/json\r\n" +
                 "x-api-key:" + public_api_key + "\r\n" +
                 "Content-Length: " + jsonString.length() + "\r\n" +
                 "Connection: close\r\n\r\n" +
                 jsonString);

    // Uncomment to get server response

    // while (client.connected()) {
    //  String line = client.readStringUntil('\n');
    //  if (line == "\r") {
    //    break;
    //  }
    // }

    // String response = client.readString();

    // if (response.indexOf("200 OK") != -1) {
    //  Serial.println("Data sent successfully");
    // } else {
    //  Serial.println("Error sending data");
    //  Serial.println("Full server response: ");
    //  Serial.println(response);  // Print full response in case of an error
    // }

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

/**
 * @brief Gets the current UTC time in ISO 8601 format.
 * 
 * @return ISO-formatted string timestamp (e.g., "2025-03-21T12:34:56Z")
 */
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
