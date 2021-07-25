// ----------------------------
// Default Libraries
// ----------------------------

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

// ----------------------------
// Additional libraries - each of these must be installed.
// ----------------------------

#include <ArduinoJson.h>
// Library used to parse Json from API responses
// Search for "Arduino Json" in Arduino's library manager

//------- Replace the fields! ------
char ssid[] = "YOUR_SSID";       // your ssid name (SSID)
char password[] = "YOUR_PASSWORD";  // your password

// For HTTPS requests
WiFiClientSecure client;

// Just the base of the URL you want to connect to
#define TEST_HOST "api.coingecko.com"

// OPTIONAL - The fingerprint of the website
#define TEST_HOST_FINGERPRINT "YOUR_FINGERPRINT_OF_API"
// The fingerprint changes every few months.

int ledPin = LED_BUILTIN;

void setup() {

  Serial.begin(9600);

  // Connect WiFi
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // Set ledpin with output and leave it off
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH); //inverse to ESP

  // Try to connect to Wifi network:
  delay(500);
  Serial.print("Connecting to Wifi: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("Server running in:");
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);

  //--------

  // Check the fingerprint
  client.setFingerprint(TEST_HOST_FINGERPRINT);
}

void makeHTTPRequest() {
  
  // Opening server connection
  if (!client.connect(TEST_HOST, 443))
  {
    Serial.println(F("Connection failed"));
    return;
  }

  // Run utility functions in the background for ESP
  yield();

  // Send GET request
  client.print(F("GET "));
  // This is the second part of the request (everything after the base URL)
  client.print("/api/v3/simple/price?ids=bitcoin&vs_currencies=usd%2Cbrl");
  client.println(F(" HTTP/1.1"));

  //Headers
  client.print(F("Host: "));
  client.println(TEST_HOST);

  client.println(F("Cache-Control: no-cache"));

  if (client.println() == 0)
  {
    Serial.println(F("Failed to send request"));
    return;
  }

  // Check HTTP status
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0)
  {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
    return;
  }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders))
  {
    Serial.println(F("Invalid response"));
    return;
  }

  // This is probably not necessary for most, but some
  // API's have characters going back before the response body.
  // peek() will look at the character, but not take it out of the queue
  while (client.available() && client.peek() != '{')
  {
    char c = 0;
    client.readBytes(&c, 1);
  }

  // Use the ArduinoJson Assistant to calculate the JSON size and not overflow the buffers:
  //https://arduinojson.org/v6/assistant/
  
  DynamicJsonDocument doc(192); //For ESP32/ESP8266 you will use dynamic

  DeserializationError error = deserializeJson(doc, client);

  if (!error) {

    long bitcoin_usd = doc["bitcoin"]["usd"];
    long bitcoin_brl = doc["bitcoin"]["brl"];
    
    Serial.print("-------------");
    Serial.println((String)"\n\n");
     
    if (bitcoin_brl < THE VALUE YOU WANT TO CHECK) { // Price of the day configured for verification
      Serial.print("Bitcoin lowered to: ");
      Serial.println((String)bitcoin_brl + ",00 reais\n");
      
      digitalWrite(ledPin, LOW); // Turns up led if value is lower
    }

    else if (bitcoin_brl > THE VALUE YOU WANT TO CHECK){ // Price of the day configured for verification
      Serial.print("Bitcoin increased to:  ");
      Serial.println((String)bitcoin_brl + ",00 reais\n");
      
      digitalWrite(ledPin, LOW); // Turns up led if value is higher
    }

    else{
      digitalWrite(ledPin, HIGH); // Turn off led if value is between the two
    }

    Serial.print("bitcoin_usd: ");
      Serial.println((String)bitcoin_usd + ",00 dolares\n");
    Serial.print("bitcoin_brl: ");
      Serial.println((String)bitcoin_brl + ",00 reais\n\n");
    
  } else {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

}

void loop() {

  makeHTTPRequest();
  
  delay(10000); //10s
  
}
