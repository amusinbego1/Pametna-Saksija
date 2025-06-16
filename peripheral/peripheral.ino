#include <ArduinoBLE.h>
#include <Arduino_HTS221.h>
#include <Crypto.h>
#include <SHA256.h>

// BLE konfiguracija
const char *correctPasskey = "123456";
bool authenticated = false;
int authAttempts = 0;
const int maxAuthAttempts = 3;

// BLE servisi i karakteristike
BLEService sensorService("5a005939-6dad-4166-9531-2d8d363a462c");
BLEService authService("9964e111-9289-4507-b935-c321bea0afbe");

BLEStringCharacteristic temperatureCharacteristic("91a0b53d-0624-4b15-b388-59afcf03f233", BLERead, 8);
BLEStringCharacteristic humidityCharacteristic("89c028a0-d1bf-4f8f-97d6-3fa8c77fdcf7", BLERead, 8);
BLEStringCharacteristic passkeyCharacteristic("30ead979-dd63-4fe5-a2ca-e76ae9ce0c9c", BLEWrite, 64);

// LED i treptanje
const int ledPin = LED_BUILTIN;
bool ledState = false;
bool shouldBlink = false;
unsigned long ledPreviousMillis = 0;
const unsigned long ledInterval = 1000; // 1s

// hash funkcija
String getHash(const String& input) {
  SHA256 hasher;
  hasher.reset();
  hasher.update(input.c_str(), input.length());
  byte hash[32];
  hasher.finalize(hash, sizeof(hash));
  String result = "";
  for (int i = 0; i < 4; i++) {
    result += String(hash[i], HEX);
  }
  return result;
}

String correctHash = getHash(correctPasskey);

long previousMillis = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  if (!BLE.begin()) {
    Serial.println("BLE init failed!");
    while (1);
  }

  if (!HTS.begin()) {
    Serial.println("Sensor init failed!");
    while (1);
  }

  BLE.setLocalName("Health");
  BLE.setAdvertisedService(sensorService);
  BLE.setAdvertisedService(authService);

  sensorService.addCharacteristic(temperatureCharacteristic);
  sensorService.addCharacteristic(humidityCharacteristic);
  authService.addCharacteristic(passkeyCharacteristic);

  BLE.addService(sensorService);
  BLE.addService(authService);

  passkeyCharacteristic.writeValue("");

  BLE.advertise();
  Serial.println("BLE device active. Waiting for connection...");
}

void loop() {
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());

    authAttempts = 0;
    authenticated = false;
    shouldBlink = true;
    ledPreviousMillis = millis();

    while (central.connected()) {

      // TREPTANJE LED ako nije autentifikovan
      if (!authenticated && shouldBlink) {
        unsigned long now = millis();
        if (now - ledPreviousMillis >= ledInterval) {
          ledPreviousMillis = now;
          ledState = !ledState;
          digitalWrite(ledPin, ledState);
        }
      }

      // Provjera passkey-a
      if (passkeyCharacteristic.written()) {
        String inputHash = getHash(passkeyCharacteristic.value());

        if (inputHash == correctHash) {
          authenticated = true;
          authAttempts = 0;
          shouldBlink = false;
          digitalWrite(ledPin, HIGH);
          Serial.println("✅ Auth success!");
        } else {
          authAttempts++;
          Serial.print("❌ Auth failed! Attempt ");
          Serial.println(authAttempts);

          if (authAttempts >= maxAuthAttempts) {
            shouldBlink = false;
            digitalWrite(ledPin, LOW);
            Serial.println("⛔ Too many attempts. Disconnecting...");
            central.disconnect();
            break;
          }
        }
      }

      // Slanje podataka
      long currentMillis = millis();
      if (currentMillis - previousMillis >= 200) {
        previousMillis = currentMillis;

        if (authenticated) {
          temperatureCharacteristic.writeValue(String(HTS.readTemperature()));
          humidityCharacteristic.writeValue(String(HTS.readHumidity()));
        } else {
          temperatureCharacteristic.writeValue("No Value");
          humidityCharacteristic.writeValue("No Value");
          Serial.println("🔒 Access denied.");
        }
      }
    }

    // Nakon diskonekcije
    authenticated = false;
    authAttempts = 0;
    shouldBlink = false;
    digitalWrite(ledPin, LOW);
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }
}
