#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>


const char* WIFI_SSID = "Home_Workshop_Net";
const char* WIFI_PASSWORD = "secure_workshop_password";


const char* MCP_SERVER_URL = "http://192.168.1.100:8000/api/mcp/telemetry";


#define DHT_PIN 4          // GPIO 4 for DHT22 Temperature & Humidity Sensor
#define DHT_TYPE DHT22     
#define RELAY_PIN 22       // GPIO 22 for Optoisolated Relay Module (Ceiling Fan / Lights)
#define CURRENT_SENSOR 34  // GPIO 34 (ADC) for SCT-013 AC Current Transformer

DHT dht(DHT_PIN, DHT_TYPE);


unsigned long lastTelemetryMillis = 0;
const long telemetryInterval = 5000; 

void connectWiFi() {
  Serial.begin(115200);
  delay(1000);
  Serial.printf("\n[WiFi] Connecting to %s", WIFI_SSID);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.printf("\n[WiFi] Connected! IP Address: %s\n", WiFi.localIP().toString().c_str());
}

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); 
  
  pinMode(CURRENT_SENSOR, INPUT);
  
  dht.begin();
  connectWiFi();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  unsigned long currentMillis = millis();
  if (currentMillis - lastTelemetryMillis >= telemetryInterval) {
    lastTelemetryMillis = currentMillis;

    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
    int rawAdcCurrent = analogRead(CURRENT_SENSOR);
    float estimatedCurrentAmps = (rawAdcCurrent / 4095.0 * 3.3) * 10.0; 

    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("[Sensor] Error: Failed to read from DHT sensor!");
      return;
    }

    
    String jsonPayload = "{";
    jsonPayload += "\"node_id\": \"ESP32-NODE-01\",";
    jsonPayload += "\"temperature_c\": " + String(temperature, 2) + ",";
    jsonPayload += "\"humidity_pct\": " + String(humidity, 2) + ",";
    jsonPayload += "\"current_amps\": " + String(estimatedCurrentAmps, 2) + ",";
    jsonPayload += "\"relay_status\": " + String(digitalRead(RELAY_PIN) == HIGH ? "true" : "false");
    jsonPayload += "}";

   
    HTTPClient http;
    http.begin(MCP_SERVER_URL);
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(jsonPayload);
    if (httpResponseCode > 0) {
      Serial.printf("[MCP Sync] HTTP Response code: %d\n", httpResponseCode);
      String responseString = http.getString();
      Serial.println("[MCP Sync] Server Reply: " + responseString);
      

      if (responseString.indexOf("\"command\":\"TURNOFF\"") > 0) {
        digitalWrite(RELAY_PIN, LOW);
        Serial.println("[Actuator] Fan/Light turned OFF by MCP Server command.");
      } else if (responseString.indexOf("\"command\":\"TURNON\"") > 0) {
        digitalWrite(RELAY_PIN, HIGH);
        Serial.println("[Actuator] Fan/Light turned ON by MCP Server command.");
      }
    } else {
      Serial.printf("[MCP Sync] Error on sending POST: %s\n", http.errorToString(httpResponseCode).c_str());
    }
    http.end();
  }
}
