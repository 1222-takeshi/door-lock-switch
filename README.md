# door-lock-switch

A battery-powered device that turns a front-door thumb-turn lock to the
LOCKED position, plus a small pusher that operates a wall light switch.
Built around an ESP32 and 3D-printed mounts (FLASHFORGE Adventurer 3).

Japanese README: [README.ja.md](README.ja.md)

## Safety design (non-negotiable)

1. Remote operation is **lock-only**. Unlocking is never done remotely;
   the firmware contains no remote unlock path (WiFi is used only to send
   low-battery notifications).
2. Manual operation always wins: with the servo unpowered the mechanism
   is free, so the thumb turn can be operated by hand at any time.
3. On fault, communication loss, or battery depletion the mechanism goes
   free - it can never lock the resident out.
4. Lock completion is detected (end-of-travel switch), not assumed.

## Hardware

| Role | Part |
|---|---|
| Thumb-turn drive | MG946R servo (high torque) with a printed adapter |
| Light switch | 9 g servo with a printed pusher arm |
| Controller | ESP32 dev board (deep sleep, wakes on button / door sensor / timer) |
| Power | Battery: AA x4 (6 V) alkaline or NiMH x5; servo rail gated by a FET |

## Battery level estimation

The ESP32 samples pack voltage through a divider (unloaded and during the
locking motion). Remaining capacity is estimated by combining a
voltage-to-discharge-curve conversion with an actuation-count consumption
integral, taking the lower of the two. Three levels (OK / low / replace)
are shown on an LED and notified over WiFi when low. The estimation logic
lives in a dependency-free C++ header (`firmware/battery_estimator.h`)
with host-side tests.

## Repository layout

- `firmware/door_lock.ino` - ESP32 firmware (Arduino core).
- `firmware/battery_estimator.h` - battery estimation module (pure C++).
- `firmware/tests/` - host tests for the estimator.
- `docs/PLAN.md` - design plan. `ROADMAP.md` - development roadmap.
- `CONTRIBUTING.md` - development process (roadmap -> issues -> TDD ->
  small PRs -> review -> merge).

## Status

Firmware skeleton is written, including the lock sequence and battery
estimation; the estimator is host-tested. Bench verification on the real
board (deep-sleep current, servo angles, ADC calibration) is next.
