# Light-switch pusher CAD (Issue #7)

Parametric OpenSCAD models for the bench rig and the v1 wall mount.
Design rationale and acceptance criteria: `docs/LIGHT_SWITCH.md`.

## Files

| File | Part |
|---|---|
| `bracket.scad` | One-piece wall plate + shelf + servo cradle. Servo lies on its side (shaft axis horizontal, parallel to the wall) and is retained by a zip tie through the base slot (v0 retention). Corner holes let the same plate be screwed to a bench board for rig use. |
| `pusher_arm.scad` | Arm that clamps over the stock servo horn. Default effective length 22 mm (see the stroke budget in the design doc). |
| (next) `dummy_rocker.scad` | Spring-loaded dummy rocker for the bench rig; added once the bracket/arm fit is confirmed, so its pivot can reuse the measured bracket geometry. |

## Parameters to set before a final print

- `bracket.scad`: `body_x/y/z` to the actual servo unit (nominal SG90
  values are the defaults), `shaft_z` / `shaft_y` to the target switch
  geometry (needs the target switch photo/dimensions).
- `pusher_arm.scad`: `arm_len` if the force measurement on the rig
  calls for a shorter arm (more tip force, less travel).

## Printing

- Adventurer 3, PLA or PETG, 0.2 mm layers, 3 walls, about 30% infill.
- Bracket prints plate-down; the shelf and cradle need no supports at
  the default dimensions if the overhangs are kept as modelled - check
  the slicer preview.
- Render status: verified with OpenSCAD 2021.01 (Issue #12). Both
  models render as single valid solids; exported bounding boxes match
  the design parameters (bracket 60.0 x 40.2 x 80.0 mm, arm
  35.5 x 14.0 x 4.0 mm). Still treat the first print as a fit prototype
  against the actual servo unit.

## Bench rig assembly

1. Screw the bracket plate to a vertical board (corner holes).
2. Seat the servo in the cradle, zip-tie it through the base slot,
   fit the stock horn and the pusher arm.
3. Flash `tools/light_servo_bench/` and tune rest/push angles against
   the dummy rocker (or a kitchen scale for tip force).

## Wall trial (v1, after the rig passes)

1. Clean the wall area beside the plate; test the removable adhesive
   on a small inconspicuous spot first (weak wallpaper can tear).
2. Mount the bracket with removable adhesive strips; the ESP32 and
   battery box stay in a separate case (cable to the servo).
3. Confirm the acceptance criteria in `docs/LIGHT_SWITCH.md`
   (20/20 toggles, manual operation unobstructed at rest, traceless
   removal, no double toggles).
