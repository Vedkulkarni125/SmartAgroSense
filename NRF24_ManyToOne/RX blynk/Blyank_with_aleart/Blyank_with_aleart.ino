#define BLYNK_TEMPLATE_ID "TMPL3yUY-7bFc"
#define BLYNK_TEMPLATE_NAME "Smart AgroSense"
char auth[] = "QJMStz-7zalzFVn-pUDBgu7w-s6Lyour";
char ssid[] = "Airtel_divy_8199";
char pass[] = "air50219";




#include <SPI.h>
#include <RF24.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>


RF24 radio(22, 21);  // CE, CSN
const byte baseAddress[6] = "BASE1";

// Data structure from nodes
struct __attribute__((packed)) SensorData {
  char nodeID[3];
  float temperature;
  float humidity;
  int soilMoisture;
  bool relayState;
};

// Alert flags
bool alertN1Sent = false;
bool alertN2Sent = false;
bool alertN3Sent = false;

const int SOIL_THRESHOLD = 500;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, pass);
  Blynk.begin(auth, ssid, pass);

  if (!radio.begin()) {
    Serial.println("❌ NRF24L01 not responding");
    while (1);
  }

  radio.setPALevel(RF24_PA_HIGH);
  radio.setChannel(108);
  radio.openReadingPipe(1, baseAddress);
  radio.startListening();

  Serial.println("✅ Receiver ready (no relay control)");
}

void loop() {
  Blynk.run();

  if (radio.available()) {
    SensorData data;
    radio.read(&data, sizeof(data));

    Serial.printf("📡 %s → Temp: %.1f°C | Hum: %.1f%% | Soil: %d\n",
      data.nodeID, data.temperature, data.humidity, data.soilMoisture);

    // === Node 1 ===
    if (strcmp(data.nodeID, "N1") == 0) {
      Blynk.virtualWrite(V0, data.temperature);
      Blynk.virtualWrite(V1, data.humidity);
      Blynk.virtualWrite(V2, data.soilMoisture);

      if (data.soilMoisture < SOIL_THRESHOLD && !alertN1Sent) {
        Blynk.logEvent("low_moisture", "⚠️ Node 1: Soil is too dry!");
        alertN1Sent = true;
      }
      if (data.soilMoisture >= SOIL_THRESHOLD) alertN1Sent = false;
    }

    // === Node 2 ===
    else if (strcmp(data.nodeID, "N2") == 0) {
      Blynk.virtualWrite(V3, data.temperature);
      Blynk.virtualWrite(V4, data.humidity);
      Blynk.virtualWrite(V5, data.soilMoisture);

      if (data.soilMoisture < SOIL_THRESHOLD && !alertN2Sent) {
        Blynk.logEvent("low_moisture", "⚠️ Node 2: Soil is too dry!");
        alertN2Sent = true;
      }
      if (data.soilMoisture >= SOIL_THRESHOLD) alertN2Sent = false;
    }

    // === Node 3 ===
    else if (strcmp(data.nodeID, "N3") == 0) {
      Blynk.virtualWrite(V6, data.temperature);
      Blynk.virtualWrite(V7, data.humidity);
      Blynk.virtualWrite(V8, data.soilMoisture);

      if (data.soilMoisture < SOIL_THRESHOLD && !alertN3Sent) {
        Blynk.logEvent("low_moisture", "⚠️ Node 3: Soil is too dry!");
        alertN3Sent = true;
      }
      if (data.soilMoisture >= SOIL_THRESHOLD) alertN3Sent = false;
    }
  }
}
