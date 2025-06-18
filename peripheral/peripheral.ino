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
BLEStringCharacteristic jsonCharacteristic("6fddf51b-4e44-4ff6-bc27-13462d5cdb0b", BLERead, 64);

// LED i treptanje
const int ledPin = LED_BUILTIN;
bool ledState = false;
bool shouldBlink = false;
unsigned long ledPreviousMillis = 0;
const unsigned long ledInterval = 500;

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

void onTemperatureRead(BLEDevice central, BLECharacteristic characteristic);
void onHumidityRead(BLEDevice central, BLECharacteristic characteristic);
void onJSONRead(BLEDevice central, BLECharacteristic characteristic);
void onPasskeyWritten(BLEDevice central, BLECharacteristic characteristic);

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

  BLEDescriptor tempDesc("2901", "Temperatura");
  BLEDescriptor humDesc("2901", "Vlažnost");
  BLEDescriptor jsonDesc("2901", "JSON");
  BLEDescriptor passDesc("2901", "Šifra");

  BLE.setLocalName("Health");
  BLE.setAdvertisedService(sensorService);
  BLE.setAdvertisedService(authService);

  temperatureCharacteristic.addDescriptor(tempDesc);
  humidityCharacteristic.addDescriptor(humDesc);
  jsonCharacteristic.addDescriptor(jsonDesc);
  passkeyCharacteristic.addDescriptor(passDesc);

  temperatureCharacteristic.setEventHandler(BLERead, onTemperatureRead);
  humidityCharacteristic.setEventHandler(BLERead, onHumidityRead);
  jsonCharacteristic.setEventHandler(BLERead, onJSONRead);
  passkeyCharacteristic.setEventHandler(BLEWritten, onPasskeyWritten);

  sensorService.addCharacteristic(temperatureCharacteristic);
  sensorService.addCharacteristic(humidityCharacteristic);
  sensorService.addCharacteristic(jsonCharacteristic);
  authService.addCharacteristic(passkeyCharacteristic);

  BLE.addService(sensorService);
  BLE.addService(authService);

  temperatureCharacteristic.writeValue("No Value");
  humidityCharacteristic.writeValue("No Value");
  jsonCharacteristic.writeValue("{}");
  passkeyCharacteristic.writeValue("");

  BLE.advertise();
  Serial.println("BLE device active. Waiting for connection...");
}

void loop() {
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());

    authenticated = false;
    authAttempts = 0;
    shouldBlink = true;
    ledPreviousMillis = millis();

    while (central.connected()) {
      BLE.poll();

      if (!authenticated && shouldBlink) {
        unsigned long now = millis();
        if (now - ledPreviousMillis >= ledInterval) {
          ledPreviousMillis = now;
          ledState = !ledState;
          digitalWrite(ledPin, ledState);
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

void onTemperatureRead(BLEDevice central, BLECharacteristic characteristic) {
  if (authenticated) {
    float temp = HTS.readTemperature();
    characteristic.writeValue(String(temp).c_str());
  } else {
    characteristic.writeValue("No Value");
  }
}

void onHumidityRead(BLEDevice central, BLECharacteristic characteristic) {
  if (authenticated) {
    float hum = HTS.readHumidity();
    characteristic.writeValue(String(hum).c_str());
  } else {
    characteristic.writeValue("No Value");
  }
}

void onJSONRead(BLEDevice central, BLECharacteristic characteristic) {
  if (authenticated) {
    float temp = HTS.readTemperature();
    float hum = HTS.readHumidity();
    String json = "{\"temperature\": " + String(temp, 1) + ", \"humidity\": " + String(hum, 1) + "}";
    characteristic.writeValue(json.c_str());
  } else {
    characteristic.writeValue("{}");
  }
}

void onPasskeyWritten(BLEDevice central, BLECharacteristic characteristic) {
  String inputHash = getHash(passkeyCharacteristic.value());

  if (inputHash == correctHash) {
    authenticated = true;
    authAttempts = 0;
    shouldBlink = false;
    digitalWrite(ledPin, HIGH);
    Serial.println("Auth success!");
  } else {
    authAttempts++;
    Serial.print("Auth failed! Attempt ");
    Serial.println(authAttempts);

    if (authAttempts >= maxAuthAttempts) {
      shouldBlink = false;
      digitalWrite(ledPin, LOW);
      Serial.println("Too many attempts. Disconnecting...");
      BLE.disconnect();
    }
  }
}
