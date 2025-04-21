#include <SPI.h>
#include <RF24.h>
#include <DHT.h>

#define DHTPIN 4
#define DHTTYPE DHT22
#define SOIL_PIN 34
#define RELAY_PIN 25

DHT dht(DHTPIN, DHTTYPE);
RF24 radio(22, 21);  // CE, CSN

const byte receiverAddress[6] = "BASE1";
const byte nodeAddress[6] = "NODE3";

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

bool currentRelayState = false;

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(RELAY_PIN, OUTPUT);

  if (!radio.begin()) {
    Serial.println("❌ NRF24L01 not responding");
    while (1);
  }

  radio.setPALevel(RF24_PA_HIGH);
  radio.setChannel(108);
  radio.openWritingPipe(receiverAddress);
  radio.openReadingPipe(1, nodeAddress);
  radio.startListening();

  Serial.println("✅ Node 3 with relay + control ready");
}

void loop() {
  // Check for incoming control commands
  ControlPacket ctrl;
  if (radio.available()) {
    radio.read(&ctrl, sizeof(ctrl));
    if (strcmp(ctrl.nodeID, "N3") == 0) {
      currentRelayState = ctrl.relayControl;
      digitalWrite(RELAY_PIN, currentRelayState);
      Serial.print("🔁 Relay controlled from Blynk: ");
      Serial.println(currentRelayState ? "ON" : "OFF");
    }
  }

  // Send sensor data
  SensorData data;
  strcpy(data.nodeID, "N3");
  data.temperature = dht.readTemperature();
  data.humidity = dht.readHumidity();
  data.soilMoisture = analogRead(SOIL_PIN);
  data.relayState = currentRelayState;

  radio.stopListening();
  bool sent = radio.write(&data, sizeof(data));
  radio.startListening();

  Serial.printf("Sending N3 → Temp: %.1f°C | Hum: %.1f%% | Soil: %d | Relay: %s\n",
                data.temperature, data.humidity, data.soilMoisture,
                data.relayState ? "ON" : "OFF");
  Serial.println(sent ? "✅ Sent!" : "❌ Send failed");

  delay(3000);
}
