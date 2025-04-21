#include <SPI.h>
#include <RF24.h>

RF24 radio(22, 21);  // CE, CSN
const byte baseAddress[6] = "BASE1";

struct __attribute__((packed)) SensorData {
  char nodeID[3];
  float temperature;
  float humidity;
  int soilMoisture;
  bool relayState;
};

void setup() {
  Serial.begin(115200);
  delay(1000);

  if (!radio.begin()) {
    Serial.println("❌ NRF24L01 not responding");
    while (1);
  }

  radio.setPALevel(RF24_PA_HIGH);
  radio.setChannel(108);
  radio.openReadingPipe(1, baseAddress);
  radio.startListening();

  Serial.println("✅ Receiver ready (many-to-one)");
}

void loop() {
  if (radio.available()) {
    SensorData data;
    radio.read(&data, sizeof(data));

    Serial.printf("📡 %s → Temp: %.1f°C | Hum: %.1f%% | Soil: %d",
                  data.nodeID, data.temperature, data.humidity, data.soilMoisture);

    // Only Node 3 has relay
    if (strcmp(data.nodeID, "N3") == 0) {
      Serial.print(" | Relay: ");
      Serial.println(data.relayState ? "ON" : "OFF");
    } else {
      Serial.println(" | Relay: N/A");
    }
  }
}
