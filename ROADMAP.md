# Roadmap

Development follows the process in CONTRIBUTING.md: roadmap -> issues ->
TDD (tests first) -> small PRs (roughly 200 lines or less) -> AI review ->
merge.

Priority update (2026-11-07, owner decision): the light-switch pusher is
built first. The thumb-turn lock work is on hold and resumes only on the
owner's word. Safety requirements are unchanged: remote operation stays
lock-only, no remote unlock path is ever added, and WiFi stays
notify-only.

## M0 - Foundations (done)

- [x] ESP32 firmware skeleton: wake sources, lock sequence, light-switch
  pusher, battery estimation
- [x] Battery estimator extracted as a pure C++ module with host tests

## M1 - ESP32 platform bench verification (common foundation)

- [ ] Compile and flash; confirm wake sources (button / door sensor /
  timer) on the bench (Issue #1)
- [ ] Measure deep-sleep current of the dev board (this decides battery
  life); if it is too high, plan a bare-module or power-path change
  (Issue #2)
- [ ] Calibrate the battery ADC against a multimeter (the ADC part of
  Issue #3; the servo part of #3 splits as below)

## M2 - Light-switch pusher (first priority, Issue #7)

- [ ] Design the pusher mechanism: reversible wall mounting, pusher arm
  shape and stroke, manual operation left fully usable
- [ ] Bench rig: verify push force and stroke, tune the 9 g servo
  rest/push angles (the light-servo part of Issue #3)
- [ ] Wall trial with the reversible fixing; repeated-push soak
- [ ] Low-battery WiFi notification enabled (notify-only, no receive
  path), verified with a forced-low test

## M3 - Battery estimation validation (Issue #6)

- [ ] Log pack voltage over real use and compare the estimate against the
  actual service life; tune the curve/consumption constants

## On hold - Thumb-turn lock (owner decision, 2026-11-07)

On hold until the owner asks to resume. Next step and resume condition
are recorded in the project status report.

- [ ] MG946R lock angles/pulse widths (the lock-servo part of Issue #3)
- [ ] Measure the thumb turn; design and print the adapter and mount
  (reversible fixing, no door modification) (Issue #4)
- [ ] Verify manual operation stays free and light with the device fitted
  (Issue #4)
- [ ] Fit to the door; lock-state detection working end to end; soak
  test (Issue #5)
