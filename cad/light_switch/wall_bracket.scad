// Wall bracket for the light-switch pusher, design v1 (Issue #18).
//
// Coordinate frame: origin at the rocker centre, X to the right when
// facing the wall, Y out from the wall, Z up. The target switch
// (measured 2026-11-28, docs/LIGHT_SWITCH.md): plate 70 x 120 mm
// (X -35..35, Z -60..60, face at about Y = 6), rocker about 46 x 92 mm
// with its face at about Y = 9, centre pivot.
//
// Layout: the adhesive pad sits on the wall to the RIGHT of the plate
// (the left side is close to a door frame). A shelf reaches left in
// front of the plate (Y from 7, clearing the plate face at 6) and
// carries the servo base. The servo lies on its side with the shaft
// axis along X, shaft centre at (Y = 35, Z = 0), hub face at X = 24,
// so the two-spoke arm (arm_two_spoke.scad, arm plane about X = 20)
// presses the rocker edge region. Nothing overlaps the swing cylinder
// of the arm (radius 31 around the shaft axis for X < 27): the shelf
// starts at X = 28 and the servo base at X = 23, both clear of the
// arm plane (X <= 22.5) in X.
//
// Mounting: re-peelable adhesive strips on the pad back (Y = 0 face).
// Foam tape about 1 mm thick shifts the whole unit out by that much,
// which adds to the rest clearance and eats the same amount of press
// margin (budgeted in docs/LIGHT_SWITCH.md). The four holes in the pad
// are for screwing the bracket to a bench board while testing.
//
// Render status: verified with OpenSCAD 2021.01 (Issue #18).

// ---- Parameters ----
part = "bracket";      // "bracket", "ghost" or "assembled"

pad_x0 = 38;           // adhesive pad, X range [pad_x0, pad_x1]
pad_x1 = 98;
pad_h = 80;            // pad height (Z), centred on 0
pad_t = 3;             // pad thickness (Y)

shelf_x0 = 28;         // shelf, X range [shelf_x0, pad_x1]
shelf_y0 = 7;          // shelf front edge clears the plate face (6)
shelf_y1 = 52;
shelf_z = -11;         // shelf bottom; thickness 3 -> top at -8

post_w = 8;            // posts joining the pad to the shelf

base_x0 = 23;          // servo base plate on the shelf
base_x1 = 50;
base_y0 = 20;
base_y1 = 50;
base_top = -5.9;       // servo body bottom rests here

body_x0 = 24;          // SG90-class body, lying on its side
body_x1 = 46.4;        // (22.4 along X, 22.2 along Y, 11.8 along Z)
body_y = 35;           // body centre Y (= shaft height out from wall)
body_w_y = 22.2;
body_h_z = 11.8;

cheek_x0 = 46.6;       // end cheek beyond the body (+X side)
cheek_x1 = 49.6;

tie_x = [30, 41];      // zip-tie positions (ties wrap body + base plate)

// ---- Bracket ----
module wall_bracket() {
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
  // Shelf in front of the switch plate.
  translate([shelf_x0, shelf_y0, shelf_z]) cube([pad_x1 - shelf_x0, shelf_y1 - shelf_y0, 3]);
  // Servo base plate (embedded 1 mm into the shelf top for a solid union).
  translate([base_x0, base_y0, shelf_z + 2]) cube([base_x1 - base_x0, base_y1 - base_y0, base_top - (shelf_z + 2)]);
  // End cheek.
  translate([cheek_x0, base_y0, shelf_z + 2]) cube([cheek_x1 - cheek_x0, base_y1 - base_y0, 9 - (shelf_z + 2)]);
}

// The servo is held by two zip ties around the body and the base
// plate at tie_x, plus the end cheek against +X sliding.

// ---- Servo ghost (for the assembled view) ----
module servo_ghost() {
  // Body.
  translate([body_x0, body_y - body_w_y / 2, -body_h_z / 2])
    cube([body_x1 - body_x0, body_w_y, body_h_z]);
  // Shaft boss and shaft, pointing -X.
  translate([19, body_y, 0]) rotate([0, 90, 0]) cylinder(h = 5, r = 3);
  translate([21, body_y, 0]) rotate([0, 90, 0]) cylinder(h = 3, r = 5.5);
}

if (part == "bracket") wall_bracket();
else if (part == "ghost") servo_ghost();
else { wall_bracket(); servo_ghost(); }
