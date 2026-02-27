#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

FLAGS=(
  -std=gnu++17
  -DTEENSYDUINO=159
  -D__IMXRT1062__
  -DARDUINO_TEENSY41
  -DF_CPU=600000000
  -I"$ROOT/include"
  -I"$ROOT/src"
  -I"$ROOT/lib/Audio"
  -I"$ROOT/lib/SerialFlash"
  "-I$ROOT/lib/Adafruit ILI9341"
  "-I$ROOT/lib/Adafruit GFX Library@src-bec0f45403ebb1c48ea0f9da3babd45f"
  "-I$ROOT/lib/Adafruit seesaw Library"
  -I"$ROOT/.pio/libdeps/teensy41/Adafruit BusIO"
  -I"$HOME/.platformio/packages/framework-arduinoteensy/cores/teensy4"
  -I"$ROOT/clang-tidy-stubs"
)

exec "$(brew --prefix llvm)/bin/clang-tidy" "$@" -- "${FLAGS[@]}"
