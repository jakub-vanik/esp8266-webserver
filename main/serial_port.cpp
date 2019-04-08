#include "serial_port.h"

static const char *TAG = "serial_port";

SerialPort::SerialPort(uart_port_t uart)
{
  ESP_LOGI(TAG, "constructing");
  this->uart = uart;
  lock = xQueueCreate(1, 1);
  uart_config_t config;
  config.baud_rate = UART_BAUD_RATE;
  config.data_bits = UART_DATA_8_BITS;
  config.parity = UART_PARITY_DISABLE;
  config.stop_bits = UART_STOP_BITS_1;
  config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
  config.rx_flow_ctrl_thresh = 0;
  uart_param_config(uart, &config);
  ESP_LOGD(TAG, "installing driver");
  uart_driver_install(uart, UART_QUEUE_SIZE, UART_QUEUE_SIZE, 0, NULL);
  ESP_LOGD(TAG, "constructed");
}

SerialPort::~SerialPort()
{
  ESP_LOGI(TAG, "destructiong");
  ESP_LOGD(TAG, "deleting driver");
  uart_driver_delete(uart);
  vQueueDelete(lock);
  ESP_LOGD(TAG, "destructed");
}

void SerialPort::Flush()
{
  uart_flush_input(uart);
}

int SerialPort::Count()
{
  size_t size;
  uart_get_buffered_data_len(uart, &size);
  return size;
}

int SerialPort::Read(char *data, int size, bool block)
{
  TickType_t timeout = block ? portMAX_DELAY : 0;
  return uart_read_bytes(uart, (uint8_t *)data, size, timeout);
}

void SerialPort::Write(const char *data, int size)
{
  uart_write_bytes(uart, data, size);
}

void SerialPort::Lock()
{
  char flag = 0;
  xQueueSend(lock, &flag, portMAX_DELAY);
}

void SerialPort::Unlock()
{
  char flag;
  xQueueReceive(lock, &flag, portMAX_DELAY);
}
