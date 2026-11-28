// Two-spoke pusher arm for the light-switch pusher, design v1
// (Issue #18, revised by Issue #24). The target rocker is a seesaw
// (docs/LIGHT_SWITCH.md), so one arm must press the upper and lower
// halves alternately.
//
// Frame: origin at the shaft centre, X along the shaft axis (+X
// toward the servo body), Y out from the wall, Z up. At rest the two
// spokes point at +/- spoke_angle from the wall-pointing (-Y)
// direction. A press swings a spoke about 11-15 degrees toward the
// wall-pointing direction (bench-tuned, U6): the disc pad meets the
// rocker half at about Z = +/-15..17 and drives it about 1.6 mm.
// Pads are discs at pad_radius - a disc has no corners, so its reach
// toward the wall is the same at every swing angle.
//
// Fixing: the centre screw through the shaft hole is the primary
// fixing, as with stock horns. The servo-side face of the hub has a
// stepped recess (disc-horn pocket r = 10.5, boss recess r = 6.5) so
// the horn and shaft boss seat inside the hub outline, and four
// diagonal slots (radius 5.2..8.8) take horn screws where the horn
// provides them. The pads are flush with the servo-side (+X) face,
// so the arm prints flat on its -X face without supports.
//
// Render status: verified with OpenSCAD 2021.01 (Issues #18, #24).

$fn = 48;

// ---- Parameters ----
spoke_angle = 45;      // rest angle of each spoke from -Y, degrees
pad_radius = 26;       // shaft centre to pad centre
spoke_w = 10;          // spoke width (tangential)
arm_t = 4;             // arm thickness (X)
hub_r = 11;            // hub disc radius
pad_r = 5;             // pad disc radius
pad_x = 10;            // pad width (X), flush with the +X face

// One spoke with its pad, at rest angle a (degrees, + = upper).
module spoke(a) {
  rotate([-a, 0, 0]) {
    // Bar from radius 4 to radius 26 (tip inside the pad disc).
    translate([0, -15, 0]) cube([arm_t, 22, spoke_w], center = true);
    // Disc pad at pad_radius: spans X = -8..+2, flush with the
    // hub's +X (servo-side) face, protruding toward the rocker.
    translate([-3, -pad_radius, 0])
      rotate([0, 90, 0]) cylinder(h = pad_x, r = pad_r, center = true);
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
    rotate([0, 90, 0]) cylinder(h = arm_t + 4, r = 2.6, center = true);
    // Stepped recess on the +X (servo) face: disc-horn pocket
    // (floor at X = +0.5, depth 1.5).
    translate([1.5, 0, 0]) rotate([0, 90, 0])
      cylinder(h = 2, r = 10.5, center = true);
    // Boss recess (floor at X = -0.5, depth 2.5; 1.5 mm of hub
    // remains on the rocker side).
    translate([0.75, 0, 0]) rotate([0, 90, 0])
      cylinder(h = 2.5, r = 6.5, center = true);
    // Four diagonal horn-screw slots (radius 5.2..8.8).
    for (sy = [-1, 1], sz = [-1, 1])
      hull() {
        for (rr = [5.2, 8.8])
          translate([0, sy * rr * 0.7071, sz * rr * 0.7071])
            rotate([0, 90, 0]) cylinder(h = arm_t + 4, r = 1.2, center = true);
      }
  }
}

arm();
