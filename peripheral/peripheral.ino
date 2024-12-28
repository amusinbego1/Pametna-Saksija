#include <ArduinoBLE.h>

BLEService newService("180A");  // creating the service
BLEUnsignedCharCharacteristic randomReading("2A58", BLERead | BLENotify);  // Analog Value characteristic
BLEByteCharacteristic switchChar("2A57", BLERead | BLEWrite);  // LED characteristic

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

  BLE.setLocalName("Pametna Saksija");  // Set a name for the BLE device
  BLE.setAdvertisedService(newService);

  newService.addCharacteristic(switchChar);
  newService.addCharacteristic(randomReading);

  BLE.addService(newService);

  switchChar.writeValue(0);
  randomReading.writeValue(0);

  BLE.advertise();  // Start advertising
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

        int randomValue = analogRead(A1);
        randomReading.writeValue(randomValue);

        if (switchChar.written()) {
          if (switchChar.value()) {
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
