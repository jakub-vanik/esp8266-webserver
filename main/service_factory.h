#ifndef __SERVICE_FACTORY_H__
#define __SERVICE_FACTORY_H__

#include "user_config.h"
#include "stddef.h"
#include "esp_log.h"
#include "service.h"
#include "utils.h"

class ServiceFactory
{
public:
  ServiceFactory();
  ~ServiceFactory();
  void RegisterService(int priority, const char *path, Service::Builder *builder);
  Service *CreateService(const char *method, const char *url);
private:
  class ServiceRecord
  {
  public:
    ServiceRecord *next;
    int priority;
    const char *path;
    Service::Builder *builder;
  };
  ServiceRecord* service_list;
};

#endif
