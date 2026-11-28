// dummy_rocker.scad - dummy rocker for the light-switch bench rig
// (Issue #13; rig for Issue #7).
//
// A spring-loaded seesaw paddle in a shallow box that stands in for a
// wall switch rocker, so push force and stroke can be measured on the
// bench (unit U6) without the real switch. Mounts on the same vertical
// bench board as bracket.scad, at the rocker position.
//
// Coordinates match bracket.scad: origin at the rocker centre on the
// wall plane. X: right (wall-parallel), Y: out from the wall, Z: up.
// Units: mm.
//
// The paddle pivots on an M3 screw (axis along X). A short compression
// spring (pen-spring class) behind the lower half returns the paddle
// to its rest tilt. Internal stops limit the tilt to +/- max_tilt.
//
// From 2026-11-28 the paddle matches the measured target switch
// (design v1, docs/LIGHT_SWITCH.md): a 46 x 92 mm wide rocker with
// about 3 mm of travel at the ends, hence max_tilt = 4 deg (about
// 3.2 mm at the ends).
//
// Render status: verified with OpenSCAD 2021.01 (Issue #13) - box,
// paddle and assembled views render as single valid solids.

// ---- Parameters ----
part = "assembled";    // "box", "paddle" or "assembled"

paddle_x = 14;          // paddle centre offset in X (under the arm)
paddle_w = 46;         // paddle width  (X)
paddle_h = 92;         // paddle height (Z)
paddle_t = 4;          // paddle thickness (Y)
box_depth = 26;        // box front rim distance from the wall (Y)
box_wall = 2.5;        // box wall thickness
pivot_y = 16;          // pivot axis distance from the wall (Y)
max_tilt = 4;         // paddle tilt limit, degrees
spring_z = -14;        // return-spring centre height (below pivot)

$fn = 32;

// ---- Derived ----
box_w = paddle_w + 4 + 2 * box_wall;         // inner clearance 2/side
box_h = paddle_h + 20;
flange_w = box_w + 24;
flange_h = box_h + 12;
stop_y = pivot_y - paddle_t / 2
         - (paddle_h / 2 - 3) * sin(max_tilt) - 0.3;

module box() {
  union() {
    // Back flange: screws to the bench board at the rocker position.
    difference() {
      translate([paddle_x - flange_w / 2, 0, -flange_h / 2])
        cube([flange_w, box_wall, flange_h]);
      for (sx = [-1, 1], sz = [-1, 1])
        translate([paddle_x + sx * (flange_w / 2 - 6), -1,
                   sz * (flange_h / 2 - 6)])
          rotate([-90, 0, 0]) cylinder(h = box_wall + 2, r = 1.7);
    }
    // Side walls (pivot cheeks), embedded 1 mm into the flange.
    for (sx = [-1, 1]) {
      wx = paddle_x + sx * (paddle_w / 2 + 2) - (sx < 0 ? box_wall : 0);
      difference() {
        translate([wx, box_wall - 1, -box_h / 2])
          cube([box_wall, box_depth - box_wall + 1, box_h]);
        // Pivot pin hole (M3 screw).
        translate([wx - 1, pivot_y, 0])
          rotate([0, 90, 0]) cylinder(h = box_wall + 2, r = 1.6);
      }
    }
    // Top and bottom walls, embedded into flange and side walls.
    for (sz = [-1, 1])
      translate([paddle_x - box_w / 2, box_wall - 1,
                 sz * box_h / 2 - (sz > 0 ? box_wall : 0)])
        cube([box_w, box_depth - box_wall + 1, box_wall]);
    // Tilt stops inside, top and bottom (behind the paddle).
    for (sz = [-1, 1])
      translate([paddle_x - 10, box_wall - 1,
                 sz * (paddle_h / 2 - 3) - (sz > 0 ? 6 : 0)])
        cube([20, stop_y - box_wall + 1, 6]);
    // Return-spring guide post on the back, below the pivot.
    translate([paddle_x, box_wall, spring_z])
      rotate([-90, 0, 0]) cylinder(h = 7, r = 2.4);
  }
}

module paddle() {
  // Modelled centred on the pivot; place with translate/rotate.
  difference() {
    translate([-paddle_w / 2, -paddle_t / 2, -paddle_h / 2])
      cube([paddle_w, paddle_t, paddle_h]);
    // Pivot pin hole (M3 screw).
    translate([-paddle_w / 2 - 1, 0, 0])
      rotate([0, 90, 0]) cylinder(h = paddle_w + 2, r = 1.6);
    // Spring pocket on the back of the lower half.
    translate([0, -paddle_t / 2 - 1, spring_z])
      rotate([-90, 0, 0]) cylinder(h = 3, r = 3.2);
    // Lightening pockets on the back, top and bottom.
    for (sz = [-1, 1])
      translate([-paddle_w / 2 + 6, -paddle_t / 2 - 1, sz * 30])
        rotate([-90, 0, 0]) cylinder(h = 3, r = 8);
  }
}

if (part == "box") box();
else if (part == "paddle") paddle();
else {
  box();
  translate([paddle_x, pivot_y, 0]) paddle();
}
