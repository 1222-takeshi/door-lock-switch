# サムターン錠＋照明スイッチ ファームウェア（door_lock.ino）

ESP32（Arduino core）用のスケッチ骨子です。**コンパイル環境のない場所で書かれており未検証**です。実機でビルド確認とベンチ動作確認をしてから組み付けてください。

計画書は `../PLAN.html` を参照。安全要件（遠隔は施錠のみ／手動が常に優先／故障・電池切れ時はフリー／施錠状態の検出）はコード構造にも反映されています。

## 前提環境

- ボード：ESP32開発ボード（HW-0054）
- ライブラリ：**ESP32Servo**（Arduino IDE のライブラリマネージャからインストール）
- サーボ：MG946R（サムターン駆動）、9G（照明スイッチ押し）
- 電源：単3形×4アルカリ（6V）またはニッケル水素×5。スケッチ先頭の `BATTERY_TYPE` で切り替える

## 動作の流れ

ESP32 は deep sleep が常態で、次の4要因で起床し、処理後に再び sleep します。

| 起床要因 | 動作 |
|---|---|
| 施錠ボタン（GPIO32） | 施錠シーケンス |
| 照明ボタン（GPIO33） | 照明スイッチ押し |
| ドアセンサ（GPIO35、ドアが閉まった） | 施錠シーケンス（施錠忘れ防止） |
| タイマー（6時間ごと） | 電池残量チェック＋施錠忘れ防止 |

電池交換時は、施錠ボタンを5秒以上長押しすると消費積算がリセットされます。

## 配線の考え方

### ピン割り当て（スケッチ先頭で変更可）

| GPIO | 接続先 | 備考 |
|---|---|---|
| 27 | サーボ電源FETのゲート | `SERVO_POWER_ACTIVE_HIGH` で極性を切替 |
| 13 | MG946R の信号線 | |
| 14 | 9G の信号線 | |
| 32 | 施錠ボタン | 押すと 3.3V に接続（内部プルダウン使用） |
| 33 | 照明ボタン | 同上 |
| 35 | リードスイッチ（ドア開閉） | **入力専用ピンのため外付けプルダウン（10kΩ）必須**。ドア閉で接点が閉じ HIGH |
| 36 | 回し切り検出マイクロスイッチ | **入力専用ピンのため外付けプルアップ（10kΩ）必須**。施錠完了で GND に落ち LOW |
| 34 | 電池電圧の分圧出力 | ADC1（WiFiと併用できるのはADC1側のみ） |
| 2 | 状態表示LED | 残量段階を点滅回数で表示（1回=十分、2回=低下、3回=要交換） |

### サーボ電源（FET遮断）

- 電池＋ → FET → サーボ用電源レール（MG946R と 9G の電源線）→ GND
- ロジックレベルで駆動できるMOSFETを使う。ハイサイド（P-ch）でもローサイド（N-ch）でもよいが、`SERVO_POWER_ACTIVE_HIGH` を回路に合わせる
- 待機中はFETをOFFにしてサーボへの通電を完全に止める。これにより待機電流ゼロと「無通電＝サーボフリー（手動優先）」を同時に満たす
- サーボレールには大きめの電解コンデンサ（例：1000µF級）を並列に入れ、MG946Rの突入・動作電流による電圧降下を抑える
- ESP32 自体は電池から（開発ボードのUSB／VIN経由ではなく）3.3V系で給電する構成を別途検討すること。開発ボードのレギュレータ経由だと待機電流が増える（下記「実機で最初に確認すべきこと」参照）

### 電池電圧の分圧

- 電池＋ → R_TOP（120kΩ）→ GPIO34 → R_BOT（100kΩ）→ GND を想定し、`DIVIDER_RATIO = 2.20`
- 満充電直後のアルカリ4本（6.6V）でも ADC 入力が約3.0Vに収まる組み合わせにすること
- 分圧抵抗は常時接続だと電池を少しずつ消費する（6V／220kΩ ≒ 27µA）。気になる場合は分圧の上側もFETで間欠駆動にする（現骨子では未実装）

### 機構側の前提

- サムターンの押し子には**一方向クラッチまたは退避機構**が必須。ファームウェアの退避動作（`retreatServo()`）は、クラッチによりサムターンが解錠方向へ駆動されないことを前提に、押し子を待機位置へ戻すだけのもの
- 回し切り検出マイクロスイッチは「施錠し切った位置」でのみ押される場所に取り付ける

## パラメータ調整手順

すべて `door_lock.ino` 先頭のパラメータブロックで変更します。再コンパイルで反映されます。

1. **サーボ角度（最重要）**：機構に仮組みし、サーボテスター等でサムターンが「待機位置→施錠完了位置」まで回る角度を実測し、`LOCK_SERVO_PARK_ANGLE` / `LOCK_SERVO_LOCK_ANGLE` に設定する。あわせて MG946R 個体のパルス幅範囲（`LOCK_SERVO_MIN_US` / `MAX_US`）も実測する。照明側は `LIGHT_SERVO_REST_ANGLE` / `PUSH_ANGLE`（押し切ってスイッチが入る角度＋少しの余裕）
2. **電池電圧の換算**：テスターで電池電圧を測りながらシリアル出力の読み値と比べ、`DIVIDER_RATIO` と `ADC_CAL` を補正する（ESP32 の ADC は個体差が大きい）
3. **消費電荷の仮値**：施錠1回の消費を実測（電流計を直列に、または電源の積算機能で）し、`MAH_PER_LOCK_CYCLE` 等に反映する
4. **放電曲線テーブル**：初号機で「無負荷電圧のログ」と「実際に電池切れになるまでの日数・施錠回数」を取り、`CURVE_ALKALINE` / `CURVE_NIMH` と容量値を実態に寄せる
5. **通知の有効化**：通知先（Webhook等）を決め、`WIFI_SSID` / `WIFI_PASSWORD` / `NOTIFY_URL` を設定し、`WIFI_NOTIFY_ENABLED` を true にして、notify スタブ内の送信処理を実装する。**受信系（操作コマンドの受け付け）は実装しないこと**

## Module layout / host tests (battery estimation)

The battery state-of-charge estimation logic has been extracted from
`door_lock.ino` into a pure C++ header so it can be tested on a host machine:

| File | Role |
|---|---|
| `battery_estimator.h` | Header-only library (namespace `battery`, C++11, no Arduino dependencies). Discharge-curve tables for AA alkaline x4 and AA NiMH x5, linear interpolation, consumption (coulomb-count) estimate, hybrid estimate (takes the smaller of voltage-based and count-based, with an under-load voltage-collapse check), and 3-level classification (`Level::Ok / Low / Replace`). Profiles are provided by `battery::alkalineAaX4Profile()` and `battery::nimhAaX5Profile()`. |
| `door_lock.ino` | Arduino glue only: ADC measurement (`readBatteryVolts`), the `g_consumedMah` accumulator persisted across deep sleep, profile selection via `BATTERY_TYPE`, and passing values into `battery::estimatePercent()` / `battery::levelFromPercent()`. Pin assignments, sequences, and externally visible behavior are unchanged. |
| `tests/test_battery.cpp` | Host-side tests (plain `assert`, no framework). Covers curve endpoints and interior points, interpolation midpoints, monotonic decrease over the full voltage sweep, consumption-based decrease and clamping, hybrid min-selection, the under-load collapse rule, and the 3-level threshold boundaries. |

Run the host tests:

```sh
cd firmware
g++ -std=c++11 -Wall -Wextra -I . tests/test_battery.cpp -o /tmp/test_battery
/tmp/test_battery   # prints "test_battery: all tests passed"
```

Note: the tests exercise `battery_estimator.h` only. `door_lock.ino` itself
remains **uncompiled and unverified** (no Arduino build environment here);
verify it on real hardware as described in the checklist below. Calibration
values (curve tables, capacity, per-event mAh costs, thresholds) are still
provisional and must be fitted to measurements; the curve/capacity values now
live in `battery_estimator.h`, while per-event costs and thresholds remain in
the parameter block at the top of `door_lock.ino`.

## 実機で最初に確認すべきこと

- [ ] ビルドが通ること（ESP32Servo のバージョン差異で API が変わっていないか）
- [ ] MG946R 単体：回転方向と角度の対応、必要トルクでサムターンが回り切るか
- [ ] 回し切りマイクロスイッチが施錠完了位置でのみ確実に入ること（チャタリング含む）
- [ ] サーボ無通電時にサムターンが手で軽く回ること（手動優先・フリーの確認）
- [ ] FET の極性：sleep 中にサーボレールへ電圧が出ていないこと
- [ ] deep sleep 電流の実測（開発ボードはLDOと電源LEDで数百µA〜数mA流れることがある。電池寿命の見積もりはこの値で決まる）
- [ ] ADC 読み値とテスター実測の誤差（→ `ADC_CAL` 補正）
- [ ] 施錠動作中の負荷時電圧：電池が新しいうちに基準値を記録（末期判定 `LOAD_CELL_MIN_V` の妥当性確認）
- [ ] ext1 起床（ボタン2種・ドアセンサ）とタイマー起床がそれぞれ正しく起きること
- [ ] ドアが開いているときに施錠シーケンスが走らないこと
- [ ] 既に施錠済みのときに無駄な駆動をしないこと
- [ ] ネットワーク経由の操作経路が存在しないこと（WiFi は通知送信のみ）の再確認
