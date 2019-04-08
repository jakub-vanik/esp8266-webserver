#include "service_factory.h"

static const char *TAG = "service_factory";

ServiceFactory::ServiceFactory()
{
  ESP_LOGI(TAG, "constructing");
  service_list = NULL;
  RegisterService(0, "", new Service::Builder(400));
  RegisterService(1, "/", new Service::Builder(404));
  ESP_LOGD(TAG, "constructed");
}

ServiceFactory::~ServiceFactory()
{
  ESP_LOGI(TAG, "destructing");
  ServiceRecord *record = service_list;
  while (record)
  {
    ServiceRecord *prev_record = record;
    record = record->next;
    delete prev_record->builder;
    delete prev_record;
  }
  ESP_LOGD(TAG, "destructed");
}

void ServiceFactory::RegisterService(int priority, const char *path, Service::Builder *builder)
{
  ESP_LOGI(TAG, "adding service with priority %d", priority);
  ServiceRecord **record_ptr = &service_list;
  ServiceRecord *record = *record_ptr;
  while (record && record->priority > priority)
  {
    record_ptr = &record->next;
    record = *record_ptr;
  }
  ServiceRecord *new_record = new ServiceRecord();
  new_record->next = record;
  new_record->priority = priority;
  new_record->path = path;
  new_record->builder = builder;
  *record_ptr = new_record;
}

Service *ServiceFactory::CreateService(const char *method, const char *url)
{
  ServiceRecord *record = service_list;
  while (record)
  {
    const char *path = Utils::StartsWith(url, record->path);
    if (path)
    {
      ESP_LOGI(TAG, "using service with priority %d", record->priority);
      return record->builder->CtreateService(method, path);
    }
    record = record->next;
  }
  ESP_LOGE(TAG, "no suitable service found");
  return new Service(500);
}
