// bracket.scad - servo bracket for the light-switch pusher (Issue #7).
//
// One printed piece: wall plate + shelf + servo cradle. The servo lies
// on its side so its shaft axis is horizontal and parallel to the wall;
// the pusher arm (pusher_arm.scad) rotates in a plane perpendicular to
// the wall and presses the rocker below/in front of the bracket.
//
// Coordinates: origin at the rocker centre on the wall plane.
//   X: right (wall-parallel), Y: out from the wall, Z: up.
// Units: mm. Servo dimensions are nominal SG90-class values; adjust the
// body_* parameters to the actual unit before printing a final version.
//
// Render status: verified with OpenSCAD 2021.01 (Issue #12) - renders
// as a single valid 2-manifold solid; exported bounding box is
// 60.0 x 40.2 x 80.0 mm, matching the parameters below.

// ---- Parameters ----
plate_w = 60;          // wall plate width  (X)
plate_h = 80;          // wall plate height (Z)
plate_t = 3;           // wall plate thickness (Y)
shaft_z = 28;          // shaft centre height above rocker centre
shaft_y = 26;          // shaft centre distance out from the wall
body_x = 23.0;         // servo body size along X (its height, on side)
body_y = 22.4;         // servo body size along Y (its length)
body_z = 12.0;         // servo body size along Z (its width, on side)
clr = 0.5;             // clearance around the body
base_t = 3;            // cradle base thickness
lip_h = 5;             // locating lip height on the cradle base
wall_t = 3;            // cradle end-wall thickness
shelf_t = 4;           // shelf thickness (plate to cradle)

$fn = 32;

// ---- Derived ----
base_z = shaft_z - body_z / 2 - clr - base_t;   // cradle base bottom
base_y0 = shaft_y - (body_y / 2 + 3);           // cradle base front edge
base_y1 = shaft_y + (body_y / 2 + 3);           // cradle base back edge
base_x0 = -(body_x / 2 + wall_t + 3);
base_x1 = body_x / 2 + 6;                       // room for the shaft/hub

module wall_plate() {
  difference() {
    translate([-plate_w / 2, 0, -plate_h / 2])
      cube([plate_w, plate_t, plate_h]);
    // Corner holes for screwing the plate to a bench board (rig use).
    // Unused in wall use, where the back is adhesive-mounted.
    for (sx = [-1, 1], sz = [-1, 1])
      translate([sx * (plate_w / 2 - 6), -1, sz * (plate_h / 2 - 6)])
        rotate([-90, 0, 0]) cylinder(h = plate_t + 2, r = 1.7);
  }
}

module shelf() {
  // Horizontal shelf from the plate to the cradle base. It embeds 1 mm
  // into the plate, the cradle base footprint and the base slab so the
  // union is a valid 2-manifold (face-only contact is not; Issue #12).
  translate([-15, plate_t - 1, base_z - shelf_t])
    cube([30, base_y0 - plate_t + 2, shelf_t + 1]);
  // Two stiffening ribs under the shelf.
  for (sx = [-1, 1])
    translate([sx * 13 - 2, plate_t - 1, base_z - shelf_t - 18])
      cube([4, base_y0 - plate_t + 2, 18 + shelf_t]);
}

module cradle() {
  // Base slab.
  translate([base_x0, base_y0, base_z])
    cube([base_x1 - base_x0, base_y1 - base_y0, base_t]);
  // End wall on the side opposite the shaft.
  translate([base_x0, base_y0, base_z])
    cube([wall_t, base_y1 - base_y0, base_t + body_z + 2 * clr]);
  // Locating lips along both Y edges.
  for (sy = [0, 1])
    translate([base_x0, sy == 0 ? base_y0 : base_y1 - 2,
               base_z + base_t])
      cube([base_x1 - base_x0, 2, lip_h]);
  // Zip-tie slot across the base to retain the servo (v0 retention).
}

module bracket() {
  difference() {
    union() { wall_plate(); shelf(); cradle(); }
    // Zip-tie slot through the cradle base.
    translate([-2, base_y0 - 1, base_z - 1])
      cube([4, (base_y1 - base_y0) + 2, base_t + 2]);
  }
}

bracket();
