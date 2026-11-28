// Two-spoke pusher arm for the light-switch pusher, design v1
// (Issue #18). The target rocker is a seesaw (docs/LIGHT_SWITCH.md),
// so one arm must press the upper and lower halves alternately.
//
// Frame: origin at the shaft centre, X along the shaft axis (arm
// thickness direction, +X toward the servo body), Y out from the
// wall, Z up. At rest the two spokes point at +/- spoke_angle from
// the wall-pointing (-Y) direction. A press swings a spoke about
// 11 degrees toward the wall-pointing direction: the disc pad meets
// the rocker half at about Z = +/-15..17 and drives it about 1.6 mm
// (see the stroke budget in docs/LIGHT_SWITCH.md; the exact swing is
// set during bench tuning, U6). Pads are discs at pad_radius - a disc
// has no corners, so its reach toward the wall is the same at every
// swing angle (a rectangular pad's corner would eat the rest
// clearance; caught in the first render of this model).
//
// The hub clamps over the stock servo horn: centre hole for the horn
// screw and four screw holes at 45-degree diagonals (radius 6.5).
// The spline itself is not printed.
//
// Render status: verified with OpenSCAD 2021.01 (Issue #18).

// ---- Parameters ----
spoke_angle = 45;      // rest angle of each spoke from -Y, degrees
pad_radius = 26;       // shaft centre to pad centre
spoke_w = 10;          // spoke width (tangential)
arm_t = 4;             // arm thickness (X)
hub_r = 11;            // hub disc radius
pad_r = 5;             // pad disc radius
pad_x = 10;            // pad width (X)

// One spoke with its pad, at rest angle a (degrees, + = upper).
module spoke(a) {
  rotate([-a, 0, 0]) {
    // Bar from radius 4 to radius 28, centred on -Y.
    translate([0, -16, 0]) cube([arm_t, 24, spoke_w], center = true);
    // Disc pad at pad_radius (axis along X).
    translate([0, -pad_radius, 0]) rotate([0, 90, 0])
      cylinder(h = pad_x, r = pad_r, center = true);
  }
}

module arm() {
  difference() {
    union() {
      // Hub disc (X from -arm_t/2 to +arm_t/2).
      rotate([0, 90, 0]) cylinder(h = arm_t, r = hub_r, center = true);
      spoke(spoke_angle);
      spoke(-spoke_angle);
    }
    // Centre hole for the horn screw.
    rotate([0, 90, 0]) cylinder(h = arm_t + 2, r = 2.6, center = true);
    // Four horn screw holes at 45-degree diagonals.
    for (sy = [-1, 1], sz = [-1, 1])
      translate([0, sy * 4.6, sz * 4.6])
        rotate([0, 90, 0]) cylinder(h = arm_t + 2, r = 1.1, center = true);
  }
}

arm();
