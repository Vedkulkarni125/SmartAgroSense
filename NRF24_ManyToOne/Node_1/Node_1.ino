#include <SPI.h>
#include <RF24.h>
#include <DHT.h>

#define DHTPIN 4
#define DHTTYPE DHT22
#define SOIL_PIN 34
//#define RELAY_PIN 25

DHT dht(DHTPIN, DHTTYPE);
RF24 radio(22, 21);  // CE, CSN

const char NODE_ID[] = "N1";
const byte receiverAddress[6] = "BASE1";

struct __attribute__((packed)) SensorData {
  char nodeID[3];
  float temperature;
  float humidity;
  int soilMoisture;
 // bool relayState;
};

void setup() {
  Serial.begin(115200);
  dht.begin();
  //pinMode(RELAY_PIN, OUTPUT);

  if (!radio.begin()) {
    Serial.println("❌ NRF24L01 not responding");
    while (1);
  }

  radio.setPALevel(RF24_PA_HIGH);
  radio.setChannel(108);
  radio.openWritingPipe(receiverAddress);
  radio.stopListening();

  Serial.print("✅ Transmitter "); Serial.print(NODE_ID); Serial.println(" ready");
}

void loop() {
  SensorData data;
  strcpy(data.nodeID, NODE_ID);
  data.temperature = dht.readTemperature();
  data.humidity = dht.readHumidity();
  data.soilMoisture = analogRead(SOIL_PIN);
  //data.relayState = data.soilMoisture < 500;
  //digitalWrite(RELAY_PIN, data.relayState);

  Serial.printf("Sending %s → Temp: %.1f°C | Hum: %.1f%% | Soil: %d | ",data.nodeID, data.temperature, data.humidity, data.soilMoisture);

  bool sent = radio.write(&data, sizeof(data));
  Serial.println(sent ? "✅ Sent!" : "❌ Send failed");

  delay(random(2000, 3000));
}
