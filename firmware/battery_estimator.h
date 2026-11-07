// battery_estimator.h — Battery state-of-charge estimation for the
// door-lock-switch project.
//
// Pure C++ (C++11), header-only, no Arduino or platform dependencies, so the
// estimation logic can be unit-tested on a host machine. The ESP32 sketch
// (door_lock.ino) is only responsible for measuring the battery voltage,
// tracking the consumed-charge accumulator, and passing those values in.
//
// Estimation strategy (hybrid):
//   1. Voltage estimate: no-load pack voltage -> per-cell voltage -> percent
//      via a discharge-curve lookup table with linear interpolation.
//   2. Coulomb-count estimate: 100% minus the accumulated consumption
//      (per-event charge costs, integrated by the caller) over capacity.
//   3. The final estimate is the smaller (more conservative) of the two.
//   4. If the per-cell voltage measured under load (during a lock cycle)
//      sags below a threshold, the pack is considered end-of-life and the
//      estimate is forced to 0%, regardless of the other two estimates.
//
// All parameters are provisional values to be calibrated on real hardware.

#pragma once

#include <cstddef>

namespace battery {

// One point of a discharge curve: no-load per-cell voltage [V] -> charge [%].
// Tables are ordered by descending voltage, starting at 100% and ending at 0%.
struct CurvePoint {
  float cellVolts;
  float percent;
};

// Everything the estimator needs to know about a battery pack.
struct Profile {
  Profile(const CurvePoint* curve_, std::size_t curveSize_, int cells_,
          float capacityMah_)
      : curve(curve_), curveSize(curveSize_), cells(cells_),
        capacityMah(capacityMah_) {}

  const CurvePoint* curve;
  std::size_t curveSize;
  int cells;
  float capacityMah;
};

namespace detail {

// AA alkaline discharge curve (per-cell, no load). Alkaline cells have a
// sloped discharge curve, so the voltage method works reasonably well.
inline const CurvePoint* alkalineCurve(std::size_t& size) {
  static const CurvePoint kCurve[] = {
      {1.60f, 100.0f}, {1.55f, 92.0f}, {1.50f, 82.0f}, {1.45f, 68.0f},
      {1.40f, 55.0f},  {1.35f, 42.0f}, {1.30f, 30.0f}, {1.25f, 20.0f},
      {1.20f, 12.0f},  {1.15f, 6.0f},  {1.10f, 3.0f},  {1.05f, 0.0f},
  };
  size = sizeof(kCurve) / sizeof(kCurve[0]);
  return kCurve;
}

// AA NiMH discharge curve (per-cell, no load). NiMH curves are flat over
// most of the range, so the voltage estimate is coarse; treat it as a guide.
inline const CurvePoint* nimhCurve(std::size_t& size) {
  static const CurvePoint kCurve[] = {
      {1.45f, 100.0f}, {1.35f, 90.0f}, {1.30f, 75.0f}, {1.27f, 60.0f},
      {1.24f, 45.0f},  {1.21f, 30.0f}, {1.18f, 18.0f}, {1.15f, 10.0f},
      {1.10f, 4.0f},   {1.00f, 0.0f},
  };
  size = sizeof(kCurve) / sizeof(kCurve[0]);
  return kCurve;
}

}  // namespace detail

// Pack profile: 4x AA alkaline in series (~6 V rail).
// Capacity is a conservative effective value for AA alkaline cells.
inline Profile alkalineAaX4Profile() {
  std::size_t n = 0;
  const CurvePoint* curve = detail::alkalineCurve(n);
  return Profile(curve, n, 4, 2000.0f);
}

// Pack profile: 5x AA NiMH in series (~6 V rail), nominal capacity.
inline Profile nimhAaX5Profile() {
  std::size_t n = 0;
  const CurvePoint* curve = detail::nimhCurve(n);
  return Profile(curve, n, 5, 1900.0f);
}

// Percent remaining for a no-load per-cell voltage, by linear interpolation
// on the profile's discharge curve. Voltages above the top of the curve
// return the top percent (100%); voltages below the bottom return 0%.
inline float percentFromCellVolts(const Profile& profile, float cellVolts) {
  const CurvePoint* curve = profile.curve;
  const std::size_t n = profile.curveSize;
  if (cellVolts >= curve[0].cellVolts) return curve[0].percent;
  for (std::size_t i = 1; i < n; i++) {
    if (cellVolts >= curve[i].cellVolts) {
      const float v0 = curve[i - 1].cellVolts, p0 = curve[i - 1].percent;
      const float v1 = curve[i].cellVolts, p1 = curve[i].percent;
      return p1 + (p0 - p1) * (cellVolts - v1) / (v0 - v1);
    }
  }
  return curve[n - 1].percent;
}

// Percent remaining for a no-load pack voltage (converted to per-cell first).
inline float percentFromPackVolts(const Profile& profile, float packVolts) {
  return percentFromCellVolts(profile, packVolts / profile.cells);
}

// Percent remaining from the accumulated consumption [mAh] alone.
// Clamped at 0% once the accumulated consumption reaches the capacity.
inline float percentFromConsumedMah(const Profile& profile,
                                    float consumedMah) {
  const float pct = 100.0f - 100.0f * consumedMah / profile.capacityMah;
  return pct < 0.0f ? 0.0f : pct;
}

// Combined estimate: the smaller of the voltage-based and consumption-based
// estimates. If loadedCellVolts is non-negative and below loadCellMinVolts
// (voltage collapse under load = internal resistance has grown), the pack is
// treated as end-of-life and 0% is returned. Pass a negative
// loadedCellVolts to skip the under-load check.
inline float estimatePercent(const Profile& profile, float restPackVolts,
                             float loadedCellVolts, float consumedMah,
                             float loadCellMinVolts) {
  const float byVolts = percentFromPackVolts(profile, restPackVolts);
  const float byCount = percentFromConsumedMah(profile, consumedMah);
  float pct = byVolts < byCount ? byVolts : byCount;
  if (loadedCellVolts >= 0.0f && loadedCellVolts < loadCellMinVolts) {
    pct = 0.0f;
  }
  return pct;
}

// Three-level classification used for user notification.
enum class Level { Ok, Low, Replace };

// pct >= okMinPercent -> Ok; pct >= lowMinPercent -> Low; otherwise Replace.
inline Level levelFromPercent(float pct, float okMinPercent = 40.0f,
                              float lowMinPercent = 15.0f) {
  if (pct >= okMinPercent) return Level::Ok;
  if (pct >= lowMinPercent) return Level::Low;
  return Level::Replace;
}

}  // namespace battery
