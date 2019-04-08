#ifndef __WIFI_MANAGER_H__
#define __WIFI_MANAGER_H__

#include "user_config.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include <string.h>

class WifiManager
{
public:
  WifiManager();
  ~WifiManager();
  bool IsConnectionSafe();
  void JoinNetwork(const char *ssid, const char *password);
  int ListNetworks(char *data, int size);
private:
  enum wifi_state
  {
    CONNECTING,
    CONNECTED,
    DISCONNECTED,
    AP_MODE
  };
  wifi_state state;
  int counter;
  bool scanning;
  wifi_ap_record_t networks[CONFIG_SCAN_AP_MAX];
  xQueueHandle lock;
  xTimerHandle timer;
  esp_err_t OnEvent(system_event_t *event);
  void OnConnect();
  void OnDisconnect();
  void OnScanDone();
  void OnTimeout();
  void Scan();
  void Lock();
  void Unlock();
  static void OnTimeoutCb(xTimerHandle timer);
  static esp_err_t EventHandler(void *ctx, system_event_t *event);
};

#endif
