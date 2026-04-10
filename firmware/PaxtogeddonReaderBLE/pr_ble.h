#ifndef PR_BLE_H
#define PR_BLE_H

#include <Arduino.h>
#include <NimBLEDevice.h>

#include "pr_settings.h"

namespace prBLE {

void FitProM4Init(void);
void MaintainConnection(void);
void ScanDevices(void);
void TriggerVibrate(void);
bool IsConnected(void);

}  //namespace prBLE

#endif
