/*
 * door_lock.ino — サムターン錠の自動施錠＋照明スイッチ自動化（ESP32 / Arduino core）
 *
 * このスケッチはコンパイル環境のない場所で書かれた骨子であり、未検証です。
 * 実機でビルド確認・動作確認を行うまで本番運用しないでください。
 * パラメータ（角度・電圧換算・消費電流など）はすべて実測で調整する前提の仮値です。
 *
 *
 * 1. 遠隔操作は「施錠のみ」。このコードには解錠をネットワーク経由で行う経路は存在しません。
 *    WiFi は「電池低下の通知を送る」ためだけに使い、受信系（サーバ、MQTT購読、
 *    コマンド解析など）は一切持ちません。
 * 2. 手動操作が常に優先。サーボは動作時以外デタッチ＋電源遮断され、無通電時は
 *    サムターンがフリーになります（機構側に一方向クラッチ／退避機構が前提。
 *    retreatServo() はクラッチを前提に押し子を退避位置へ戻すだけで、
 *    サムターンを解錠方向へ駆動する処理ではありません）。
 * 3. 故障・電池切れ・通信断でも締め出しにならない（無通電＝フリーに倒れる）。
 * 4. 施錠の回し切りはマイクロスイッチで検出します。
 *
 *
 * ESP32 は deep sleep が常態。起床要因は (a) 施錠ボタン (b) 照明ボタン
 * (c) ドア開閉センサ（ドアが閉まった） (d) タイマー（定期点検）の4つ。
 * 起床したら必要な処理だけ行い、サーボ電源を切って再び deep sleep します。
 *
 *
 * ESP32Servo（Arduino Library Manager からインストール）
 */

#include <Arduino.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include "esp_sleep.h"
#include "driver/rtc_io.h"
// Battery state-of-charge estimation (discharge curves, interpolation,
// consumption integration, hybrid estimate, 3-level classification) lives in
// battery_estimator.h: pure C++, no Arduino dependencies, host-testable
// (see tests/test_battery.cpp). This sketch only measures voltages, keeps
// the consumption accumulator, and passes values to that module.
#include "battery_estimator.h"

// =====================================================================
// パラメータ（実機で実測して調整すること。ここに集約）
// =====================================================================

// ---- ピン割り当て ----
static const int PIN_SERVO_POWER_EN = 27;   // サーボ電源FETのゲート制御
static const int PIN_LOCK_SERVO     = 13;   // MG946R（サムターン駆動）の信号線
static const int PIN_LIGHT_SERVO    = 14;   // 9G（照明スイッチ押し）の信号線
static const int PIN_LOCK_BUTTON    = 32;   // 施錠ボタン（押すとHIGH、要プルダウン）
static const int PIN_LIGHT_BUTTON   = 33;   // 照明ボタン（押すとHIGH、要プルダウン）
static const int PIN_DOOR_SENSOR    = 35;   // リードスイッチ：ドア閉でHIGH（入力専用ピン、外付けプルダウン必須）
static const int PIN_LOCK_STATE     = 36;   // 回し切り検出マイクロスイッチ：施錠完了でLOW（入力専用ピン、外付けプルアップ必須）
static const int PIN_BATTERY_ADC    = 34;   // 電池電圧の分圧入力（ADC1。WiFi併用可のADC1を使うこと）
static const int PIN_STATUS_LED     = 2;    // 状態表示LED（開発ボード搭載LED想定）

// ---- サーボ電源FET ----
static const bool SERVO_POWER_ACTIVE_HIGH = true;  // HIGHでサーボ電源ON（回路に合わせて変更）

// ---- 施錠サーボ（MG946R）の角度［度］：実測で決めること ----
// 機構想定：待機位置（押し子が退避）→ 施錠完了位置まで一方向に回す。
static const int LOCK_SERVO_PARK_ANGLE = 10;    // 待機（退避）位置
static const int LOCK_SERVO_LOCK_ANGLE = 130;   // 施錠方向へ回し切る位置（サムターン形状で実測）
static const int LOCK_SERVO_MIN_US     = 500;   // パルス幅下限（個体で実測）
static const int LOCK_SERVO_MAX_US     = 2400;  // パルス幅上限（個体で実測）
static const int LOCK_SWEEP_STEP_MS    = 12;    // 1度ごとの待ち時間（突入電流と衝撃の抑制）
static const uint32_t LOCK_TIMEOUT_MS  = 3000;  // 回し切り検出のタイムアウト

// ---- 照明スイッチ用9Gの角度［度］：実測で決めること ----
static const int LIGHT_SERVO_REST_ANGLE = 20;   // 待機位置
static const int LIGHT_SERVO_PUSH_ANGLE = 75;   // スイッチを押し切る位置
static const uint32_t LIGHT_PUSH_HOLD_MS = 300; // 押し続ける時間

// ---- タイマー起床 ----
// 定期点検の間隔。電池残量チェックと「ドア閉＋未施錠なら自動施錠（施錠忘れ防止）」を行う。
static const uint64_t TIMER_WAKEUP_US = 6ULL * 3600ULL * 1000000ULL;  // 6時間

// ---- 電池・残量推定 ----
// 電池種別：0=単3アルカリ×4（6V系）、1=単3ニッケル水素×5（6V系）
// 放電曲線テーブル・セル数・容量は battery_estimator.h のプロファイル側にある。
#define BATTERY_TYPE_ALKALINE_AA_X4 0
#define BATTERY_TYPE_NIMH_AA_X5     1
static const int BATTERY_TYPE = BATTERY_TYPE_ALKALINE_AA_X4;

// 分圧回路：電池＋ → R_TOP → ADCピン → R_BOT → GND
// 例：R_TOP=120kΩ, R_BOT=100kΩ なら比は (120+100)/100 = 2.20
// 満充電直後（アルカリ 1.65V×4=6.6V）でも ADC が 3.0V 程度に収まること。
static const float DIVIDER_RATIO = 2.20f;
static const float ADC_CAL       = 1.00f;  // テスター実測との照合で補正する係数

// 施錠動作中の負荷時電圧がこの値（セルあたり）を下回ったら、内部抵抗の増大とみなし
// 電圧換算の結果によらず「要交換」に倒す。
static const float LOAD_CELL_MIN_V = 1.00f;

// 積算補正用：1イベントあたりの消費電荷の仮値［mAh］（すべて実測で調整）
static const float MAH_PER_LOCK_CYCLE = 1.0f;  // 施錠1回（サーボ駆動＋起床オーバーヘッド）
static const float MAH_PER_LIGHT_PUSH = 0.4f;  // 照明押し1回
static const float MAH_PER_TIMER_WAKE = 0.3f;  // タイマー起床1回（WiFiなし）
static const float MAH_PER_NOTIFY     = 1.5f;  // WiFi通知1回（接続＋送信）

// 3段階判定のしきい値［%］
static const float LEVEL_OK_MIN  = 40.0f;  // これ以上：十分
static const float LEVEL_LOW_MIN = 15.0f;  // これ以上：低下／未満：要交換

// ---- WiFi通知（スタブ）----
// 送信先はプレースホルダ。実運用では自分の環境の通知先（Webhook等）に差し替えること。
// 繰り返し：WiFiは「送るだけ」。受信して何かを操作する機能は作らない（安全要件1）。
static const bool  WIFI_NOTIFY_ENABLED = false;  // 送信先を設定するまで false
static const char* WIFI_SSID           = "YOUR_SSID";  // TODO: 実環境に合わせて設定
static const char* WIFI_PASSWORD       = "YOUR_PASSWORD";
static const char* NOTIFY_URL          = "";  // TODO: 通知先URL（空なら送信しない）
static const uint32_t WIFI_TIMEOUT_MS  = 10000;

// デバッグ出力（実運用では電池節約のため 0 にしてもよい）
#define DEBUG_SERIAL 1
#if DEBUG_SERIAL
  #define DBG(...)   Serial.print(__VA_ARGS__)
  #define DBGLN(...) Serial.println(__VA_ARGS__)
#else
  #define DBG(...)
  #define DBGLN(...)
#endif

// =====================================================================
// 永続カウンタ（deep sleep をまたいで保持）
// =====================================================================
RTC_DATA_ATTR static uint32_t g_lockCycleCount = 0;    // 施錠回数（積算補正用）
RTC_DATA_ATTR static float    g_consumedMah    = 0.0f; // 推定消費電荷の積算
RTC_DATA_ATTR static float    g_lastBatteryPct = 100.0f;
RTC_DATA_ATTR static uint32_t g_bootCount      = 0;

// =====================================================================
// サーボ電源（FET）
// =====================================================================
static void servoPower(bool on) {
  digitalWrite(PIN_SERVO_POWER_EN, on == SERVO_POWER_ACTIVE_HIGH ? HIGH : LOW);
  if (on) delay(30);  // レール安定待ち
}

// =====================================================================
// 電池残量推定（計算本体は battery_estimator.h。ここは計測と受け渡しのみ）
// =====================================================================

// Battery profile selected by BATTERY_TYPE (curves / cells / capacity are
// defined in battery_estimator.h).
static battery::Profile activeProfile() {
  return (BATTERY_TYPE == BATTERY_TYPE_NIMH_AA_X5)
             ? battery::nimhAaX5Profile()
             : battery::alkalineAaX4Profile();
}

static int cellCount() { return activeProfile().cells; }

// 電池電圧を読む［V］。複数回サンプルして平均する。
static float readBatteryVolts() {
  const int N = 16;
  uint32_t sumMv = 0;
  for (int i = 0; i < N; i++) {
    sumMv += analogReadMilliVolts(PIN_BATTERY_ADC);
    delay(2);
  }
  float pinVolts = (sumMv / (float)N) / 1000.0f;
  return pinVolts * DIVIDER_RATIO * ADC_CAL;
}

// 総合推定：電圧法（放電曲線の線形補間）と積算法のうち保守側（小さい方）を採用する。
// loadedCellVolts は施錠動作中に測った負荷時電圧（セルあたり）。負でなければ負荷チェックも行う。
static float estimateBatteryPercent(float restVolts, float loadedCellVolts) {
  float pct = battery::estimatePercent(activeProfile(), restVolts,
                                       loadedCellVolts, g_consumedMah,
                                       LOAD_CELL_MIN_V);
  g_lastBatteryPct = pct;
  return pct;
}

static battery::Level batteryLevel(float pct) {
  return battery::levelFromPercent(pct, LEVEL_OK_MIN, LEVEL_LOW_MIN);
}

static const char* batteryLevelName(battery::Level lv) {
  switch (lv) {
    case battery::Level::Ok:  return "十分";
    case battery::Level::Low: return "低下";
    default:                  return "要交換";
  }
}

// LEDで残量段階を知らせる（十分=1回、低=2回、要交換=3回の点滅）
static void indicateBattery(battery::Level lv) {
  int blinks = (lv == battery::Level::Ok) ? 1 : (lv == battery::Level::Low) ? 2 : 3;
  for (int i = 0; i < blinks; i++) {
    digitalWrite(PIN_STATUS_LED, HIGH);
    delay(120);
    digitalWrite(PIN_STATUS_LED, LOW);
    delay(120);
  }
}

// =====================================================================
// WiFi通知（スタブ：電池低下／要交換のときだけ、起床時に送る）
// =====================================================================
static void notifyBatteryIfNeeded(battery::Level lv, float pct) {
  if (lv == battery::Level::Ok) return;
  if (!WIFI_NOTIFY_ENABLED || NOTIFY_URL[0] == '\0') {
    DBGLN("[notify] 通知は未設定のためスキップ（スタブ）");
    return;
  }
  DBG("[notify] WiFi接続中... ");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_TIMEOUT_MS) {
    delay(200);
  }
  if (WiFi.status() != WL_CONNECTED) {
    DBGLN("接続失敗（通知は諦めて sleep する）");
    WiFi.mode(WIFI_OFF);
    return;
  }
  // TODO: ここに通知送信処理を実装する（例：HTTP POST で Webhook を叩く）。
  // 送る内容は「電池残量 pct%／段階」のみ。受信系は作らないこと（安全要件1）。
  DBG("[notify] 送信スタブ: 電池 ");
  DBG(pct, 0);
  DBG("% / ");
  DBGLN(batteryLevelName(lv));
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  g_consumedMah += MAH_PER_NOTIFY;
}

// =====================================================================
// センサ
// =====================================================================
static bool isDoorClosed() {
  return digitalRead(PIN_DOOR_SENSOR) == HIGH;  // リードスイッチ：ドア閉でHIGH
}

static bool isLocked() {
  return digitalRead(PIN_LOCK_STATE) == LOW;    // マイクロスイッチ：回し切りでLOW
}

// =====================================================================
// 施錠シーケンス
//   FETでサーボ電源ON → 施錠方向へ回す → 回し切り検出 → 退避 → 電源OFF
//   戻り値：施錠完了を確認できたら true
// =====================================================================
static Servo g_lockServo;

static bool runLockSequence() {
  if (!isDoorClosed()) {
    DBGLN("[lock] ドアが開いているため施錠しません");
    return false;
  }
  if (isLocked()) {
    DBGLN("[lock] 既に施錠済みです");
    return true;
  }

  servoPower(true);
  g_lockServo.setPeriodHertz(50);
  g_lockServo.attach(PIN_LOCK_SERVO, LOCK_SERVO_MIN_US, LOCK_SERVO_MAX_US);
  g_lockServo.write(LOCK_SERVO_PARK_ANGLE);
  delay(200);

  // 施錠方向へ少しずつ回す。途中で負荷時電圧をサンプルする（残量推定用）。
  float loadedVolts = -1.0f;
  uint32_t start = millis();
  bool reached = false;
  for (int angle = LOCK_SERVO_PARK_ANGLE; angle <= LOCK_SERVO_LOCK_ANGLE; angle++) {
    g_lockServo.write(angle);
    if (angle == (LOCK_SERVO_PARK_ANGLE + LOCK_SERVO_LOCK_ANGLE) / 2) {
      loadedVolts = readBatteryVolts();  // 動作中（負荷あり）の電圧
    }
    if (isLocked()) { reached = true; break; }
    if (millis() - start > LOCK_TIMEOUT_MS) break;
    delay(LOCK_SWEEP_STEP_MS);
  }
  // 角度を回し切ってもスイッチが入らない場合は、最終位置で少し待って再確認
  if (!reached) {
    uint32_t waitStart = millis();
    while (millis() - waitStart < 500) {
      if (isLocked()) { reached = true; break; }
      delay(20);
    }
  }

  if (reached) {
    g_lockCycleCount++;
    g_consumedMah += MAH_PER_LOCK_CYCLE;
    DBGLN("[lock] 施錠完了を確認");
  } else {
    // 回し切り未検出：締め出し防止のため、これ以上の駆動はしない。
    // サーボは退避・電源OFFしてフリーに戻す（手動操作は常に可能）。
    DBGLN("[lock] 警告: 回し切りを検出できませんでした（機構の詰まり等を確認）");
  }

  // 退避：クラッチ（一方向）機構を前提に、押し子を待機位置へ戻す。
  // サムターンはクラッチにより解錠方向へは駆動されない。解錠動作ではない。
  for (int angle = LOCK_SERVO_LOCK_ANGLE; angle >= LOCK_SERVO_PARK_ANGLE; angle--) {
    g_lockServo.write(angle);
    delay(LOCK_SWEEP_STEP_MS);
  }
  delay(100);
  g_lockServo.detach();
  servoPower(false);  // 無通電＝サーボフリー＝手動優先

  // 負荷時電圧を残量推定へ反映（セルあたりに換算して渡す）
  float loadedCell = (loadedVolts >= 0.0f) ? loadedVolts / cellCount() : -1.0f;
  float rest = readBatteryVolts();
  float pct = estimateBatteryPercent(rest, loadedCell);
  battery::Level lv = batteryLevel(pct);
  indicateBattery(lv);
  notifyBatteryIfNeeded(lv, pct);
  return reached;
}

// =====================================================================
// 照明スイッチ押し（施錠とは独立）
// =====================================================================
static Servo g_lightServo;

static void pushLightSwitch() {
  servoPower(true);
  g_lightServo.setPeriodHertz(50);
  g_lightServo.attach(PIN_LIGHT_SERVO, 500, 2400);  // 9Gの一般的な範囲（実測で調整）
  g_lightServo.write(LIGHT_SERVO_REST_ANGLE);
  delay(200);
  g_lightServo.write(LIGHT_SERVO_PUSH_ANGLE);
  delay(LIGHT_PUSH_HOLD_MS);
  g_lightServo.write(LIGHT_SERVO_REST_ANGLE);
  delay(200);
  g_lightServo.detach();
  servoPower(false);
  g_consumedMah += MAH_PER_LIGHT_PUSH;
  DBGLN("[light] 照明スイッチを押しました");
}

// =====================================================================
// 電池交換リセット：施錠ボタンを起床直後に長押しし続けたら積算をリセット
// （新しい電池に替えたときの操作。5秒長押しで判定）
// =====================================================================
static bool checkBatteryResetGesture() {
  if (digitalRead(PIN_LOCK_BUTTON) != HIGH) return false;
  uint32_t start = millis();
  while (digitalRead(PIN_LOCK_BUTTON) == HIGH) {
    if (millis() - start > 5000) {
      g_consumedMah = 0.0f;
      g_lastBatteryPct = 100.0f;
      DBGLN("[battery] 積算をリセットしました（電池交換）");
      for (int i = 0; i < 5; i++) {  // 確認の高速点滅
        digitalWrite(PIN_STATUS_LED, HIGH); delay(60);
        digitalWrite(PIN_STATUS_LED, LOW);  delay(60);
      }
      return true;
    }
    delay(20);
  }
  return false;
}

// =====================================================================
// deep sleep へ入る
//   起床要因：施錠ボタン／照明ボタン／ドアセンサ（いずれもHIGHでext1起床）
//             ＋ タイマー（定期点検）
// =====================================================================
static void enterDeepSleep() {
  // ext1用ピンのRTCプルダウンを有効化（sleep中のレベル保持）
  rtc_gpio_pulldown_en((gpio_num_t)PIN_LOCK_BUTTON);
  rtc_gpio_pulldown_en((gpio_num_t)PIN_LIGHT_BUTTON);
  rtc_gpio_pulldown_en((gpio_num_t)PIN_DOOR_SENSOR);

  uint64_t mask = (1ULL << PIN_LOCK_BUTTON) |
                  (1ULL << PIN_LIGHT_BUTTON) |
                  (1ULL << PIN_DOOR_SENSOR);
  esp_sleep_enable_ext1_wakeup(mask, ESP_EXT1_WAKEUP_ANY_HIGH);
  esp_sleep_enable_timer_wakeup(TIMER_WAKEUP_US);

  DBGLN("[sleep] deep sleep へ移行します");
#if DEBUG_SERIAL
  Serial.flush();
#endif
  esp_deep_sleep_start();
}

// =====================================================================
// メイン
// =====================================================================
void setup() {
#if DEBUG_SERIAL
  Serial.begin(115200);
  delay(50);
#endif
  g_bootCount++;

  pinMode(PIN_SERVO_POWER_EN, OUTPUT);
  servoPower(false);  // 起動直後は必ずサーボ無通電（フリー）にする
  pinMode(PIN_STATUS_LED, OUTPUT);
  digitalWrite(PIN_STATUS_LED, LOW);
  pinMode(PIN_LOCK_BUTTON, INPUT_PULLDOWN);
  pinMode(PIN_LIGHT_BUTTON, INPUT_PULLDOWN);
  pinMode(PIN_DOOR_SENSOR, INPUT);   // 外付けプルダウン前提（入力専用ピン）
  pinMode(PIN_LOCK_STATE, INPUT);    // 外付けプルアップ前提（入力専用ピン）
  analogReadResolution(12);
  analogSetPinAttenuation(PIN_BATTERY_ADC, ADC_11db);

  esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
  uint64_t ext1Status = esp_sleep_get_ext1_wakeup_status();

  // 電池交換リセットのジェスチャを最初に見る
  if (cause == ESP_SLEEP_WAKEUP_EXT1 && (ext1Status & (1ULL << PIN_LOCK_BUTTON))) {
    if (checkBatteryResetGesture()) {
      enterDeepSleep();
    }
  }

  if (cause == ESP_SLEEP_WAKEUP_TIMER) {
    // 定期点検：残量チェック＋施錠忘れ防止
    g_consumedMah += MAH_PER_TIMER_WAKE;
    float rest = readBatteryVolts();
    float pct = estimateBatteryPercent(rest, -1.0f);
    battery::Level lv = batteryLevel(pct);
    DBG("[timer] 電池 ");
    DBG(pct, 0);
    DBG("% / ");
    DBGLN(batteryLevelName(lv));
    notifyBatteryIfNeeded(lv, pct);
    runLockSequence();  // ドア閉＋未施錠のときだけ実際に施錠する（内部で判定）
  } else if (cause == ESP_SLEEP_WAKEUP_EXT1) {
    if (ext1Status & (1ULL << PIN_LIGHT_BUTTON)) {
      pushLightSwitch();
    }
    if (ext1Status & (1ULL << PIN_LOCK_BUTTON)) {
      runLockSequence();
    }
    if (ext1Status & (1ULL << PIN_DOOR_SENSOR)) {
      // ドアが閉まったタイミング：施錠忘れ防止として自動施錠する
      delay(500);  // 閉まり際のチャタリング回避
      runLockSequence();
    }
  } else {
    // 初回起動（電源投入・リセット）：残量だけ測って表示し、何も駆動しない
    float rest = readBatteryVolts();
    float pct = estimateBatteryPercent(rest, -1.0f);
    DBG("[boot] 電池 ");
    DBG(pct, 0);
    DBGLN("%");
    indicateBattery(batteryLevel(pct));
  }

  enterDeepSleep();
}

void loop() {
  // deep sleep 運用のため loop は使わない（setup の末尾で sleep する）
}
