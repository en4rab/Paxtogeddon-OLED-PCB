#include "pr_ble.h"
#include "pr_util.h"

// Requires: NimBLE-Arduino by h2zero v2.5.0 (install via Arduino Library Manager)
// Do NOT use the standard ESP32 Arduino BLE library — it is ~5x larger in flash and RAM.

namespace prBLE {

NimBLEClient *pClient = nullptr;
NimBLERemoteCharacteristic *pCharacteristic = nullptr;

void FitProM4Init(void) {
  NimBLEDevice::init("");
  NimBLEDevice::setPower(9);
  pClient = NimBLEDevice::createClient();
  pClient->setConnectTimeout(10000);  // milliseconds
}

void MaintainConnection(void) {
  if (pClient == nullptr) { return; }
  if (pClient->isConnected()) { return; }
  if (prSettings::bleMAC == "") { return; }

  pClient->disconnect();
  pCharacteristic = nullptr;

  NimBLEAddress bleAddress(prSettings::bleMAC.c_str(), prSettings::bleAddrType);
  pClient->connect(bleAddress);
  if (pClient->isConnected()) {
    NimBLERemoteService *pService = pClient->getService("6e400001-b5a3-f393-e0a9-e50e24dcca9d");
    if (pService) {
      pCharacteristic = pService->getCharacteristic("6e400002-b5a3-f393-e0a9-e50e24dcca9d");
    }
  } else {
    // Connection failed - back off before retrying to avoid starving WiFi
    vTaskDelay(pdMS_TO_TICKS(30000));
  }
}

class BLECallBack : public NimBLEScanCallbacks {
  void onResult(const NimBLEAdvertisedDevice* bleDevice) override {
    if (bleDevice->getName() == prSettings::bleManufacturerTarget.c_str()
        && prSettings::bleMAC == "") {
      NimBLEDevice::getScan()->stop();
      prSettings::bleAddrType = bleDevice->getAddress().getType();
      prSettings::bleMAC = bleDevice->getAddress().toString().c_str();
    }
  }
};

static BLECallBack bleScanCallbacks;

void ScanDevices(void) {
  if (pClient == nullptr) { return; }
  pClient->disconnect();
  pCharacteristic = nullptr;
  NimBLEScan *pBLEScan = NimBLEDevice::getScan();
  pBLEScan->setScanCallbacks(&bleScanCallbacks, false);
  pBLEScan->setActiveScan(true);
  // interval 100ms / window 50ms = 50% BLE duty cycle, leaves WiFi adequate airtime
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(50);
  pBLEScan->start(5000, false);
  pBLEScan->clearResults();
  if (prSettings::bleMAC != "") {
    prUtil::SaveSettings();
    prSettings::bleScanMode = false;
  } else {
    // Device not found - back off before next scan to avoid starving WiFi
    vTaskDelay(pdMS_TO_TICKS(30000));
  }
}

bool IsConnected(void) {
  return pClient != nullptr
      && pClient->isConnected()
      && pCharacteristic != nullptr;
}

void TriggerVibrate(void) {
  if (pClient == nullptr) { return; }
  if (!pClient->isConnected()) {
    MaintainConnection();
  }
  if (pCharacteristic == nullptr) { return; }
  uint8_t vibrateCmd[] = { 205, 0, 6, 18, 1, 11, 0, 1, 1 };
  uint8_t stopCmd[] = { 220, 0, 5, 21, 1, 0, 20, 1 };
  pCharacteristic->writeValue(vibrateCmd, 9);
  delay(500);
  pCharacteristic->writeValue(stopCmd, 8);
}

}  //namespace prBLE
