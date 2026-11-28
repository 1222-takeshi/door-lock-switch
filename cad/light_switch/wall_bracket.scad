// Wall bracket for the light-switch pusher, design v1 (Issue #18,
// revised by Issue #24). Coordinate frame: origin at the rocker
// centre, X to the right when facing the wall, Y out from the wall,
// Z up. Target switch (measured 2026-11-28, docs/LIGHT_SWITCH.md):
// plate 70 x 120 mm (X -35..35, Z -60..60, face at about Y = 6),
// rocker about 46 x 92 mm, face at about Y = 9, centre pivot.
//
// Layout: the adhesive pad sits on the wall to the RIGHT of the plate
// (the left side is close to a door frame). A shelf reaches left in
// front of the plate (Y from 7, clearing the plate face at 6) and
// carries the servo base. The servo lies on its side, shaft axis
// along X, shaft centre at (Y = shaft_y = 37, Z = 0). NOTE: on an
// SG90 the output shaft is offset from the centre of the body length
// by about 5 mm (shaft_offset_y; clones differ - confirm on the
// actual unit). The body is placed so the SHAFT lands at shaft_y:
// body centre Y = shaft_y + shaft_offset_y, shaft end toward the
// wall.
//
// Clearance statement (precise version): the arm sweeps X = 10.5..
// 20.5 at the pads (radius 21..31 from the shaft axis, felt included)
// and X = 16.5..20.5 at the hub and spokes. Bracket parts that enter
// X < 27 (servo base from X = 23, shelf from X = 23) stay at radius
// <= 17.5 from the shaft axis, inside the pads' inner radius, so the
// pads cannot meet them; the hub and spokes stay clear in X.
//
// Retention: two zip ties at tie_x wrap the servo body and pass
// through slots in the base and shelf on both sides of the body; the
// end cheek stops +X sliding. The adhesive pad takes the press
// reaction; gussets under the posts stiffen the shelf root.
//
// Mounting: re-peelable adhesive strips on the pad back (Y = 0 face).
// Foam tape about 1 mm thick shifts the unit out by that much; the
// stroke budget in docs/LIGHT_SWITCH.md covers it (press swing is
// bench-tuned). The four pad holes screw the bracket to a bench
// board for rig use. Printing needs supports under the shelf/base
// overhangs (see PRINTING.md).
//
// Render status: verified with OpenSCAD 2021.01 (Issues #18, #24).

$fn = 48;

// ---- Parameters ----
part = "bracket";      // "bracket", "ghost" or "assembled"

shaft_y = 37;          // design shaft distance from the wall
shaft_offset_y = 5.2;  // SG90 shaft offset from body centre (confirm)

pad_x0 = 38;           // adhesive pad, X range [pad_x0, pad_x1]
pad_x1 = 98;
pad_h = 80;            // pad height (Z), centred on 0
pad_t = 3;             // pad thickness (Y)

shelf_x0 = 23;         // shelf, X range [shelf_x0, pad_x1]
shelf_y0 = 7;          // shelf front edge clears the plate face (6)
shelf_y1 = 60;
shelf_z = -11;         // shelf bottom; thickness 3 -> top at -8

post_w = 8;            // posts joining the pad to the shelf

base_x0 = 23;          // servo base plate on the shelf
base_x1 = 50;
base_y0 = 26;
base_y1 = 59;
base_top = -5.9;       // servo body bottom rests here

body_x0 = 24;          // SG90-class body, lying on its side
body_x1 = 46.4;        // (22.4 along X, 22.2 along Y, 11.8 along Z)
body_w_y = 22.2;
body_h_z = 11.8;
body_y = shaft_y + shaft_offset_y;   // body centre Y

cheek_x0 = 46.6;       // end cheek beyond the body (+X side)
cheek_x1 = 49.6;

tie_x = [30, 41];      // zip-tie centre positions (slots at these X)

// ---- Bracket ----
module wall_bracket_solid() {
  // Adhesive pad on the wall.
  difference() {
    translate([pad_x0, 0, -pad_h / 2]) cube([pad_x1 - pad_x0, pad_t, pad_h]);
    // Bench-board screw holes.
    for (sx = [pad_x0 + 6, pad_x1 - 6], sz = [-pad_h / 2 + 6, pad_h / 2 - 6])
      translate([sx, -1, sz]) rotate([-90, 0, 0]) cylinder(h = pad_t + 2, r = 1.6);
  }
  // Posts from the pad out to the shelf (same Z range as the shelf).
  for (px = [58, 84])
    translate([px, 0, shelf_z]) cube([post_w, shelf_y0 + 3, 3]);
  // Gussets under the posts (triangle in the Y-Z plane, extruded in X).
  for (px = [58, 84])
    translate([px, 0, shelf_z]) rotate([90, 0, 0]) rotate([0, 90, 0])
      linear_extrude(height = post_w)
        polygon(points = [[0, 0], [16, 0], [0, -16]]);
  // Shelf in front of the switch plate.
  translate([shelf_x0, shelf_y0, shelf_z]) cube([pad_x1 - shelf_x0, shelf_y1 - shelf_y0, 3]);
  // Servo base plate (embedded 1 mm into the shelf top).
  translate([base_x0, base_y0, shelf_z + 2]) cube([base_x1 - base_x0, base_y1 - base_y0, base_top - (shelf_z + 2)]);
  // End cheek.
  translate([cheek_x0, base_y0, shelf_z + 2]) cube([cheek_x1 - cheek_x0, base_y1 - base_y0, 9 - (shelf_z + 2)]);
}

module wall_bracket() {
  difference() {
    wall_bracket_solid();
    // Zip-tie slots through base and shelf, both sides of the body.
    for (tx = tie_x, sy = [[27.5, 30.5], [53.8, 56.8]])
      translate([tx - 2, sy[0], shelf_z - 1]) cube([4, sy[1] - sy[0], base_top - shelf_z + 2]);
  }
}

// ---- Servo ghost (for the assembled view) ----
module servo_ghost() {
  // Body (centre offset from the shaft by shaft_offset_y).
  translate([body_x0, body_y - body_w_y / 2, -body_h_z / 2])
    cube([body_x1 - body_x0, body_w_y, body_h_z]);
  // Shaft boss and shaft at shaft_y, pointing -X.
  translate([19, shaft_y, 0]) rotate([0, 90, 0]) cylinder(h = 5, r = 3);
  translate([21, shaft_y, 0]) rotate([0, 90, 0]) cylinder(h = 3, r = 5.5);
}

if (part == "bracket") wall_bracket();
else if (part == "ghost") servo_ghost();
else { wall_bracket(); servo_ghost(); }
