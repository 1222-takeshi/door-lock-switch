#!/usr/bin/env bash
# Run from any directory. Requires Arduino CLI (on PATH or in ~/.local/bin),
# the esp32:esp32 core, the ESP32Servo library, and a host g++ compiler.
# See firmware/README.md for setup commands and verified toolchain versions.
set -eu

stage="build setup"
work_dir=
trap '
    status=$?
    if [[ -n "$work_dir" ]]; then
        rm -rf -- "$work_dir" || status=1
    fi
    if (( status != 0 )); then
        printf "FAIL: %s (exit %s)\n" "$stage" "$status" >&2
    fi
    exit "$status"
' EXIT

repo_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
cd "$repo_dir"
export PATH="$PATH:$HOME/.local/bin"
mkdir -p build
work_dir=$(mktemp -d "$repo_dir/build/build-check.XXXXXXXX")
export TMPDIR="$work_dir"

stage="ESP32 sketch compile"
# Arduino requires the primary .ino file and its sketch directory to share a name.
mkdir "$work_dir/door_lock"
cp firmware/door_lock.ino firmware/battery_estimator.h "$work_dir/door_lock/"
arduino-cli compile --fqbn esp32:esp32:esp32 \
    --build-path "$work_dir/esp32" "$work_dir/door_lock"
printf 'PASS: ESP32 sketch compile\n'

stage="host battery test build"
g++ -std=c++11 -I firmware firmware/tests/test_battery.cpp -o "$work_dir/test_battery"
stage="host battery tests"
"$work_dir/test_battery"
printf 'PASS: host battery tests\n'
printf 'PASS: firmware build check\n'
