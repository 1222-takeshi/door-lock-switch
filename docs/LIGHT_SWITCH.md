# Light-Switch Pusher Design (Issue #7)

Status: design v0, written before the target switch is measured. Values
marked TBD are fixed once a photo and dimensions of the target switch are
available; everything else is designed to be adjustable so a first bench
rig can be built before that.

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

## Mechanism

- The servo shaft is horizontal and parallel to the wall. The pusher arm
  rotates in a plane perpendicular to the wall, so the arm tip travels
  toward and away from the rocker face.
- One press cycle: rest angle -> push angle -> hold -> rest angle. The
  rocker toggles on each press; there is no pull-back mechanism.
- Firmware parameters (provisional, in `firmware/door_lock.ino`):
  rest 20 deg, push 75 deg, hold 300 ms, servo on GPIO14.

## Stroke budget

Effective arm length L = 22 mm (shaft centre to pad centre), rotation
55 deg (20 -> 75 deg). If the arm works between 60 deg and 25 deg from
the wall plane, the tip travel toward the wall is

    L * (sin 60 - sin 25) = 22 * (0.866 - 0.423) = about 9.7 mm

Budget for that travel:

| Item | Allowance |
|---|---|
| Rest gap (tip to rocker face) | about 4 mm |
| Rocker press travel | about 4 mm (TBD, measure) |
| Pad compression / margin | about 1.5 mm |

- Force estimate: an SG90-class servo is nominally about 1.8 kg cm at
  4.8 V, giving roughly 0.8 kg (about 8 N) at a 22 mm tip. This is an
  estimate from nominal catalogue values, not a measurement. The bench
  rig test measures the real value; if it is short, shorten the arm
  (L = 18 mm gives roughly 1.0 kg) or move the contact point further
  from the rocker pivot.
- The rest gap is set with printed shims (1 mm steps) and slots in the
  bracket, not by firmware angles alone.

## Pusher arm

- Printed arm clamped over the stock servo horn with screws; the spline
  itself is not printed.
- Overall length about 28 mm, width 10 mm tapering to a 12 mm tip pad,
  thickness 4 mm, printed on the Adventurer 3 (PLA/PETG for v0).
- Tip pad carries a 2 mm silicone or felt pad: protects the rocker and
  absorbs over-travel. Tip edges rounded (R3 or more).
- A mechanical stop on the bracket limits the maximum press depth even
  if the servo is driven to a wrong angle.

## Mounting (reversible)

- Primary (v1): removable adhesive strips (re-peelable type) on the flat
  wall beside the switch plate; bracket back about 60 x 80 mm. Only the
  servo unit (target under 50 g) sits on the wall; the ESP32 and battery
  box live in a separate case connected by a cable, so battery changes
  do not disturb the mount.
- Candidate (v2): a bracket sharing the switch plate frame screws.
  Japanese 1-gang mounting frames use an 83.5 mm screw pitch as the
  standard, but the actual screw type and pitch on the target plate
  must be confirmed first. Not used in v1; if adopted, work stays on
  the cosmetic plate only and never touches terminals or wiring.
- No new holes, no wall modification. Removal is peel-only for v1.
  Test the adhesive on a small inconspicuous area first: weak wallpaper
  can tear on removal.

## Parameters waiting on the target switch (TBD)

| Parameter | Needed for | Source |
|---|---|---|
| Plate width x height, rocker count | Bracket outline | Photo + tape measure |
| Rocker width x height, pivot side | Arm length, contact point | Photo + tape measure |
| Rocker press travel and force | Stroke budget, arm length | Bench measurement |
| Clear space above/beside the plate | Servo placement | Photo |

## Bench rig (before any wall mounting)

- The bracket and arm are fixed to a board; a dummy rocker (a printed
  lever on a hinge, spring-loaded) stands in for the real switch so the
  stroke and force can be measured without the wall.
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
