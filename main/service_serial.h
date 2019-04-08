#ifndef __SERVICE_SERIAL_H__
#define __SERVICE_SERIAL_H__

#include "user_config.h"
#include "esp_log.h"
#include "stdlib.h"
#include "string.h"
#include "service.h"
#include "serial_port.h"
#include "utils.h"

class ServiceSerial : public Service
{
public:
  class Builder : public Service::Builder
  {
  public:
    Builder(SerialPort *serial);
    ~Builder();
    Service *CtreateService(const char *method, const char *path);
  private:
    SerialPort *serial;
  };
  ServiceSerial(SerialPort *serial, const char *serial_param);
  ~ServiceSerial();
  void SetReqData(char *data, int size);
  const char *GetType();
  int GetRespCode();
  int GetLength();
  int GetNextChunk(char *data, int size);
private:
  enum access_type
  {
    INVALID,
    READ,
    WRITE,
    EXCHANGE
  };
  SerialPort *serial;
  access_type access;
  int expected_bytes;
  int read_timeout;
  int available_bytes;
};

#endif
