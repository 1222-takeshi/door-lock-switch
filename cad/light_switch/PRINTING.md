# Printing manifest - light-switch pusher (design v1)

Status: **awaiting print approval** (2026-11-29). Printing itself is an
owner-approved action; this manifest is the package to approve. All
models are render-verified in OpenSCAD 2021.01 as single valid solids.

Filament and time figures are estimates computed from the mesh volumes
(PLA, 3 walls, 30% infill, 0.2 mm layers, Adventurer 3 at an effective
~7 mm3/s; real times vary roughly +/-40% with slicer settings).

## Print set A - bench tuning (U6) for design v1

Print these four first. The wall bracket doubles as the rig mount: its
adhesive pad has screw holes for the bench board, and the dummy rocker
stands in for the real switch.

| # | Part | Source (part=) | Size (mm) | Filament | Time |
|---|---|---|---|---|---|
| A1 | Wall bracket | `wall_bracket.scad` (`bracket`) | 75 x 52 x 80 | ~21 g | ~1.1 h |
| A2 | Two-spoke arm | `arm_two_spoke.scad` | 10 x 34 x 47 | ~3 g | ~0.2 h |
| A3 | Dummy rocker box | `dummy_rocker.scad` (`box`) | 79 x 26 x 124 | ~35 g | ~1.8 h |
| A4 | Dummy rocker paddle | `dummy_rocker.scad` (`paddle`) | 46 x 4 x 92 | ~12 g | ~0.6 h |

Set A total: about 71 g, about 3.7 h.

Orientation and notes:

- A1: pad back flat on the bed (rotate so the model's Y axis points
  up). All other features become vertical walls in that orientation;
  check the slicer preview for the servo-base underside.
- A2: flat as modelled (thickness along the build Z). No supports.
- A3: flange down, opening up, as modelled. The largest part; the
  lightening pockets are on the paddle, not the box.
- A4: flat as modelled. Needs an M3 screw (pivot) and a pen-class
  compression spring at assembly time (not printed parts).

## Print set B - optional, rig force measurement

The v0 rig parts, only needed if the tip force is measured with the
single-spoke arm before the v1 tuning (the design doc's force margin
is about 2x, so this set can be skipped unless U6 shows a shortfall).

| # | Part | Source | Size (mm) | Filament | Time |
|---|---|---|---|---|---|
| B1 | Rig bracket | `bracket.scad` | 60 x 40 x 80 | ~17 g | ~0.9 h |
| B2 | Rig pusher arm | `pusher_arm.scad` | 36 x 14 x 4 | ~1 g | ~0.1 h |

Set B total: about 18 g, about 1.0 h.

## After printing

1. Assemble the dummy rocker (paddle + M3 pivot + spring) and screw
   the wall bracket and dummy box to the bench board at the rocker
   spacing of the design (rocker centre = bracket shaft height).
2. Run bench unit U6 with `tools/light_servo_bench/` (design v1
   version): tune the press swing for 20/20 toggles on the dummy,
   measure tip force on a kitchen scale, confirm rest clearance.
3. Only after U6 passes: wall trial per `docs/LIGHT_SWITCH.md`
   acceptance criteria.
