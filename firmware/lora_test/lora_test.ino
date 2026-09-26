#include <SPI.h>
#include <LoRa.h>

// ESP32-S3 N16R8 pins
#define LORA_SCK   12
#define LORA_MISO  14
#define LORA_MOSI  13
#define LORA_SS    10
#define LORA_RST   9
#define LORA_DIO0  8

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println();
  Serial.println("================================");
  Serial.println("ESP32-S3 + SX1278 LoRa Test");
  Serial.println("================================");

  // Start SPI with our selected pins
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);

  // Tell LoRa library which pins are being used
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  Serial.println("Starting LoRa...");

  // SX1278 / LoRa-02 = 433 MHz
  if (!LoRa.begin(433E6)) {
    Serial.println("❌ LoRa initialization FAILED!");
    Serial.println("Check wiring and power.");

    while (1) {
      delay(1000);
    }
  }

  Serial.println("✅ LoRa initialization SUCCESS!");
  Serial.println("✅ SX1278 detected successfully.");
  Serial.println("Frequency: 433 MHz");
}

void loop() {
  Serial.println("LoRa module is working...");
  delay(2000);
}
