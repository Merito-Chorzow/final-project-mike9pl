# tests.md — Scenariusze testów i wyniki

## Jak uruchomić testy

### PlatformIO

VSCode:

Rozszerzenie PlatformIO -> "Nazwa Projektu" -> Advanced -> Test


## Lista testów (scenariusze)

### T1 — Unit: `set_safeMode_led()` wyłącza wyjścia

- Plik: `test/test.cpp`
- Cel: sprawdzić, że `set_safeMode_led()` ustawia wszystkie istotne piny LED na stan LOW.
- Kroki:
  1. Inicjalizacja pinów przez `led_pin_init()` (jak w produkcji).
  2. Wywołanie `set_safeMode_led()`.
  3. Asercje: wszystkie piny LED powinny być LOW.

### T2 — Manual: start systemu i logi

- Środowisko: Wokwi lub realna płytka
- Kroki:
  1. Uruchom firmware.
  2. Sprawdź UART 115200.
- Oczekiwane:
  - `SYS: Tasks started`
  - `SYSTEM READY`
  - FSM startuje w `INIT` i przechodzi do `IDLE`.

### T3 — Manual: krótki klik (ShortPress) uruchamia RUN

- Kroki:
  1. Będąc w `IDLE`, naciśnij i puść przycisk krócej niż 3 s.
- Oczekiwane logi:
  - `BTN->FSM: ShortPress`
  - `SYS: RUN MODE`
  - po timeoutach faz pieszych: powrót do `SYS: IDLE MODE`

### T4 — Manual: długie przytrzymanie (LongPress) przełącza SAFE ↔ IDLE

- Kroki:
  1. Przytrzymaj przycisk ≥ 3000 ms.
  2. Powtórz przytrzymanie ≥ 3000 ms.
- Oczekiwane:
  - `BTN->FSM: LongPress`
  - `SYS: SAFE MODE` (pierwsze przytrzymanie)
  - `LED: YELLOW BLINK` cyklicznie
  - `SYS: IDLE MODE` (drugie przytrzymanie)

### T5 — Manual: cykl faz IDLE

- Kroki:
  1. Pozostaw urządzenie w `IDLE`.
- Oczekiwane:
  - przełączenia faz w logach co kilka sekund, np.:
    - `LED: MAIN RED -> MAIN YELLOW`
    - `LED: MAIN YELLOW -> MAIN GREEN`
    - `LED: MAIN GREEN -> MAIN YELLOW`
    - `LED: MAIN YELLOW -> MAIN RED`

## Wyniki (logi)

### Wynik: build (terminal PlatformIO)

```text
Verbose mode can be enabled via `-v, --verbose` option
CONFIGURATION: https://docs.platformio.org/page/boards/espressif32/esp32dev.html
PLATFORM: Espressif 32 (6.12.0) > Espressif ESP32 Dev Module
HARDWARE: ESP32 240MHz, 320KB RAM, 4MB Flash
DEBUG: Current (cmsis-dap) External (cmsis-dap, esp-bridge, esp-prog, iot-bus-jtag, jlink, minimodule, olimex-arm-usb-ocd, olimex-arm-usb-ocd-h, olimex-arm-usb-tiny-h, olimex-jtag-tiny, tumpa)
PACKAGES: 
 - framework-arduinoespressif32 @ 3.20017.241212+sha.dcc1105b 
 - tool-esptoolpy @ 2.40900.250804 (4.9.0) 
 - toolchain-xtensa-esp32 @ 8.4.0+2021r2-patch5
LDF: Library Dependency Finder -> https://bit.ly/configure-pio-ldf
LDF Modes: Finder ~ chain, Compatibility ~ soft
Found 34 compatible libraries
Scanning dependencies...
No dependencies
Building in debug mode
Compiling .pio/build/esp32dev/src/app/fsm.cpp.o
Compiling .pio/build/esp32dev/src/app/task.cpp.o
Compiling .pio/build/esp32dev/src/drivers/button.cpp.o
Compiling .pio/build/esp32dev/src/drivers/led.cpp.o
Compiling .pio/build/esp32dev/src/drivers/uart.cpp.o
Compiling .pio/build/esp32dev/src/main.cpp.o
Linking .pio/build/esp32dev/firmware.elf
Retrieving maximum program size .pio/build/esp32dev/firmware.elf
Checking size .pio/build/esp32dev/firmware.elf
Advanced Memory Usage is available via "PlatformIO Home > Project Inspect"
RAM:   [=         ]   6.6% (used 21504 bytes from 327680 bytes)
Flash: [==        ]  20.8% (used 272461 bytes from 1310720 bytes)
Building .pio/build/esp32dev/firmware.bin
esptool.py v4.9.0
Creating esp32 image...
Merged 2 ELF sections
Successfully created esp32 image.
```

### Wynik: testy (Serial Terminal Wokwi)

```text
rst:0x1 (POWERON_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:2
load:0x3fff0030,len:1156
load:0x40078000,len:11456
ho 0 tail 12 room 4
load:0x40080400,len:2972
entry 0x400805dc
SYS: Tasks started
SYSTEM READY
LED: MAIN RED -> MAIN YELLOW
LED: MAIN YELLOW -> MAIN GREEN
LED: MAIN GREEN -> MAIN YELLOW
LED: MAIN YELLOW -> MAIN RED
LED: MAIN RED -> MAIN YELLOW
LED: MAIN YELLOW -> MAIN GREEN
LED: MAIN GREEN -> MAIN YELLOW
LED: MAIN YELLOW -> MAIN RED
BTN: Press Start
BTN: Released
BTN: Duration ms: 430
BTN->FSM: ShortPress
SYS: Exited IDLE, total time: 19131 ms
SYS: RUN MODE
LED: RED -> GREEN
LED: GREEN -> RED
SYS: Exited RUN, total time: 6008 ms
SYS: IDLE MODE
LED: MAIN RED -> MAIN YELLOW
LED: MAIN YELLOW -> MAIN GREEN
LED: MAIN GREEN -> MAIN YELLOW
LED: MAIN YELLOW -> MAIN RED
LED: MAIN RED -> MAIN YELLOW
BTN: Press Start
LED: MAIN YELLOW -> MAIN GREEN
LED: MAIN GREEN -> MAIN YELLOW
BTN: Released
BTN: Duration ms: 4700
BTN->FSM: LongPress
SYS: Exited IDLE, total time: 36258 ms
SYS: SAFE MODE
LED: YELLOW BLINK
LED: YELLOW BLINK
LED: YELLOW BLINK
LED: YELLOW BLINK
LED: YELLOW BLINK
LED: YELLOW BLINK
LED: YELLOW BLINK
LED: YELLOW BLINK
LED: YELLOW BLINK
LED: YELLOW BLINK
LED: YELLOW BLINK
LED: YELLOW BLINK
BTN: Press Start
LED: YELLOW BLINK
LED: YELLOW BLINK
LED: YELLOW BLINK
LED: YELLOW BLINK
LED: YELLOW BLINK
LED: YELLOW BLINK
LED: YELLOW BLINK
BTN: Released
BTN: Duration ms: 2930
BTN->FSM: ShortPress
LED: YELLOW BLINK
LED: YELLOW BLINK
LED: YELLOW BLINK
LED: YELLOW BLINK
LED: YELLOW BLINK
BTN: Press Start
LED: YELLOW BLINK
LED: YELLOW BLINK
LED: YELLOW BLINK
LED: YELLOW BLINK
LED: YELLOW BLINK
LED: YELLOW BLINK
LED: YELLOW BLINK
LED: YELLOW BLINK
LED: YELLOW BLINK
LED: YELLOW BLINK
LED: YELLOW BLINK
LED: YELLOW BLINK
LED: YELLOW BLINK
LED: YELLOW BLINK
LED: YELLOW BLINK
BTN: Released
BTN: Duration ms: 5890
BTN->FSM: LongPress
SYS: Exited SAFE, total time: 16060 ms
SYS: IDLE MODE
LED: MAIN RED -> MAIN YELLOW
LED: MAIN YELLOW -> MAIN GREEN
```