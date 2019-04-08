#ifndef __SERVICE_WIFI_H__
#define __SERVICE_WIFI_H__

#include "user_config.h"
#include "esp_log.h"
#include "service.h"
#include "wifi_manager.h"
#include "utils.h"

class ServiceWifi : public Service
{
public:
  class Builder : public Service::Builder
  {
  public:
    Builder(WifiManager *wifi);
    ~Builder();
    Service *CtreateService(const char *method, const char *path);
  private:
    WifiManager *wifi;
  };
  ServiceWifi(WifiManager *wifi, const char *wifi_param);
  ~ServiceWifi();
  void SetReqData(char *data, int size);
  const char *GetType();
  int GetRespCode();
  int GetLength();
  int GetNextChunk(char *data, int size);
private:
  enum command_type
  {
    INVALID,
    JOIN,
    LIST
  };
  WifiManager *wifi;
  command_type command;
  const char *ssid;
  const char *password;
  char text_data[512];
  int text_begin;
  int text_end;
};

#endif
