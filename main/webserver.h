#ifndef __WEBSERVER_H__
#define __WEBSERVER_H__

#include "user_config.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "sys/socket.h"
#include "service.h"
#include "service_factory.h"
#include "utils.h"

class Webserver
{
public:
  Webserver(ServiceFactory *factory, int port);
  ~Webserver();
private:
 ServiceFactory *factory;
  int listen_fd;
  char buffers[(WEB_REQ_BUFF_SIZE + WEB_RESP_BUFF_SIZE) * WEB_CONNECTIONS];
  char *next_beffer;
  xQueueHandle lock;
  xTaskHandle tasks[WEB_CONNECTIONS];
  void ServerTask();
  char *GetMemory(int size);
  void LockMutex();
  void UnlockMutex();
  int ReceiveRequest(int fd, char *data, int size, char *&content_data, int &content_size);
  void ParseRequest(char *data, int size, const char *&method, const char *&url);
  Service *CreateService(const char *method, const char *url);
  Service *CreateErrorService(int code);
  int CreateHeader(char *data, int size, int code, const char *type, int length);
  static void TaskEntryPoint(void *arg);
};

#endif
