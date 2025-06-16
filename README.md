# Health BLE Peripheral – Arduino Nano 33 BLE Sense

Ovaj projekt implementira BLE peripheral uređaj nazvan **"Health"**, koji omogućava slanje podataka sa senzora temperature i vlažnosti, uz kontrolu pristupa pomoću jednostavne hash autentifikacije. Uređaj je spreman za korištenje bez dodatne konfiguracije – dovoljno je dovesti napajanje.

## 🔧 Hardver

- Arduino Nano 33 BLE Sense
- Ugrađeni HTS221 senzor (temperatura i vlažnost)
- Ugrađena LED (LED_BUILTIN)
- Napajanje putem USB-a, baterije ili powerbanka

---

## 🔌 Napajanje uređaja

Uređaj koristi **Arduino Nano 33 BLE Sense**, koji se može napajati na više načina:

### 1. Putem USB kabla
- **Preporučeni način za testiranje**
- Koristi standardni **micro-USB kabel**
- Kada se priključi USB, uređaj automatski pokreće BLE oglašavanje

### 2. Putem baterije (npr. LiPo)
- Dovesti **3.7V – 6V** na **VIN** pin
- GND pin spoji na minus baterije

| Arduino pin | Spojiti na             |
|-------------|------------------------|
| VIN         | + baterije (3.7–6V)    |
| GND         | - baterije             |

### 3. Putem powerbanka
- Spoji USB kabel u powerbank
- Uređaj se odmah aktivira i oglašava BLE signal

### ⚠️ Važno
- **Ne spajati više izvora napajanja istovremeno!**
- **Ne koristiti 3.3V pin za napajanje** – to je izlazni pin, ne ulaz

---

## 🔗 BLE Servisi i Karakteristike

| Servis         | UUID                                  | Karakteristike                             |
|----------------|---------------------------------------|--------------------------------------------|
| `sensorService`| `5a005939-6dad-4166-9531-2d8d363a462c`| - `temperatureCharacteristic` (read) <br> - `humidityCharacteristic` (read) |
| `authService`  | `9964e111-9289-4507-b935-c321bea0afbe`| - `passkeyCharacteristic` (write) – unos šifre |

---

## 🔐 Autentifikacija

- Passkey: `123456` (hashiran pomoću SHA256)
- Nakon 3 pogrešna unosa → veza se automatski prekida
- Autentifikacija je obavezna da bi se očitali pravi podaci sa senzora

### Postupak:
1. Poveži se na uređaj **Health** putem BLE aplikacije
2. Pronađi karakteristiku `passkeyCharacteristic`
3. Unesi `"123456"` kao šifru
4. Ako je tačno → LED ostaje upaljena i podaci su dostupni
5. Ako je netačno → nakon 3 pokušaja, veza se prekida

---

## 💡 LED Signalizacija

| Stanje uređaja               | LED ponašanje          |
|-----------------------------|------------------------|
| Diskonektovan               | Ugašena                |
| Povezan, nije autentifikovan| Treperi svake sekunde  |
| Autentifikovan              | Stalno upaljena        |

---

## 📱 Testiranje putem BLE aplikacije

Preporučene aplikacije:


- **LightBlue (iOS, Android)
- [nrf connect for mobile](https://www.nordicsemi.com/products/development-tools/nrf-connect-for-mobile) (Android/iOS)

---

## 📁 Struktura projekta

```
Pametna-Saksija/
│
├── peripheral/
│ └── peripheral.ino // Glavni BLE kod (već učitan na Arduino)
├── README.md 
```
