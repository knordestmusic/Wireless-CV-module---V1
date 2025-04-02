#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_MCP4725.h>

// ----- WiFi Settings -----
const char* ssid     = "Knordest";       // Your WiFi SSID
const char* password = "jugbug69";       // Your WiFi Password

// Start a WiFi server on port 80 to receive tilt data from the sensor board
WiFiServer wifiServer(80);

// ----- DAC Setup -----
// DAC for output 1 (using tilt from X-axis) at I²C address 0x60
Adafruit_MCP4725 dac1;
// DAC for output 2 (using tilt from Y-axis) at I²C address 0x61
Adafruit_MCP4725 dac2;

// Global variables for tilt data from sensor board
float tiltX = 0.0;
float tiltY = 0.0;
bool sensorActive = false; // true if sensor board sent enable flag (1)

// This function maps a tilt value (in m/s²) to a normalized value between 0 and 1.
// It assumes a tilt range from -3.0 to +3.0 m/s².
float mapTiltToNormalized(float tilt) {
  float norm = (tilt + 3.0) / 6.0;
  if (norm < 0.0) norm = 0.0;
  if (norm > 1.0) norm = 1.0;
  return norm;
}

// Update tilt data from sensor board via WiFi.
// Expects CSV data in the format: "accX,accY,enable\n"
void updateTiltData() {
  WiFiClient client = wifiServer.available();
  if (client) {
    String data = client.readStringUntil('\n');
    data.trim();
    if (data.length() > 0) {
      int firstComma = data.indexOf(',');
      int secondComma = data.indexOf(',', firstComma + 1);
      if (firstComma != -1 && secondComma != -1) {
        tiltX = data.substring(0, firstComma).toFloat();
        tiltY = data.substring(firstComma + 1, secondComma).toFloat();
        int enFlag = data.substring(secondComma + 1).toInt();
        sensorActive = (enFlag == 1);
        Serial.print("IP ");
        Serial.print(WiFi.localIP());
        Serial.print(" | Tilt received: X=");
        Serial.print(tiltX, 2);
        Serial.print(" Y=");
        Serial.print(tiltY, 2);
        Serial.print(" Active=");
        Serial.println(sensorActive);
      }
    }
    client.stop();
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  
  // Connect to WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("WiFi connected! IP: ");
  Serial.println(WiFi.localIP());
  
  wifiServer.begin();
  
  // Initialize the DACs
  dac1.begin(0x60);
  dac2.begin(0x61);
}

void loop() {
  // Update tilt data from sensor board via WiFi
  updateTiltData();
  
  // Compute output only if sensor is active; otherwise, output 0
  int dacVal1 = 0;
  int dacVal2 = 0;
  
  if (sensorActive) {
    float normX = mapTiltToNormalized(tiltX); // normalized value for X-axis
    float normY = mapTiltToNormalized(tiltY); // normalized value for Y-axis
    dacVal1 = int(normX * 4095);
    dacVal2 = int(normY * 4095);
  }
  
  // Update the DAC outputs
  dac1.setVoltage(dacVal1, false);
  dac2.setVoltage(dacVal2, false);
  
  // Print debug information, with IP on every line
  Serial.print("IP ");
  Serial.print(WiFi.localIP());
  Serial.print(" | DAC1: ");
  Serial.print(dacVal1);
  Serial.print(" | DAC2: ");
  Serial.println(dacVal2);
  
  delay(5); 
}
