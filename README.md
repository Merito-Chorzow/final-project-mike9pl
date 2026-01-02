[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/XfC33h1O)

# Sygnalizacja świetlna (ESP32 + Wokwi/PlatformIO)

Projekt symuluje system sygnalizacji świetlnej dla pieszych i pojazdów, wykorzystując ESP32 oraz środowisko Wokwi z PlatformIO. 
System obsługuje przycisk dla pieszych, który inicjuje sekwencję zmiany świateł, a także tryb bezpieczny z "migającym" żółtym światłem.


## Wymagania

- Docker (dla devcontainer)
- VS Code 
- wtyczka PlatformIO
- wtyczka Wokwi

## Setup

1. Otwórz repo w VS Code.
1. Otwórz repo w devcontainerze Docker (zalecane dla spójnego środowiska).
1. Zainstaluj wymagane wtyczki (PlatformIO, Wokwi).
1. PlatformIO przy pierwszym buildzie pobierze wymagane narzędzia dla ESP32.
1. Wokwi przy pierwszym uruchomieniu wymaga licencji (postępować według instrukcji wtyczki).

## Build

- `Rozszerzenie PlatformIO -> "Nazwa Projektu" -> General -> Build`
- lub F5 w VSCode

Po udanym buildzie Wokwi oczekuje artefaktów:

- `.pio/build/esp32dev/firmware.bin`
- `.pio/build/esp32dev/firmware.elf`

## Uruchomienie w Wokwi

Wokwi korzysta z plików `.pio/build/...` zdefiniowanych w `wokwi/wokwi.toml`, więc build jest wymagany przed uruchomieniem symulacji.

1. Uruchom symulację z pliku `wokwi/diagram.json` (Wokwi w VS Code).
1. Kliknij "Start Simulation".
1. Otwórz Serial Monitor w Wokwi
1. Interakcja z przyciskiem w symulacji. 
1. Obserwuj logi.

## Mapowanie GPIO (Wokwi)

- Przycisk: `GPIO2` (INPUT_PULLUP, podłączony do GND)
- Światła główne: czerwony `GPIO5`, żółty `GPIO4`, zielony `GPIO0`
- Światła pieszych: czerwony `GPIO14`, zielony `GPIO12`

## Uruchomienie na płytce (ESP32)

1. Podłącz ESP32 po USB.
2. Wgraj firmware:
  - `platformio run -e esp32dev -t upload`
3. Otwórz monitor UART (baud 115200):
  - `platformio device monitor -b 115200`