# Plan

## Safety requirements

See README.md. In short: remote lock only, never remote unlock; manual
operation always takes priority; fail-free on any fault or battery
depletion; lock completion is detected, not assumed. These requirements
outrank every other design decision.

## Configuration

- Thumb-turn: MG946R with a printed adapter shaped to the actual thumb
  turn. Mounting is reversible (tape + auxiliary fixing), no permanent
  door/wall modification.
- Light switch: 9 g servo with a pusher arm.
- Controller: ESP32. A physical button is fitted so locking also works
  with no network.
- Wake sources: physical button, door open/close sensor (reed switch),
  and a periodic timer (checks battery and forgotten locks).

## Power and battery estimation

- Battery powered (AA x4 alkaline 6 V, or NiMH x5). The ESP32 deep-sleeps;
  the servo rail is FET-gated so standby draw is near zero on the servo
  side. WiFi connects only briefly on wake.
- Expected life at a few locks per day is months to a year class
  (unverified estimate; to be measured on the first unit). The dev board's
  own deep-sleep current (LDO, power LED) may dominate - measure early.
- Estimation: pack voltage via divider + ADC, sampled unloaded and during
  the locking motion. Percent = min(voltage-curve estimate,
  consumption-integral estimate). Levels: OK / low / replace; "low"
  triggers a notification on the next WiFi wake and an LED indication.
  Validation: log voltage over the pack's real life and compare.

## Stages

1. Measure the thumb turn (shape, torque needed)
2. Adapter and mount design/print; verify the manual-priority behavior
3. ESP32 control with the physical button
4. Lock-state detection
5. Light-switch side
6. Battery estimation implementation and validation
7. WiFi (notify-only) last
