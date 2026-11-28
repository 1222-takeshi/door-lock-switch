// Bench-only test sketch for the light-switch servo (9 g, GPIO14).
//
// BENCH ONLY - not for installation. It drives the servo on a fixed
// cycle so the pusher geometry can be tuned on the rig without
// flashing the full firmware.
//
// Design v1 (docs/LIGHT_SWITCH.md): the arm has two spokes and rests
// at the centre angle. A press swings the servo by PRESS_DELTA
// degrees to one side (direction A) or the other (direction B),
// holds, and returns to rest. Which direction presses the rocker's
// upper half depends on how the horn is mounted - record it on the
// rig during U6.
//
// Cycle: every 4 s, alternate A and B presses.
// Serial commands (115200):
//   'a' press direction A once    'b' press direction B once
//   '+' / '-'  PRESS_DELTA +- 2 deg (range 4..30)
//   'r' report current settings
//
// Status: compiles with the firmware build check toolchain
// (esp32:esp32 core 4.0.0, ESP32Servo 3.2.1). Geometry values are
// provisional until bench unit U6 fixes them.

#include <ESP32Servo.h>

static const int SERVO_PIN = 14;
static const int REST_ANGLE = 90;     // centre rest (two-spoke arm)
static const int HOLD_MS = 300;
static const int STEP_MS_PER_DEG = 12;
static const unsigned long CYCLE_MS = 4000;

static int pressDelta = 11;           // design v1 nominal swing
static Servo servo;
static int currentAngle = REST_ANGLE;
static bool nextIsA = true;

static void moveTo(int target) {
  int step = (target > currentAngle) ? 1 : -1;
  while (currentAngle != target) {
    currentAngle += step;
    servo.write(currentAngle);
    delay(STEP_MS_PER_DEG);
  }
}

static void press(bool dirA) {
  int target = REST_ANGLE + (dirA ? pressDelta : -pressDelta);
  Serial.printf("[bench] press %s: %d -> %d\n", dirA ? "A" : "B",
                currentAngle, target);
  moveTo(target);
  delay(HOLD_MS);
  moveTo(REST_ANGLE);
  Serial.println("[bench] back to rest");
}

static void report() {
  Serial.printf("[bench] rest=%d delta=%d hold=%dms\n", REST_ANGLE,
                pressDelta, HOLD_MS);
}

void setup() {
  Serial.begin(115200);
  servo.setPeriodHertz(50);
  servo.attach(SERVO_PIN, 500, 2400);
  servo.write(REST_ANGLE);
  currentAngle = REST_ANGLE;
  Serial.println("[bench] light servo bench v1 - bench only, not for installation");
  report();
}

void loop() {
  if (Serial.available()) {
    char c = (char)Serial.read();
    if (c == 'a') press(true);
    else if (c == 'b') press(false);
    else if (c == '+' && pressDelta < 30) { pressDelta += 2; report(); }
    else if (c == '-' && pressDelta > 4) { pressDelta -= 2; report(); }
    else if (c == 'r') report();
  }
  static unsigned long last = 0;
  if (millis() - last >= CYCLE_MS) {
    last = millis();
    press(nextIsA);
    nextIsA = !nextIsA;
  }
}
