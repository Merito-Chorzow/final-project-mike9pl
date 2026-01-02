#!/usr/bin/env bash

printf "\n\nBuild PlatformIO for esp32dev\n\n\n"
platformio debug
printf "\n\nRun PlatformIO for esp32dev\n\n\n"
platformio run --environment esp32dev