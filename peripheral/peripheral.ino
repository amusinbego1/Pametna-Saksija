#include <ArduinoBLE.h>
#include <Arduino_HTS221.h>

BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214"); // Bluetooth® Low Energy LED Service
BLEService sensorService("5a005939-6dad-4166-9531-2d8d363a462c");

// Bluetooth® Low Energy LED Switch Characteristic - custom 128-bit UUID, read and writable by central
BLEByteCharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);
BLEByteCharacteristic temperatureCharacteristic("91a0b53d-0624-4b15-b388-59afcf03f233", BLERead);
BLEByteCharacteristic humidityCharacteristic("89c028a0-d1bf-4f8f-97d6-3fa8c77fdcf7", BLERead);

const int ledPin = LED_BUILTIN; // pin to use for the LED

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // set LED pin to output mode
  pinMode(ledPin, OUTPUT);

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");
    while (1);
  }
  if (!HTS.begin()) {
    Serial.println("Failed to initialize humidity temperature sensor!");
    while (1);
  }

  // set advertised local name and service UUID:
  BLE.setLocalName("Peripheral");
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

  Serial.println("BLE LED Peripheral");
}

void loop() {
  // listen for Bluetooth® Low Energy peripherals to connect:
  BLEDevice central = BLE.central();

  // if a central is connected to peripheral:
  if (central) {
    Serial.print("Connected to central: ");
    // print the central's MAC address:
    Serial.println(central.address());

    // while the central is still connected to peripheral:
    while (central.connected()) {
      // if the remote device wrote to the characteristic,
      // use the value to control the LED:
      if (switchCharacteristic.written()) {
        if (switchCharacteristic.value()) {   // any value other than 0
          temperatureCharacteristic.writeValue(HTS.readTemperature());
          humidityCharacteristic.writeValue(HTS.readHumidity());
          digitalWrite(ledPin, HIGH);         // will turn the LED on
        } else {                              // a 0 value
          digitalWrite(ledPin, LOW);          // will turn the LED off
        }
      }
    }

    // when the central disconnects, print it out:
    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());
  }
}