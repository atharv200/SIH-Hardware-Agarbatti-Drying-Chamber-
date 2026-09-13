# ESP32 Firmware

Firmware for an ESP32-based aroma oil mist environmental control system. The ESP32 reads six DHT11 sensors, controls a heater and exhaust fan, operates an aroma mist sprayer, and sends average temperature and humidity to ThingSpeak.

## Pin Configuration

| Component | GPIO |
|---|---:|
| DHT11 Sensor 1 | 33 |
| DHT11 Sensor 2 | 32 |
| DHT11 Sensor 3 | 25 |
| DHT11 Sensor 4 | 26 |
| DHT11 Sensor 5 | 27 |
| DHT11 Sensor 6 | 14 |
| Heater | 16 |
| Exhaust | 17 |
| Spray Button | 18 |
| Pre-Spray Button | 19 |
| Post-Spray Button | 21 |
| Sprayer | 22 |

Buttons use `INPUT_PULLUP` and should be connected between the GPIO pin and GND.

## Modes

### Pre-Spray Mode
Default mode.

- Target: 50 ± 5 °C
- Below 45 °C → Heater ON
- Above 55 °C → Heater OFF
- 45–55 °C → Maintain previous heater state

### Post-Spray Mode

- Target: 35 ± 5 °C
- Below 30 °C → Heater ON
- Above 40 °C → Heater OFF
- 30–40 °C → Maintain previous heater state

## Buttons

- **Spray button:** Starts the sprayer for 5 seconds.
- **Pre-Spray button:** Switches to Pre-Spray mode.
- **Post-Spray button:** Switches to Post-Spray mode.

## Exhaust Control

- Average humidity > 70% → Exhaust ON
- Average humidity ≤ 70% → Exhaust OFF

## Sensors

Six DHT11 sensors are read every 2 seconds.

The firmware calculates:

- `avgT` → Average temperature
- `avgH` → Average humidity

All six sensor readings must be valid before updating the averages.

## ThingSpeak

Data is sent every 10 seconds.

- Field 1 → Average Temperature
- Field 2 → Average Humidity

## Timing

- Sensor reading: 2 seconds
- ThingSpeak update: 10 seconds
- Spray duration: 5 seconds

`millis()` is used for the main runtime timing.

## Required Libraries

- `WiFi.h`
- `HTTPClient.h`
- `DHT.h`
