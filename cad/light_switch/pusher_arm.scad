// pusher_arm.scad - pusher arm for the light-switch pusher (Issue #7).
//
// Flat printed arm that clamps over the stock servo horn (the spline is
// not printed). The hub sits on the servo shaft; the wide tip pad presses
// the rocker. Print flat as modelled (thickness along Z).
//
// Units: mm. See docs/LIGHT_SWITCH.md for the stroke budget: with the
// default arm_len = 22 and a 55 deg swing, tip travel toward the wall is
// about 9.7 mm.
//
// Render status: verified with OpenSCAD 2021.01 (Issue #12) - renders
// cleanly as a single solid; exported bounding box is
// 35.5 x 14.0 x 4.0 mm, matching the parameters below.

// ---- Parameters ----
arm_len = 22;      // hub centre to tip-pad centre (effective length)
thickness = 4;     // arm thickness
hub_r = 7;         // hub disc radius
mid_r = 5;         // arm radius near the hub
tip_r = 6.5;       // tip pad radius (wider, flat contact)

$fn = 32;

module arm() {
  difference() {
    // Body: hub disc blended into the arm and the tip pad.
    hull() {
      cylinder(h = thickness, r = hub_r);
      translate([arm_len, 0, 0]) cylinder(h = thickness, r = tip_r);
    }
    // Centre hole for the horn screw.
    translate([0, 0, -1]) cylinder(h = thickness + 2, r = 1.6);
    // Two holes matching the stock horn's screw positions.
    for (sx = [-1, 1])
      translate([sx * 4.5, 0, -1]) cylinder(h = thickness + 2, r = 1.1);
    // Lightening hole in the arm.
    translate([arm_len / 2, 0, -1]) cylinder(h = thickness + 2, r = 3);
  }
}

arm();
