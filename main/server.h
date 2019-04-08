#ifndef __SERVER_H__
#define __SERVER_H__

#include "user_config.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "sys/socket.h"
#include "driver/gpio.h"
#include "wifi_manager.h"
#include "serial_port.h"

class Server
{
public:
  Server(WifiManager *wifi, SerialPort *serial, int port);
  ~Server();
private:
  WifiManager *wifi;
  SerialPort *serial;
  int listen_fd;
  xTaskHandle task;
  void ServerTask();
  int WaitForData(int fd, int timeout);
  static void TaskEntryPoint(void *arg);
};

#endif
