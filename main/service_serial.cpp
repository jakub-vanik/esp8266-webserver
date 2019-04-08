#include "service_serial.h"

static const char *TAG = "service_serial";

ServiceSerial::Builder::Builder(SerialPort *serial) : Service::Builder(500)
{
  this->serial = serial;
}

ServiceSerial::Builder::~Builder()
{
}

Service *ServiceSerial::Builder::CtreateService(const char *method, const char *path)
{
  return new ServiceSerial(serial, path);
}

ServiceSerial::ServiceSerial(SerialPort *serial, const char *serial_param) : Service(500)
{
  this->serial = serial;
  if (strcmp(serial_param, "read") == 0)
  {
    access = READ;
  }
  else if (strcmp(serial_param, "write") == 0)
  {
    access = WRITE;
  }
  else
  {
    char *rest;
    expected_bytes = strtol(serial_param, &rest, 10);
    serial_param = Utils::StartsWith(rest, "/");
    if (serial_param)
    {
      read_timeout = strtol(serial_param, &rest, 10) / (portTICK_RATE_MS * 10) + 1;
      if (strlen(rest) == 0)
      {
        access = EXCHANGE;
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
  ESP_LOGD(TAG, "locking");
  serial->Lock();
}

ServiceSerial::~ServiceSerial()
{
  ESP_LOGD(TAG, "unlocking");
  serial->Unlock();
}

void ServiceSerial::SetReqData(char *data, int size)
{
  if (access == WRITE)
  {
    ESP_LOGD(TAG, "writing %d B", size);
    serial->Write(data, size);
  }
  else if (access == EXCHANGE)
  {
    ESP_LOGD(TAG, "flushing input");
    serial->Flush();
    ESP_LOGD(TAG, "writing %d B", size);
    serial->Write(data, size);
  }
}

const char *ServiceSerial::GetType()
{
  return "application/octet-stream";
}

int ServiceSerial::GetRespCode()
{
  if (access == READ || access == WRITE || access == EXCHANGE)
  {
    return 200;
  }
  return 400;
}

int ServiceSerial::GetLength()
{
  if (access == READ)
  {
    available_bytes = serial->Count();
    ESP_LOGD(TAG, "found %d B in input buffer", available_bytes);
    return available_bytes;
  }
  else if (access == EXCHANGE)
  {
    ESP_LOGD(TAG, "waiting for %d B in input buffer", expected_bytes);
    available_bytes = 0;
    portTickType xLastWakeTime = xTaskGetTickCount();
    for (int i = 0; i < read_timeout; i++)
    {
      vTaskDelayUntil(&xLastWakeTime, 10);
      available_bytes = serial->Count();
      if (available_bytes >= expected_bytes)
      {
        break;
      }
    }
    if (available_bytes > expected_bytes)
    {
      available_bytes = expected_bytes;
    }
    ESP_LOGD(TAG, "found %d B in input buffer", available_bytes);
    return available_bytes;
  }
  return 0;
}

int ServiceSerial::GetNextChunk(char *data, int size)
{
  if (access == READ || access == EXCHANGE)
  {
    if (available_bytes > 0)
    {
      if (size > available_bytes)
      {
        size = available_bytes;
      }
      ESP_LOGD(TAG, "reading %d B", size);
      int read = serial->Read(data, size, false);
      available_bytes -= read;
      return read;
    }
  }
  return 0;
}
