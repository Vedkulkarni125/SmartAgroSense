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

struct __attribute__((packed)) SensorData {
  char nodeID[3];
  float temperature;
  float humidity;
  int soilMoisture;
  bool relayState;
};

struct __attribute__((packed)) ControlPacket {
  char nodeID[3];
  bool relayControl;
};

bool relayControlFromApp = false;

BLYNK_WRITE(V9) {
  relayControlFromApp = param.asInt();
  Serial.print("📲 Blynk command → Relay Node 3: ");
  Serial.println(relayControlFromApp ? "ON" : "OFF");

  ControlPacket ctrl;
  strcpy(ctrl.nodeID, "N3");
  ctrl.relayControl = relayControlFromApp;

  radio.stopListening();
  radio.openWritingPipe("NODE3");
  bool sent = radio.write(&ctrl, sizeof(ctrl));
  radio.startListening();

  Serial.println(sent ? "✅ Relay control sent to N3" : "❌ Send failed");
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, pass);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  if (!radio.begin()) {
    Serial.println("❌ NRF24L01 not responding");
    while (1);
  }

  radio.setPALevel(RF24_PA_HIGH);
  radio.setChannel(108);
  radio.openReadingPipe(1, baseAddress);
  radio.startListening();

  Serial.println("✅ Receiver ready with Blynk control");
}

void loop() {
  Blynk.run();

  if (radio.available()) {
    SensorData data;
    radio.read(&data, sizeof(data));

    Serial.printf("📡 %s → Temp: %.1f°C | Hum: %.1f%% | Soil: %d | Relay: %s\n",
                  data.nodeID, data.temperature, data.humidity, data.soilMoisture,
                  data.relayState ? "ON" : "OFF");

    if (strcmp(data.nodeID, "N1") == 0) {
      Blynk.virtualWrite(V0, data.temperature);
      Blynk.virtualWrite(V1, data.humidity);
      Blynk.virtualWrite(V2, data.soilMoisture);
    } else if (strcmp(data.nodeID, "N2") == 0) {
      Blynk.virtualWrite(V3, data.temperature);
      Blynk.virtualWrite(V4, data.humidity);
      Blynk.virtualWrite(V5, data.soilMoisture);
    } else if (strcmp(data.nodeID, "N3") == 0) {
      Blynk.virtualWrite(V6, data.temperature);
      Blynk.virtualWrite(V7, data.humidity);
      Blynk.virtualWrite(V8, data.soilMoisture);
      Blynk.virtualWrite(V9, data.relayState);
    }
  }
}
