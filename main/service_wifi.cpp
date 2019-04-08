#include "service_wifi.h"

static const char *TAG = "service_wifi";

ServiceWifi::Builder::Builder(WifiManager *wifi) : Service::Builder(500)
{
  this->wifi = wifi;
}

ServiceWifi::Builder::~Builder()
{
}

Service *ServiceWifi::Builder::CtreateService(const char *method, const char *path)
{
  return new ServiceWifi(wifi, path);
}

ServiceWifi::ServiceWifi(WifiManager *wifi, const char *wifi_param) : Service(500)
{
  this->wifi = wifi;
  command = INVALID;
  ssid = NULL;
  password = NULL;
  text_begin = 0;
  text_end = 0;
  if (strcmp(wifi_param, "join") == 0)
  {
    command = JOIN;
  }
  if (strcmp(wifi_param, "list") == 0)
  {
    command = LIST;
  }
}

ServiceWifi::~ServiceWifi()
{
  if (command == JOIN && ssid && password)
  {
    wifi->JoinNetwork(ssid, password);
  }
}

void ServiceWifi::SetReqData(char *data, int size)
{
  if (command == JOIN)
  {
    ssid = Utils::NextString(data, size, ";");
    password = Utils::NextString(data, size, ";");
    ESP_LOGI(TAG, "setting network %s", ssid);
  }
}

const char *ServiceWifi::GetType()
{
  return "text/plain";
}

int ServiceWifi::GetRespCode()
{
  if (command == JOIN && ssid && password)
  {
    return 200;
  }
  if (command == LIST)
  {
    return 200;
  }
  return 400;
}

int ServiceWifi::GetLength()
{
  if (command == LIST)
  {
    ESP_LOGI(TAG, "listing networks");
    text_end = wifi->ListNetworks(text_data, sizeof(text_data));
    return text_end;
  }
  return 0;
}

int ServiceWifi::GetNextChunk(char *data, int size)
{
  if (command == LIST)
  {
    if (text_end > text_begin)
    {
      int count = size < (text_end - text_begin) ? size : (text_end - text_begin);
      memcpy(data, text_data + text_begin, count);
      text_begin += count;
      return count;
    }
  }
  return 0;
}
