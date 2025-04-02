#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <ESP8266WiFi.h>

// ----- WiFi Settings -----
const char* ssid     = "Knordest";         // Replace with your WiFi SSID
const char* password = "jugbug69";     // Replace with your WiFi Password
const char* serverIP = "192.168.105.118";        // Replace with your Receiver's IP address (it changes with each change in the connection, so needs to be replaced everytime)
const int serverPort = 80;                   // Receiver's server port

// Create MPU6050 object
Adafruit_MPU6050 mpu;

// Pin definitions
const int buttonPin = 13; // Button on GPIO13
const int ledPin    = 2;  // LED on GPIO2

void setup() {
  Serial.begin(115200);
  Wire.begin();
  
  // Configure button pin with internal pull-up enabled.
  // With this setting, the pin will normally be HIGH, and when the button is pressed (connecting to ground) it will read LOW.
  pinMode(buttonPin, INPUT_PULLUP);
  
  // Configure LED pin as output.
  pinMode(ledPin, OUTPUT);
  
  // Connect to WiFi.
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(5);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  
  // Initialize MPU6050.
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 initialized");
  
  // Optional: Set accelerometer range and filter bandwidth.
  mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
}

void loop() {
  // Read the button state.
  // With internal pull-up enabled, the button will read HIGH when not pressed,
  // and LOW when pressed.
  bool sensorEnabled = (digitalRead(buttonPin) == LOW);
  
  // Update the LED: turn it on if the button is pressed.
  digitalWrite(ledPin, sensorEnabled ? HIGH : LOW);
  
  String message;
  if (sensorEnabled) {
    // If the button is pressed, read the accelerometer.
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    
    // Create a CSV string with the X and Y acceleration values (with 2 decimal places)
    // and add an enable flag (1).
    message = String(a.acceleration.x, 2) + "," + String(a.acceleration.y, 2) + ",1";
  } else {
    // If not pressed, send zeros.
    message = "0,0,0";
  }
  
  // Transmit the message over WiFi to the receiver.
  WiFiClient client;
  if (client.connect(serverIP, serverPort)) {
    client.println(message);
    client.stop();
  } else {
    Serial.println("Failed to connect to receiver");
  }
  
  // Print the message to Serial for debugging.
  Serial.print("Sent: ");
  Serial.println(message);
  
  delay(5);  
}
