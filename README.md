# Health BLE Peripheral – Arduino Nano 33 BLE Sense

Ovaj projekt implementira BLE peripheral uređaj nazvan **"Health"**, koji omogućava slanje podataka sa senzora temperature i vlažnosti, uz kontrolu pristupa pomoću jednostavne hash autentifikacije. Uređaj je spreman za korištenje bez dodatne konfiguracije – dovoljno je dovesti napajanje.

## 🔧 Hardver

- Arduino Nano 33 BLE Sense
- Ugrađeni HTS221 senzor (temperatura i vlažnost)
- Ugrađena LED (LED_BUILTIN)
- Napajanje putem USB-a, baterije ili powerbanka

## 🟢 Aktivacija

1. Uključite uređaj napajanjem (USB, baterija, powerbank).
2. Uređaj se automatski pokreće i počinje emitovati BLE signal pod imenom **`Health`**.
3. Koristite BLE aplikaciju (npr. **nRF Connect**) za povezivanje.

## 🔐 Autentifikacija i Karakteristike

| Servis | UUID | Karakteristike |
|--------|------|----------------|
| `sensorService` | `5a005939-...` | - `temperatureCharacteristic` (read)<br>- `humidityCharacteristic` (read) |
| `authService`   | `9964e111-...` | - `passkeyCharacteristic` (write) – za unos šifre |

### ➕ Kako se autentificirati:

1. Povežite se na uređaj **Health** putem BLE aplikacije (npr. nRF Connect).
2. Pronađite karakteristiku `passkeyCharacteristic`.
3. Unesite sljedeći passkey: `123456`
4. Ako je unos tačan:
   - LED prestaje treptati i ostaje stalno upaljena
   - Temperatura i vlažnost su dostupni za čitanje
5. Ako je passkey pogrešan:
   - Broj pokušaja se povećava
   - Nakon 3 pogrešna unosa, veza se automatski prekida

## 💡 LED Signalizacija

| Stanje uređaja             | LED ponašanje |
|---------------------------|----------------|
| Uređaj nije povezan       | Ugašena        |
| Povezan, nije autentifikovan | Treperi svake sekunde |
| Autentifikovan            | Stalno upaljena |

## 📱 Preporučene aplikacije za testiranje

- [nRF Connect for Mobile (Android/iOS)](https://www.nordicsemi.com/Products/Development-tools/nRF-Connect-for-mobile)
- LightBlue (iOS, Android)

## 📦 Potrebne biblioteke (za razvojere)

Ako želiš samostalno modificirati kod, instaliraj sljedeće biblioteke u Arduino IDE:

- `ArduinoBLE`
- `Arduino_HTS221`
- `Crypto by Rhys Weatherley`

## 📁 Struktura

