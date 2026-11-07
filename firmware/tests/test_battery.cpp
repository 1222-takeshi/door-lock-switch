// test_battery.cpp — Host-side tests for battery_estimator.h.
//
// Build and run (from the firmware/ directory):
//   g++ -std=c++11 -Wall -Wextra -I . tests/test_battery.cpp -o /tmp/test_battery
//   /tmp/test_battery
//
// Uses plain assert(); do not compile with -DNDEBUG.

#include <cassert>
#include <cmath>
#include <cstdio>

#include "battery_estimator.h"

namespace {

bool near(float a, float b, float eps = 0.01f) { return std::fabs(a - b) <= eps; }

void testAlkalineEndpoints() {
  const battery::Profile p = battery::alkalineAaX4Profile();
  // Exact table endpoints.
  assert(near(battery::percentFromCellVolts(p, 1.60f), 100.0f));
  assert(near(battery::percentFromCellVolts(p, 1.05f), 0.0f));
  // Above the top of the curve saturates at 100%; below the bottom is 0%.
  assert(near(battery::percentFromCellVolts(p, 1.70f), 100.0f));
  assert(near(battery::percentFromCellVolts(p, 0.90f), 0.0f));
  // Exact interior table points.
  assert(near(battery::percentFromCellVolts(p, 1.30f), 30.0f));
  assert(near(battery::percentFromCellVolts(p, 1.20f), 12.0f));
}

void testAlkalineInterpolation() {
  const battery::Profile p = battery::alkalineAaX4Profile();
  // Midpoint between (1.50 V, 82%) and (1.45 V, 68%) is 75%.
  assert(near(battery::percentFromCellVolts(p, 1.475f), 75.0f));
  // Quarter point between (1.40 V, 55%) and (1.35 V, 42%):
  // 42 + 13 * 0.75 = 51.75 at 1.3875 V.
  assert(near(battery::percentFromCellVolts(p, 1.3875f), 51.75f, 0.05f));
}

void testNimhCurve() {
  const battery::Profile p = battery::nimhAaX5Profile();
  assert(near(battery::percentFromCellVolts(p, 1.45f), 100.0f));
  assert(near(battery::percentFromCellVolts(p, 1.00f), 0.0f));
  assert(near(battery::percentFromCellVolts(p, 1.24f), 45.0f));
  // Midpoint between (1.30 V, 75%) and (1.27 V, 60%) is 67.5%.
  assert(near(battery::percentFromCellVolts(p, 1.285f), 67.5f));
  assert(near(battery::percentFromCellVolts(p, 1.60f), 100.0f));
  assert(near(battery::percentFromCellVolts(p, 0.80f), 0.0f));
}

void testPackVolts() {
  const battery::Profile alk = battery::alkalineAaX4Profile();
  // 4 cells * 1.30 V = 5.20 V pack -> 30%.
  assert(near(battery::percentFromPackVolts(alk, 5.20f), 30.0f));
  // Fresh pack: 4 * 1.60 V = 6.40 V -> 100%.
  assert(near(battery::percentFromPackVolts(alk, 6.40f), 100.0f));

  const battery::Profile nimh = battery::nimhAaX5Profile();
  // 5 cells * 1.24 V = 6.20 V pack -> 45%.
  assert(near(battery::percentFromPackVolts(nimh, 6.20f), 45.0f));
}

void testMonotonicDecreasing() {
  // For both curves, percent must never increase as voltage decreases.
  const battery::Profile profiles[2] = {battery::alkalineAaX4Profile(),
                                        battery::nimhAaX5Profile()};
  for (int k = 0; k < 2; k++) {
    float prev = battery::percentFromCellVolts(profiles[k], 1.80f);
    assert(near(prev, 100.0f));
    for (float v = 1.79f; v >= 0.70f; v -= 0.01f) {
      const float pct = battery::percentFromCellVolts(profiles[k], v);
      assert(pct <= prev + 1e-4f);
      assert(pct >= 0.0f && pct <= 100.0f);
      prev = pct;
    }
    assert(near(prev, 0.0f));
  }
}

void testConsumedMah() {
  const battery::Profile p = battery::alkalineAaX4Profile();  // 2000 mAh
  assert(near(battery::percentFromConsumedMah(p, 0.0f), 100.0f));
  assert(near(battery::percentFromConsumedMah(p, 1000.0f), 50.0f));
  assert(near(battery::percentFromConsumedMah(p, 2000.0f), 0.0f));
  // Over-consumption clamps at 0%, never negative.
  assert(near(battery::percentFromConsumedMah(p, 5000.0f), 0.0f));

  const battery::Profile nimh = battery::nimhAaX5Profile();  // 1900 mAh
  assert(near(battery::percentFromConsumedMah(nimh, 950.0f), 50.0f));
}

void testHybridTakesSmaller() {
  const battery::Profile p = battery::alkalineAaX4Profile();
  const float kLoadMin = 1.00f;

  // Voltage says 82% (6.00 V pack = 1.50 V/cell), count says 50%
  // (1000 of 2000 mAh used) -> the smaller (count) wins.
  assert(near(battery::estimatePercent(p, 6.00f, -1.0f, 1000.0f, kLoadMin),
              50.0f));
  // Voltage says 12% (4.80 V pack = 1.20 V/cell), count says 95%
  // (100 mAh used) -> the smaller (voltage) wins.
  assert(near(battery::estimatePercent(p, 4.80f, -1.0f, 100.0f, kLoadMin),
              12.0f));
  // Both agree at a fresh pack.
  assert(near(battery::estimatePercent(p, 6.40f, -1.0f, 0.0f, kLoadMin),
              100.0f));
}

void testLoadCollapse() {
  const battery::Profile p = battery::alkalineAaX4Profile();
  const float kLoadMin = 1.00f;

  // Per-cell voltage under load below the threshold forces 0%, even when
  // both other estimates are healthy.
  assert(near(battery::estimatePercent(p, 6.40f, 0.95f, 0.0f, kLoadMin),
              0.0f));
  // Exactly at the threshold is not a collapse.
  assert(near(battery::estimatePercent(p, 6.40f, 1.00f, 0.0f, kLoadMin),
              100.0f));
  // A negative loaded voltage means "not measured": check is skipped.
  assert(near(battery::estimatePercent(p, 6.40f, -1.0f, 0.0f, kLoadMin),
              100.0f));
}

void testLevels() {
  using battery::Level;
  // Default thresholds: Ok >= 40, Low >= 15, Replace < 15.
  assert(battery::levelFromPercent(100.0f) == Level::Ok);
  assert(battery::levelFromPercent(40.0f) == Level::Ok);    // boundary
  assert(battery::levelFromPercent(39.9f) == Level::Low);
  assert(battery::levelFromPercent(15.0f) == Level::Low);   // boundary
  assert(battery::levelFromPercent(14.9f) == Level::Replace);
  assert(battery::levelFromPercent(0.0f) == Level::Replace);

  // Custom thresholds are honored.
  assert(battery::levelFromPercent(50.0f, 60.0f, 20.0f) == Level::Low);
  assert(battery::levelFromPercent(60.0f, 60.0f, 20.0f) == Level::Ok);
  assert(battery::levelFromPercent(19.9f, 60.0f, 20.0f) == Level::Replace);
}

}  // namespace

int main() {
  testAlkalineEndpoints();
  testAlkalineInterpolation();
  testNimhCurve();
  testPackVolts();
  testMonotonicDecreasing();
  testConsumedMah();
  testHybridTakesSmaller();
  testLoadCollapse();
  testLevels();
  std::printf("test_battery: all tests passed\n");
  return 0;
}
