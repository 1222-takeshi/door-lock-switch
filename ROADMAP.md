# Roadmap

Development follows the process in CONTRIBUTING.md: roadmap -> issues ->
TDD (tests first) -> small PRs (roughly 200 lines or less) -> AI review ->
merge.

## M0 - Foundations (done)

- [x] ESP32 firmware skeleton: wake sources, lock sequence, light-switch
  pusher, battery estimation
- [x] Battery estimator extracted as a pure C++ module with host tests

## M1 - Bench verification

- [ ] Compile and flash; confirm wake sources (button / door sensor /
  timer) and the lock sequence on the bench
- [ ] Measure deep-sleep current of the dev board (this decides battery
  life); if it is too high, plan a bare-module or power-path change
- [ ] Determine servo angles/pulse widths and ADC calibration against a
  multimeter

## M2 - Thumb-turn adapter

- [ ] Measure the thumb turn; design and print the adapter and mount
  (reversible fixing, no door modification)
- [ ] Verify manual operation stays free and light with the device fitted

## M3 - Installation

- [ ] Fit to the door; lock-state detection working end to end
- [ ] Soak test: manual lock/unlock remains normal over daily use

## M4 - Battery estimation validation

- [ ] Log pack voltage over real use and compare the estimate against the
  actual service life; tune the curve/consumption constants

## M5 - Light switch and notifications

- [ ] Light-switch pusher fitted and working
- [ ] Low-battery WiFi notification enabled (notify-only, no receive path)
