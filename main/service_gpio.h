#ifndef __SERVICE_GPIO_H__
#define __SERVICE_GPIO_H__

#include "user_config.h"
#include "esp_log.h"
#include "stdlib.h"
#include "driver/gpio.h"
#include "service.h"
#include "utils.h"

class ServiceGpio : public Service
{
public:
  class Builder : public Service::Builder
  {
  public:
    Builder();
    ~Builder();
    Service *CtreateService(const char *method, const char *path);
  };
  ServiceGpio(const char *gpio_param);
  ~ServiceGpio();
  void SetReqData(char *data, int size);
  const char *GetType();
  int GetRespCode();
  int GetLength();
  int GetNextChunk(char *data, int size);
private:
  enum access_type
  {
    INVALID,
    VALUE,
    DIRECTION
  };
  int pin_num;
  access_type access;
};

#endif
