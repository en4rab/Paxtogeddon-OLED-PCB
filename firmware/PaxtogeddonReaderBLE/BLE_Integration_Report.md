# BLE Integration Report: Merging VibrateBLE into HidogeddonReaderV1

**Date:** 2026-04-07  
**Target Hardware:** ESP32-C3 Super Mini

---

## 1. Executive Summary

The original architecture used two separate ESP32 boards: the main HidogeddonReaderV1 (ESP32-C3 Super Mini) and a secondary VibrateBLE board (FireBeetle 2 ESP32-E) that communicated with a FitPro smartwatch. The two boards communicated via a physical GPIO wire — EXT_PIN (GPIO 5) on the main board pulsed high to trigger a BLE scan/vibration on the secondary board.

The ESP32-C3 has **built-in BLE 5.0 hardware**, making the secondary board redundant. This document describes the integration that was carried out, including the issues encountered during testing and the fixes applied to resolve them.

---

## 2. Hardware Constraints

| Resource | ESP32-C3 Super Mini | Notes |
|---|---|---|
| Flash | 4MB | Default partition scheme |
| SRAM | 400KB | WiFi + BLE coexistence is tight — see section 6 |
| CPU | Single-core RISC-V @ 160MHz | No dual-core; task scheduling matters |
| BLE | BLE 5.0 (built-in) | Shares RF with WiFi; time-multiplexed |
| WiFi | 2.4GHz 802.11 b/g/n | Already in use |

**Coexistence:** The ESP32-C3 shares a single 2.4GHz RF for both WiFi and BLE. Espressif's SDK coexistence scheduler time-multiplexes the two radios automatically. Scan duty cycle and connection attempt frequency must be managed carefully by the application to avoid starving WiFi — see section 6.

---

## 3. Files Changed

### New files added to root of project

| File | Purpose |
|---|---|
| `hr_ble.h` | BLE namespace header |
| `hr_ble.cpp` | BLE client implementation (ported from `VibrateBLE/hr_ble.cpp`) |

### Modified files

| File | Changes |
|---|---|
| `hr_settings.h` | Added BLE field declarations |
| `hr_settings.cpp` | Added BLE field defaults; `LOGO_HTML` changed from `const String` to `const char[]`; logo image changed from JPEG to GIF to reduce size |
| `hr_util.cpp` | `SettingsInit` loads/saves BLE settings; `RequestTriggerBLEScan` sets flag instead of pulsing GPIO |
| `hr_server.cpp` | `/triggerBleScan` route updated; `/saveSettings` handles BLE checkboxes; all HTML responses converted to `AsyncResponseStream` |
| `hr_html.cpp` | Menu BLE link condition updated; BLE section added to Settings page |
| `HidogeddonReaderV1.ino` | Added `hr_ble.h` include; added `BLETask`; conditional BLE init in `setup()` |

---

## 4. Implementation Detail

### 4.1 hr_ble.h / hr_ble.cpp

Ported from `VibrateBLE/hr_ble.cpp` with the following changes:

- `InputTriggerHandler()` (GPIO 17 polling) removed — trigger now comes from the `bleTriggerPending` flag set by the web server route
- `StatusLED()` calls removed — the main project has its own LED control
- New public function `TriggerVibrate()` replaces the private `Vibrate()` static
- BLE MAC stored in the existing `"hrSettings"` NVS namespace rather than a separate one

**Vibration commands (unchanged from VibrateBLE):**
```cpp
uint8_t vibrateCmd[9] = { 205, 0, 6, 18, 1, 11, 0, 1, 1 };
uint8_t stopCmd[8]    = { 220, 0, 5, 21, 1, 0, 20, 1 };
```

**Target device:** FitPro M4/M5/M6 — identified by BLE advertisement name `"716"`  
**Service UUID:** `6e400001-b5a3-f393-e0a9-e50e24dcca9d` (Nordic UART)  
**TX Characteristic:** `6e400002-b5a3-f393-e0a9-e50e24dcca9d`

### 4.2 hr_settings.h / hr_settings.cpp

New fields added to the `hrSettings` namespace:

```cpp
const String bleManufacturerTarget = "716"; // BLE advertisement name to scan for
bool         bleEnabled            = false;  // Master on/off; gates NimBLE init
String       bleMAC                = "";     // Saved MAC of paired FitPro
bool         bleScanMode           = false;  // true when no MAC saved yet
volatile bool bleTriggerPending    = false;  // Inter-task vibrate trigger flag
```

`bleEnabled` defaults to `false` — NimBLE is never initialised unless the user explicitly enables BLE in the Settings page. This is important: NimBLE consumes ~50-60KB of heap on init; leaving it disabled when no watch is paired avoids this entirely.

`LOGO_HTML` was changed from `const String` to `const char[]`. A `const String` copies its entire contents from flash into heap at boot. A `const char[]` remains in flash (DROM) and is read directly from there — zero heap cost. The logo image was also changed from JPEG to GIF, reducing its base64 size.

### 4.3 hr_util.cpp

`SettingsInit()` — added load of `BLE_EN` and `BLE_MAC` from NVS. Sets `bleScanMode = true` if BLE is enabled but no MAC is saved yet.

`SaveSettings()` — added save of `BLE_EN` and `BLE_MAC` to NVS.

`RequestTriggerBLEScan()` — replaced the 24-second blocking GPIO pulse loop with a single flag set:
```cpp
hrSettings::bleTriggerPending = true;
```

`CheckTriggerBLEScan()` — now a no-op; the BLE task monitors `bleTriggerPending` directly.

### 4.4 hr_server.cpp

`/triggerBleScan` route — comment updated to reflect it now triggers the integrated BLE stack rather than an external board.

`/saveSettings` POST handler — added handling for two new form parameters:
- `bleenabled` (checkbox) — sets `hrSettings::bleEnabled`
- `blerscan` (checkbox) — clears `hrSettings::bleMAC` to force re-pairing on next boot

**All HTML-sending routes converted from `request->send(200, "text/html", html)` to `AsyncResponseStream`** — see section 6.2 for why this was necessary.

### 4.5 hr_html.cpp

`Menu()` — BLE trigger link condition changed from `EXT_PIN != 255` to `bleEnabled`:
```cpp
html += hrSettings::bleEnabled ? "<a href=\"/triggerBleScan\">Trigger BLE Vibrate</a><hr>" : "";
```

`ViewSettings()` — new BLE section added at the bottom of the settings form:
- **BLE Enabled** checkbox
- Paired device MAC display (shows `"None"` if not yet paired)
- **Re-scan** checkbox (clears MAC on save, forces scan on next boot)

### 4.6 HidogeddonReaderV1.ino

Added `#include "hr_ble.h"`.

Added `BLETask` FreeRTOS task. Key design points:
- **8-second startup delay** before first BLE operation, allowing the WiFi AP, DNS server and web server to be fully running before BLE uses the radio
- `FitProM4Init()` called inside the task after the delay, not in `setup()` — this defers the ~50-60KB NimBLE heap allocation until after the web server has already served its first requests
- Task monitors `bleTriggerPending` flag and calls `TriggerVibrate()` when set

```cpp
void BLETask(void* p) {
  vTaskDelay(pdMS_TO_TICKS(8000));  // wait for WiFi AP + web server
  hrBLE::FitProM4Init();            // allocate NimBLE heap after first pages served
  while (true) {
    if (hrSettings::bleScanMode) {
      hrBLE::ScanDevices();
    } else {
      hrBLE::MaintainConnection();
    }
    if (hrSettings::bleTriggerPending) {
      hrSettings::bleTriggerPending = false;
      hrBLE::TriggerVibrate();
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
```

`setup()` — `xTaskCreate(BLETask, ...)` called only when `bleEnabled` is true. `FitProM4Init()` is no longer called from `setup()`.

---

## 5. GPIO Changes

| GPIO | Before integration | After integration |
|---|---|---|
| GPIO 5 (EXT_PIN) | Pulsed high for 24s to signal secondary board | No longer used for BLE; remains available |
| GPIO 17 (secondary board) | Trigger input from main board | Eliminated — secondary board removed |

---

## 6. Issues Encountered During Testing and Fixes Applied

### 6.1 Captive Portal Unreachable After Enabling BLE

**Symptom:** After enabling BLE in Settings and rebooting, connecting devices could no longer reach the captive portal page — the connection would time out.

**Root cause:** Two problems combined:

1. `bleScanMode` was `true` on first boot (no MAC saved yet). `ScanDevices()` was called immediately and ran with a scan window of `99ms` out of every `100ms` interval — effectively 99% BLE radio duty cycle. This starved the WiFi AP of airtime, causing DNS responses and HTTP connections to time out.

2. When no device was found, `ScanDevices()` returned and the task looped back after only a 10ms `vTaskDelay`, immediately starting another 5-second scan. This continued indefinitely with no pause.

**Fix applied in `hr_ble.cpp`:**
- Scan window reduced from `99ms` to `50ms` (50% duty cycle), leaving WiFi adequate airtime
- Added a `vTaskDelay(30000ms)` backoff after a failed scan before retrying
- Added a `vTaskDelay(30000ms)` backoff in `MaintainConnection()` after a failed connection attempt
- Connection timeout reduced from 10 seconds to 5 seconds

```cpp
pBLEScan->setInterval(100);
pBLEScan->setWindow(50);   // was 99 — reduced to 50% duty cycle
...
} else {
  vTaskDelay(pdMS_TO_TICKS(30000));  // back off 30s before next scan
}
```

**Fix applied in `HidogeddonReaderV1.ino`:**
- Added 8-second startup delay in `BLETask` before the first BLE operation, ensuring the AP and DNS server are fully running before BLE touches the radio

### 6.2 Blank Page After BLE Enabled

**Symptom:** After fixing the captive portal timeout, the page loaded but was completely blank — no HTML content rendered.

**Root cause:** Heap exhaustion. `BLEDevice::init()` (called from `FitProM4Init()`) allocates the entire NimBLE host stack — approximately 50-60KB of heap — in a single synchronous call. After this allocation, the heap was sufficiently fragmented that the `String` concatenation chain in `hr_html.cpp`'s `Header()` function could not obtain a large enough contiguous block. Arduino `String` silently returns an empty string on allocation failure, so every page came back blank.

The `Header()` function alone builds ~3KB of HTML including a ~6KB base64-encoded inline logo image, peaking at ~9KB in one string before any other section was appended. The full home page string reached ~10-12KB.

**Fixes applied:**

**`hr_settings.cpp` — `LOGO_HTML` type change:**  
Changed from `const String` (copies entire ~6KB logo into heap at boot) to `const char[]` (stays in flash, zero heap cost). The logo image was also changed from JPEG to GIF, reducing its base64-encoded size.

**`HidogeddonReaderV1.ino` — deferred `FitProM4Init()`:**  
Moved `FitProM4Init()` from `setup()` into `BLETask`, after the 8-second delay. This means NimBLE's ~50-60KB heap allocation happens only after the web server has already served its first requests, rather than before any page is ever loaded.

**`hr_server.cpp` — `AsyncResponseStream` for all HTML routes:**  
All 8 routes that previously built a complete page `String` and sent it with `request->send(200, "text/html", html)` were rewritten to use `AsyncResponseStream`. Each section is printed to the stream individually:

```cpp
// Before — entire page in one String, peaks at ~10KB heap
String html = hrHTML::Header(autoRefresh, requestedPage);
html += hrHTML::Menu(autoRefresh);
html += hrHTML::CardData(...);
html += hrHTML::Footer();
request->send(200, "text/html", html);

// After — each section allocated, printed, and freed individually
AsyncResponseStream *response = request->beginResponseStream("text/html");
response->print(hrHTML::Header(autoRefresh, requestedPage));
response->print(hrHTML::Menu(autoRefresh));
response->print(hrHTML::CardData(...));
response->print(hrHTML::Footer());
request->send(response);
```

`AsyncResponseStream` buffers output in 1460-byte TCP chunks and flushes to the network as each chunk fills. Peak heap at any moment is the size of the **largest single section** (~3KB) rather than the entire page (~10KB).

---

## 7. First-Time Setup Instructions

1. Flash the firmware to the ESP32-C3 Super Mini
2. Connect to the `Hidogeddon` WiFi AP (password: `13371337`)
3. Navigate to `http://192.168.3.10` → **Options → Settings**
4. Tick **BLE Enabled**, leave **Re-scan** unticked, click **Save**
5. The device reboots; after ~8 seconds it begins scanning for a FitPro M4/M5/M6 watch
6. Once the watch is found its MAC is saved to NVS — subsequent boots connect directly without scanning
7. To pair a different watch: go to Settings, tick **Re-scan**, click **Save**, reboot

---

## 8. Library Required

**NimBLE-Arduino by h2zero v2.5.0** — must be installed via Arduino Library Manager or added to the `lib/` directory. The standard ESP32 Arduino BLE library must **not** be used; it is approximately 5× larger in flash and RAM footprint.

| Library | Flash | RAM |
|---|---|---|
| ESP32 Arduino BLE (standard) | ~500KB | ~60KB |
| NimBLE-Arduino | ~180KB | ~50-60KB at init |
