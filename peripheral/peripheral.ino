#include <ArduinoBLE.h>
#include <Arduino_HTS221.h>

const char *passkey = "123456";
bool authenticated = false;

BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214"); // Bluetooth® Low Energy LED Service
BLEService sensorService("5a005939-6dad-4166-9531-2d8d363a462c");
BLEService authService("9964e111-9289-4507-b935-c321bea0afbe");

// Bluetooth® Low Energy LED Switch Characteristic - custom 128-bit UUID, read and writable by central
BLEByteCharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);
BLEStringCharacteristic temperatureCharacteristic("91a0b53d-0624-4b15-b388-59afcf03f233", BLERead, 8);
BLEStringCharacteristic humidityCharacteristic("89c028a0-d1bf-4f8f-97d6-3fa8c77fdcf7", BLERead, 8);
BLEStringCharacteristic passkeyCharacteristic("30ead979-dd63-4fe5-a2ca-e76ae9ce0c9c", BLEWrite, 8);

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
  BLE.setAdvertisedService(authService);

  // add the characteristic to the service
  ledService.addCharacteristic(switchCharacteristic);
  sensorService.addCharacteristic(temperatureCharacteristic);
  sensorService.addCharacteristic(humidityCharacteristic);
  authService.addCharacteristic(passkeyCharacteristic);

  // add service
  BLE.addService(ledService);
  BLE.addService(sensorService);
  BLE.addService(authService);

  // set the initial value for the characeristic:
  switchCharacteristic.writeValue(0);
  passkeyCharacteristic.writeValue("");

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
      if (passkeyCharacteristic.written()){
        if (passkeyCharacteristic.value() == passkey) {
          authenticated = true;
          Serial.println("Authenticated!");
        } else {
          authenticated = false;
          Serial.println("Authentication failed!");
        }
      }

      long currentMillis = millis();
      if (currentMillis - previousMillis >= 200) {
        previousMillis = currentMillis;

        if(authenticated){
          temperatureCharacteristic.writeValue(String(HTS.readTemperature()));
          humidityCharacteristic.writeValue(String(HTS.readHumidity()));

          if (switchCharacteristic.written()) {
            if (switchCharacteristic.value()) {
              Serial.println("LED on");
              digitalWrite(ledPin, HIGH);
            } else {
              Serial.println("LED off");
              digitalWrite(ledPin, LOW);
            }
          }
        } else {
          digitalWrite(ledPin, LOW);
          temperatureCharacteristic.writeValue("No Value");
          humidityCharacteristic.writeValue("No Value");
          Serial.println("Access to sensor data is blocked");
        }
      }
    }
    authenticated = false;
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }

  //BLE.poll();  // Keep BLE stack running when no central is connected
}
