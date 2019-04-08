#ifndef __SERIAL_PORT_H__
#define __SERIAL_PORT_H__

#include "user_config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "driver/uart.h"

class SerialPort
{
public:
  SerialPort(uart_port_t uart);
  ~SerialPort();
  void Flush();
  int Count();
  int Read(char *data, int size, bool block);
  void Write(const char *data, int size);
  void Lock();
  void Unlock();
private:
  uart_port_t uart;
  xQueueHandle lock;
};

#endif
