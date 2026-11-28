# Light-Switch Pusher Design (Issue #7)

Status: design v1 (2026-11-28), updated with the measured target switch
(bench unit U0, Issue #17). Design v0 assumed a single press point; the
target rocker turned out to be a seesaw type, so v1 uses a two-spoke arm
that presses the upper and lower halves alternately. Values still marked
TBD are fixed on the bench (U6) or at the wall trial.

## Scope and safety

- A 9 g servo physically presses a wall light-switch rocker, in the same
  spirit as a commercial switch bot. Pressing toggles the light.
- Manual operation always wins: in the rest position the pusher arm does
  not touch the rocker, and a finger can operate the switch normally at
  any time. This clearance is a hard requirement, because a small servo's
  gear train cannot be back-driven easily when unpowered.
- The project safety requirements apply unchanged: remote operation is
  lock-only for the door unit, no remote unlock path exists anywhere in
  the system, and WiFi is used only to send low-battery notifications
  (no receive path).
- Nothing is wired into the switch or the mains. The device sits outside
  the switch plate and can be removed without a trace.

## Target switch (measured 2026-11-28)

- Plate: 70 x 120 mm (standard Japanese 1-gang wide plate; height read
  directly against the ruler, width confirmed by ratio to the rocker).
- Rocker: a single wide rocker (Panasonic Cosmo-series style, with the
  pilot-lamp dot), about 46 x 92 mm, seesaw pivot at its centre. The
  face sits about 9 mm proud of the wall (plate face about 6 mm).
- The rocker toggles by pressing the upper half in or the lower half
  in; end travel is about 3 mm at the ends (to be confirmed at the wall
  trial). Pressing near the centre does nothing - the pivot is there.
- Surroundings: open wall on the right of the plate; a door frame is
  close on the left. The unit mounts on the right side.

## Mechanism (v1)

- The servo shaft is horizontal and parallel to the wall, at the height
  of the rocker centre, about 33 mm out from the wall. The arm plane
  sits over the rocker edge region (about 20 mm right of centre), where
  the wide rocker can be pressed reliably.
- The arm has two spokes at about +/-40 deg from the wall-pointing
  direction, pads at 26 mm radius. Swinging the servo one way brings
  the upper pad onto the rocker's upper half (press point about 17 mm
  above centre); swinging the other way presses the lower half. Each
  swing toggles the light; there is no pull-back mechanism.
- At the rest (centre) position both pads clear the rocker face by
  about 4 mm, so manual operation is unobstructed (hard requirement).
- Firmware parameters: the v0 values (rest 20 deg, push 75 deg in
  `firmware/door_lock.ino`) are superseded by this geometry - centre
  rest with about +/-40 deg presses, hold 300 ms. The exact angles are
  set during bench tuning (U6) and updated in the firmware then.

## Stroke budget (v1)

Geometry: shaft centre 33 mm out from the wall, rocker face at 9 mm,
spoke radius R = 26 mm, spokes at +/-40 deg at rest.

- Rest clearance: pad distance from the wall is
  33 - 26 * cos(40 deg) = about 13.1 mm, i.e. about 4.1 mm clear of the
  rocker face. Manual operation is unobstructed.
- Press: rotating a spoke to point at the wall moves its pad to
  33 - 26 = 7 mm from the wall. At the press point (17 mm from the
  pivot) the rocker face starts at about 10.1 mm when that half is out
  (9 mm + tilt) and needs about 1.6 mm of travel to toggle (about
  1.1 mm to flat plus over-travel). The pad delivers about 3.1 mm -
  roughly 1.5 mm of margin for pad compression and tolerances.
- Force estimate: an SG90-class servo is nominally about 1.8 kg cm at
  4.8 V, giving about 0.69 kg (about 6.8 N) at the 26 mm pad. Wide
  rockers of this type press with roughly 2-4 N (TBD, measured on the
  rig), so the margin is about 2x. If the rig shows a shortfall, a
  22 mm spoke variant raises the force to about 8 N at the cost of
  press margin; re-check the geometry before adopting it.
- The rest gap is fine-tuned with printed shims (1 mm steps), not by
  firmware angles alone.

## Pusher arm (v1)

- Two-spoke printed arm clamped over the stock servo horn with screws;
  the spline itself is not printed. Spokes at about +/-40 deg, pads at
  26 mm radius, pad width 12 mm, thickness 4 mm, printed on the
  Adventurer 3 (PLA/PETG for v1).
- Pads carry a 2 mm silicone or felt layer: protects the rocker and
  absorbs over-travel. Pad edges rounded (R3 or more).
- A mechanical stop on the bracket limits the swing to about +/-45 deg
  even if the servo is driven to a wrong angle.
- CAD for the arm and the wall bracket is Issue #18; the bench-rig arm
  (single spoke, PR #10) stays as the rig's force-measurement tool.

## Mounting (reversible)

- Primary (v1): removable adhesive strips (re-peelable type) on the flat
  wall beside the switch plate, on the right side (the left side is
  close to a door frame); bracket back about 60 x 80 mm. Only the servo
  unit (target under 50 g) sits on the wall; the ESP32 and battery box
  live in a separate case connected by a cable, so battery changes do
  not disturb the mount.
- Note: the bench-rig bracket (PR #10) is centred on the rocker and
  would overlap the 70 x 120 mm switch plate on the wall; the wall
  bracket is a separate variant (Issue #18) whose adhesive pad sits
  beside the plate and whose servo hub reaches over the rocker edge.
- Candidate (v2): a bracket sharing the switch plate frame screws.
  Japanese 1-gang mounting frames use an 83.5 mm screw pitch as the
  standard, but the actual screw type and pitch on the target plate
  must be confirmed first. Not used in v1; if adopted, work stays on
  the cosmetic plate only and never touches terminals or wiring.
- No new holes, no wall modification. Removal is peel-only for v1.
  Test the adhesive on a small inconspicuous area first: weak wallpaper
  can tear on removal.

## Parameters from the target switch

| Parameter | Value | Source |
|---|---|---|
| Plate width x height | 70 x 120 mm | Photo + ruler (2026-11-28) |
| Rocker count / type | 1, wide seesaw, centre pivot | Photo |
| Rocker width x height | about 46 x 92 mm | Photo ratios to the plate |
| Rocker face protrusion | about 9 mm (plate face about 6 mm) | Side photo + ruler |
| Clear space | Right side open; door frame close on the left | Photo |
| Rocker press travel and force | about 3 mm at the ends (est.); force TBD | Bench measurement (U6) |

## Bench rig (before any wall mounting)

- The bracket and arm are fixed to a board; a dummy rocker stands in
  for the real switch so the stroke and force can be measured without
  the wall. From v1 the dummy matches the real rocker: 46 x 92 mm
  paddle, centre pivot, about 3 mm end travel (Issue #13).
- Push force is measured by pressing the tip against a kitchen scale.
- A bench-only sketch (`tools/light_servo_bench/`) repeats one press
  cycle every few seconds using the same angles as the firmware, so the
  geometry can be tuned without flashing the full firmware.

## Acceptance criteria (fixed before testing)

- 20 consecutive presses toggle the (dummy, then real) rocker 20 times.
- At rest, the switch can be operated by hand with normal effort and
  full travel.
- Removal of the v1 mount leaves no trace.
- No double toggles from a single press cycle.
