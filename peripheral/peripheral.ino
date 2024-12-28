#include <ArduinoBLE.h>
#include <Arduino_HTS221.h>

BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214"); // Bluetooth® Low Energy LED Service
BLEService sensorService("5a005939-6dad-4166-9531-2d8d363a462c");

// Bluetooth® Low Energy LED Switch Characteristic - custom 128-bit UUID, read and writable by central
BLEByteCharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);
BLEByteCharacteristic temperatureCharacteristic("91a0b53d-0624-4b15-b388-59afcf03f233", BLERead);
BLEByteCharacteristic humidityCharacteristic("89c028a0-d1bf-4f8f-97d6-3fa8c77fdcf7", BLERead);

const int ledPin = LED_BUILTIN;
long previousMillis = 0;

void setup() {
  Serial.begin(9600);  // initialize serial communication
  while (!Serial);

  pinMode(ledPin, OUTPUT);       // initialize the LED pin

  // Initialize ArduinoBLE library
  if (!BLE.begin()) {
    Serial.println("Starting BLE failed!");
    while (1);
  }
  if (!HTS.begin()) {
    Serial.println("Failed to initialize humidity temperature sensor!");
    while (1);
  }

    // set advertised local name and service UUID:
  BLE.setLocalName("Health");
  BLE.setAdvertisedService(ledService);
  BLE.setAdvertisedService(sensorService);

  // add the characteristic to the service
  ledService.addCharacteristic(switchCharacteristic);
  sensorService.addCharacteristic(temperatureCharacteristic);
  sensorService.addCharacteristic(humidityCharacteristic);

  // add service
  BLE.addService(ledService);
  BLE.addService(sensorService);

  // set the initial value for the characeristic:
  switchCharacteristic.writeValue(0);

  // start advertising
  BLE.advertise();
  Serial.println("Bluetooth device active, waiting for connections...");
}

void loop() {
  BLEDevice central = BLE.central();  // Wait for a central to connect

  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());

    while (central.connected()) {
      BLE.poll();  // Keep BLE stack running

      long currentMillis = millis();
      if (currentMillis - previousMillis >= 200) {
        previousMillis = currentMillis;

        temperatureCharacteristic.writeValue(HTS.readTemperature());
        humidityCharacteristic.writeValue(HTS.readHumidity());

        if (switchCharacteristic.written()) {
          if (switchCharacteristic.value()) {
            Serial.println("LED on");
            digitalWrite(ledPin, HIGH);
          } else {
            Serial.println("LED off");
            digitalWrite(ledPin, LOW);
          }
        }
      }
    }
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }

  BLE.poll();  // Keep BLE stack running when no central is connected
}
