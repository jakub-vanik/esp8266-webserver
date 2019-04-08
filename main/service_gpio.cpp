#include "service_gpio.h"

static const char *TAG = "service_gpio";

ServiceGpio::Builder::Builder() : Service::Builder(500)
{
}

ServiceGpio::Builder::~Builder()
{
}

Service *ServiceGpio::Builder::CtreateService(const char *method, const char *path)
{
  return new ServiceGpio(path);
}

ServiceGpio::ServiceGpio(const char *gpio_param) : Service(500)
{
  char *rest;
  pin_num = strtol(gpio_param, &rest, 10);
  if (pin_num < 16)
  {
    gpio_param = Utils::StartsWith(rest, "/");
    if (gpio_param)
    {
      if (strcmp(gpio_param, "val") == 0)
      {
        access = VALUE;
      }
      else if (strcmp(gpio_param, "dir") == 0)
      {
        access = DIRECTION;
      }
      else
      {
        access = INVALID;
      }
    }
    else
    {
      access = INVALID;
    }
  }
  else
  {
    access = INVALID;
  }
}

ServiceGpio::~ServiceGpio()
{
}

void ServiceGpio::SetReqData(char *data, int size)
{
  if (size >= 1)
  {
    int bit = data[0] & 1;
    if (access == VALUE)
    {
      ESP_LOGI(TAG, "setting pin %d level to %d", pin_num, bit);
      gpio_set_level((gpio_num_t) pin_num, bit);
    }
    else if (access == DIRECTION)
    {
      ESP_LOGI(TAG, "setting pin %d direction to %d", pin_num, bit);
      gpio_set_direction((gpio_num_t) pin_num, bit ? GPIO_MODE_OUTPUT : GPIO_MODE_INPUT);
    }
  }
}

const char *ServiceGpio::GetType()
{
  return "application/octet-stream";
}

int ServiceGpio::GetRespCode()
{
  if (access == VALUE || access == DIRECTION)
  {
    return 200;
  }
  return 400;
}

int ServiceGpio::GetLength()
{
  if (access == VALUE)
  {
    return 1;
  }
  return 0;
}

int ServiceGpio::GetNextChunk(char *data, int size)
{
  if (access == VALUE)
  {
    if (size >= 1)
    {
      int bit = gpio_get_level((gpio_num_t) pin_num);
      ESP_LOGI(TAG, "read pin %d level as %d", pin_num, bit);
      data[0] = bit;
      access = INVALID;
      return 1;
    }
  }
  return 0;
}
