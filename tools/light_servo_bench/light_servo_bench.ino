/*
 * light_servo_bench.ino - bench-only test sketch for the light-switch
 * pusher (Issue #7). NOT for installation.
 *
 * Repeats one press cycle (rest -> push -> hold -> rest) every 4 s and
 * prints the cycle count, so the pusher geometry (arm length, rest gap,
 * angles) can be tuned on the bench rig with a dummy rocker. Sending
 * 'p' over serial triggers one extra cycle immediately.
 *
 * Angles and hold time mirror the provisional values in
 * firmware/door_lock.ino; tune them here first, then copy the result
 * back. No WiFi, no deep sleep, no lock-servo code.
 *
 * Unverified: written without a build environment, like the main
 * sketch. Compile it in the Arduino IDE before relying on it.
 *
 * Library: ESP32Servo (Arduino Library Manager).
 */

#include <Arduino.h>
#include <ESP32Servo.h>

static const int PIN_LIGHT_SERVO = 14;  // same pin as the main firmware

static const int REST_ANGLE = 20;       // tune on the rig
static const int PUSH_ANGLE = 75;       // tune on the rig
static const int MIN_US = 500;          // 9 g servo provisional range
static const int MAX_US = 2400;
static const uint32_t HOLD_MS = 300;
static const uint32_t CYCLE_MS = 4000;

static Servo g_servo;
static uint32_t g_cycles = 0;

static void pressOnce() {
  g_servo.write(PUSH_ANGLE);
  delay(HOLD_MS);
  g_servo.write(REST_ANGLE);
  delay(300);
  g_cycles++;
  Serial.print("[bench] press cycle ");
  Serial.println(g_cycles);
}

void setup() {
  Serial.begin(115200);
  delay(50);
  g_servo.setPeriodHertz(50);
  g_servo.attach(PIN_LIGHT_SERVO, MIN_US, MAX_US);
  g_servo.write(REST_ANGLE);
  Serial.println("[bench] light servo bench: rest position, cycling every 4 s, 'p' = press now");
}

void loop() {
  static uint32_t last = 0;
  if (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == 'p') pressOnce();
  }
  if (millis() - last >= CYCLE_MS) {
    last = millis();
    pressOnce();
  }
}
