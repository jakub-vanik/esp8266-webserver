#include "server.h"

static const char *TAG = "server";

Server::Server(WifiManager *wifi, SerialPort *serial, int port)
{
  ESP_LOGI(TAG, "constructing");
  this->wifi = wifi;
  this->serial = serial;
  sockaddr_in listen_addr;
  memset(&listen_addr, 0, sizeof(listen_addr));
  listen_addr.sin_family = AF_INET;
  listen_addr.sin_addr.s_addr = INADDR_ANY;
  listen_addr.sin_port = htons(port);
  ESP_LOGD(TAG, "creating socket");
  listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  ESP_LOGD(TAG, "binding");
  bind(listen_fd, (sockaddr* )&listen_addr, sizeof(listen_addr));
  ESP_LOGD(TAG, "listenning");
  listen(listen_fd, 1);
  ESP_LOGD(TAG, "creating task");
  xTaskCreate(TaskEntryPoint, "server_task", 2048, this, 2, &task);
  ESP_LOGD(TAG, "constructed");
}

Server::~Server()
{
  ESP_LOGI(TAG, "destructing");
  ESP_LOGD(TAG, "deleting task");
  vTaskDelete(task);
  ESP_LOGD(TAG, "closing socket");
  close(listen_fd);
  ESP_LOGD(TAG, "destructed");
}

void Server::ServerTask()
{
  ESP_LOGI(TAG, "task entered");
  while (true)
  {
    sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);
    ESP_LOGD(TAG, "waiting for client");
    int client_fd = accept(listen_fd, (sockaddr* )&client_addr, (socklen_t* )&client_addr_len);
    ESP_LOGI(TAG, "client connected");
    if (!wifi->IsConnectionSafe())
    {
      ESP_LOGI(TAG, "connection is not safe, closing connection");
      close(client_fd);
      continue;
    }
    gpio_set_direction(GPIO_NUM_16, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_16, 0);
    os_delay_us(20);
    gpio_set_direction(GPIO_NUM_16, GPIO_MODE_INPUT);
    serial->Flush();
    while (true)
    {
      char buff[128];
      int data_ready = WaitForData(client_fd, 100);
      if (data_ready)
      {
        int count = recv(client_fd, buff, sizeof(buff), 0);
        if (count <= 0)
        {
          ESP_LOGD(TAG, "unable to receive");
          break;
        }
        ESP_LOGD(TAG, "received %d B from client", count);
        serial->Write(buff, count);
        ESP_LOGD(TAG, "sent %d B to serial port", count);
      }
      int out_count = serial->Read(buff, sizeof(buff), false);
      if (out_count > 0)
      {
        ESP_LOGD(TAG, "received %d B from serial port", out_count);
        char *send_pointer = buff;
        int send_remains = out_count;
        while (send_remains > 0)
        {
          int count = send(client_fd, send_pointer, send_remains, 0);
          if (count <= 0)
          {
            ESP_LOGD(TAG, "unable to send");
            break;
          }
          ESP_LOGD(TAG, "sent %d B to client", count);
          send_pointer += count;
          send_remains -= count;
        }
      }
    }
    ESP_LOGI(TAG, "closing connection");
    close(client_fd);
  }
}

int Server::WaitForData(int fd, int timeout)
{
  fd_set readset;
  FD_ZERO(&readset);
  FD_SET(fd, &readset);
  fd_set writeset;
  FD_ZERO(&writeset);
  fd_set errset;
  FD_ZERO(&errset);
  FD_SET(fd, &errset);
  timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = timeout;
  ESP_LOGD(TAG, "waiting for data, timeout %d us", timeout);
  return select(fd + 1, &readset, &writeset, &errset, &tv);
}

void Server::TaskEntryPoint(void *arg)
{
  Server *self = (Server *)arg;
  self->ServerTask();
}
