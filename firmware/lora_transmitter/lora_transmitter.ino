#include <SPI.h>
#include <LoRa.h>

// =============================
// ESP32-S3 N16R8 LoRa pins
// =============================
#define LORA_SCK   12
#define LORA_MISO  14
#define LORA_MOSI  13
#define LORA_SS    10
#define LORA_RST   9
#define LORA_DIO0  8

int packetNumber = 0;

void setup() {

  Serial.begin(115200);
  delay(2000);

  Serial.println();
  Serial.println("================================");
  Serial.println("ESP32-S3 N16R8 - LoRa TRANSMITTER");
  Serial.println("================================");

  // Start SPI
  SPI.begin(
    LORA_SCK,
    LORA_MISO,
    LORA_MOSI,
    LORA_SS
  );

  // Set LoRa pins
  LoRa.setPins(
    LORA_SS,
    LORA_RST,
    LORA_DIO0
  );

  Serial.println("Starting LoRa...");

  // SX1278 = 433 MHz
  if (!LoRa.begin(433E6)) {

    Serial.println("❌ LoRa initialization FAILED!");

    while (1) {
      delay(1000);
    }
  }

  Serial.println("✅ LoRa initialized successfully!");
  Serial.println("Frequency: 433 MHz");
  Serial.println("Starting transmission...");
  Serial.println();
}

void loop() {

  packetNumber++;

  String message = "Hello from N16R8 | Packet: ";
  message += packetNumber;

  Serial.print("Sending: ");
  Serial.println(message);

  // Start packet
  LoRa.beginPacket();

  // Send message
  LoRa.print(message);

  // Finish and transmit
  LoRa.endPacket();

  Serial.println("✅ Packet sent!");
  Serial.println();

  delay(2000);
}
