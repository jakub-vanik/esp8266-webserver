#include "wifi_manager.h"

static const char *TAG = "wifi_manager";

WifiManager::WifiManager()
{
  ESP_LOGI(TAG, "constructing");
  state = CONNECTING;
  counter = 0;
  scanning = false;
  lock = xQueueCreate(1, 1);
  timer = xTimerCreate("wifi_timer", 10000 / portTICK_RATE_MS, true, this, OnTimeoutCb);
  xTimerStart(timer, 0);
  esp_event_loop_init(EventHandler, this);
  wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
  ESP_LOGD(TAG, "initializing");
  esp_wifi_init(&config);
  esp_wifi_set_auto_connect(false);
  esp_wifi_set_mode(WIFI_MODE_STA);
  ESP_LOGD(TAG, "starting");
  esp_wifi_start();
  ESP_LOGD(TAG, "connecting");
  esp_wifi_connect();
  ESP_LOGD(TAG, "constructed");
}

WifiManager::~WifiManager()
{
  ESP_LOGI(TAG, "destructing");
  xTimerDelete(timer, 0);
  vQueueDelete(lock);
  ESP_LOGD(TAG, "deinitializing");
  esp_wifi_deinit();
  ESP_LOGD(TAG, "destructed");
}

bool WifiManager::IsConnectionSafe()
{
  if (state == CONNECTED)
  {
    return true;
  }
  return false;
}

void WifiManager::JoinNetwork(const char *ssid, const char *password)
{
  ESP_LOGI(TAG, "joining network %s", ssid);
  Lock();
  if (state == CONNECTING || state == CONNECTED)
  {
    ESP_LOGD(TAG, "disconnecting");
    esp_wifi_disconnect();
  }
  wifi_config_t config;
  memset(&config, 0, sizeof(wifi_config_t));
  strncpy((char *)config.sta.ssid, ssid, sizeof(config.sta.ssid));
  strncpy((char *)config.sta.password, password, sizeof(config.sta.password));
  esp_wifi_set_config(ESP_IF_WIFI_STA, &config);
  ESP_LOGD(TAG, "connecting");
  esp_wifi_connect();
  state = CONNECTING;
  counter = WIFI_CONN_SCAN_INTERVAL;
  xTimerReset(timer, 0);
  Unlock();
}

int WifiManager::ListNetworks(char *data, int size)
{
  int count = 0;
  Lock();
  for (int i = 0; i < CONFIG_SCAN_AP_MAX; i++)
  {
    if (networks[i].rssi != 0)
    {
      int length = snprintf(data, size, "%d\t%s\n", networks[i].rssi, networks[i].ssid);
      if (length <= size)
      {
        data += length;
        size -= length;
        count += length;
      }
      else
      {
        break;
      }
    }
  }
  Unlock();
  return count;
}

esp_err_t WifiManager::OnEvent(system_event_t *event)
{
  switch(event->event_id)
  {
    case SYSTEM_EVENT_STA_GOT_IP:
      OnConnect();
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      OnDisconnect();
      break;
    case SYSTEM_EVENT_SCAN_DONE:
      OnScanDone();
      break;
    default:
      break;
  }
  return ESP_OK;
}

void WifiManager::OnConnect()
{
  ESP_LOGI(TAG, "connected");
  Lock();
  if (state == AP_MODE)
  {
    ESP_LOGD(TAG, "switching off AP");
    esp_wifi_set_mode(WIFI_MODE_STA);
  }
  state = CONNECTED;
  counter = WIFI_CONN_SCAN_INTERVAL;
  xTimerReset(timer, 0);
  Unlock();
}

void WifiManager::OnDisconnect()
{
  ESP_LOGI(TAG, "disconnected");
  Lock();
  if (state == CONNECTED)
  {
    state = DISCONNECTED;
    counter = WIFI_DISC_SCAN_INTERVAL;
    xTimerReset(timer, 0);
  }
  Unlock();
}

void WifiManager::OnScanDone()
{
  ESP_LOGI(TAG, "scan done");
  Lock();
  scanning = false;
  uint16_t count = CONFIG_SCAN_AP_MAX;
  memset(networks, 0, sizeof(networks));
  esp_wifi_scan_get_ap_records(&count, networks);
  for (uint16_t i = count - 1; i > 0 ; i--)
  {
    for (uint16_t j = 0; j < i; j++)
    {
      if (networks[j].rssi < networks[j + 1].rssi)
      {
        wifi_ap_record_t temp;
        memcpy(&temp, &networks[j], sizeof(wifi_ap_record_t));
        memcpy(&networks[j], &networks[j + 1], sizeof(wifi_ap_record_t));
        memcpy(&networks[j + 1], &temp, sizeof(wifi_ap_record_t));
      }
    }
  }
  Unlock();
}

void WifiManager::OnTimeout()
{
  ESP_LOGD(TAG, "timer tick");
  Lock();
  if (state == CONNECTING)
  {
    ESP_LOGD(TAG, "switching on AP");
    esp_wifi_set_mode(WIFI_MODE_APSTA);
    state = AP_MODE;
  }
  else if (state == DISCONNECTED)
  {
    ESP_LOGD(TAG, "connecting");
    esp_wifi_connect();
    state = CONNECTING;
  }
  if (counter == 0)
  {
    if (state == CONNECTED)
    {
      counter = WIFI_CONN_SCAN_INTERVAL;
    }
    else
    {
      counter = WIFI_DISC_SCAN_INTERVAL;
    }
    Scan();
  }
  counter--;
  Unlock();
}

void WifiManager::Scan()
{
  if (!scanning)
  {
    scanning = true;
    wifi_scan_config_t config;
    memset(&config, 0, sizeof(wifi_scan_config_t));
    config.scan_type = WIFI_SCAN_TYPE_PASSIVE;
    config.scan_time.passive = 1000;
    ESP_LOGI(TAG, "starting scan");
    esp_wifi_scan_start(&config, false);
  }
}

void WifiManager::Lock()
{
  char flag = 0;
  xQueueSend(lock, &flag, portMAX_DELAY);
}

void WifiManager::Unlock()
{
  char flag;
  xQueueReceive(lock, &flag, portMAX_DELAY);
}

void WifiManager::OnTimeoutCb(xTimerHandle timer)
{
  WifiManager *self = (WifiManager *)pvTimerGetTimerID(timer);
  self->OnTimeout();
}

esp_err_t WifiManager::EventHandler(void *ctx, system_event_t *event)
{
  WifiManager *self = (WifiManager *)ctx;
  return self->OnEvent(event);
}
