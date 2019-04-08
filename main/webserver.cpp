#include "webserver.h"

static const char *TAG = "webserver";

Webserver::Webserver(ServiceFactory *factory, int port)
{
  ESP_LOGI(TAG, "constructing");
  this->factory = factory;
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
  listen(listen_fd, 8);
  next_beffer = buffers;
  lock = xQueueCreate(1, 1);
  ESP_LOGD(TAG, "creating tasks");
  for (int i = 0; i < WEB_CONNECTIONS; i++)
  {
    xTaskCreate(TaskEntryPoint, "server_task", 2048, this, 2, &tasks[i]);
  }
  ESP_LOGD(TAG, "constructed");
}

Webserver::~Webserver()
{
  ESP_LOGI(TAG, "destructing");
  ESP_LOGD(TAG, "deleting tasks");
  for (int i = 0; i < WEB_CONNECTIONS; i++)
  {
    vTaskDelete(tasks[i]);
  }
  vQueueDelete(lock);
  ESP_LOGD(TAG, "closing socket");
  close(listen_fd);
  ESP_LOGD(TAG, "destructed");
}

void Webserver::ServerTask()
{
  ESP_LOGI(TAG, "task entered");
  char *req_data = GetMemory(WEB_REQ_BUFF_SIZE);
  char *resp_data = GetMemory(WEB_RESP_BUFF_SIZE);
  while (true)
  {
    sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);
    LockMutex();
    ESP_LOGD(TAG, "waiting for client");
    int client_fd = accept(listen_fd, (sockaddr* )&client_addr, (socklen_t* )&client_addr_len);
    ESP_LOGI(TAG, "client connected");
    UnlockMutex();
    char *req_content_data;
    int req_content_size;
    int req_size = ReceiveRequest(client_fd, req_data, WEB_REQ_BUFF_SIZE, req_content_data, req_content_size);
    while (req_size > 0)
    {
      ESP_LOGI(TAG, "received %d B request", req_size);
      const char *method = NULL;
      const char *url = NULL;
      ParseRequest(req_data, req_size, method, url);
      if (method != NULL && url != NULL)
      {
        Service *service = CreateService(method, url);
        service->SetReqData(req_content_data, req_content_size);
        int code = service->GetRespCode();
        if (code >= 400)
        {
          delete service;
          service = CreateErrorService(code);
        }
        const char *type = service->GetType();
        int length = service->GetLength();
        int resp_size = CreateHeader(resp_data, WEB_RESP_BUFF_SIZE, code, type, length);
        ESP_LOGD(TAG, "generated %d B header", resp_size);
        resp_size += service->GetNextChunk(resp_data + resp_size, WEB_RESP_BUFF_SIZE - resp_size);
        while (resp_size > 0)
        {
          ESP_LOGI(TAG, "sending %d B response", resp_size);
          char *send_pointer = resp_data;
          int send_remains = resp_size;
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
          resp_size = service->GetNextChunk(resp_data, WEB_RESP_BUFF_SIZE);
        }
        delete service;
      }
      req_size = ReceiveRequest(client_fd, req_data, WEB_REQ_BUFF_SIZE, req_content_data, req_content_size);
    }
    ESP_LOGI(TAG, "closing connection");
    close(client_fd);
  }
}

char *Webserver::GetMemory(int size)
{
  char *buffer = next_beffer;
  next_beffer += size;
  return buffer;
}

void Webserver::LockMutex()
{
  char flag = 0;
  xQueueSend(lock, &flag, portMAX_DELAY);
}

void Webserver::UnlockMutex()
{
  char flag;
  xQueueReceive(lock, &flag, portMAX_DELAY);
}

int Webserver::ReceiveRequest(int fd, char *data, int size, char *&content_data, int &content_size)
{
  content_data = NULL;
  content_size = 0;
  int position = 0;
  int last_line_start = 0;
  while (true)
  {
    if (size - position <= 0)
    {
      ESP_LOGW(TAG, "request buffer is full");
      return 0;
    }
    int count = recv(fd, data + position, size - position, 0);
    if (count <= 0)
    {
      ESP_LOGD(TAG, "unable to receive");
      return 0;
    }
    if (!content_data)
    {
      ESP_LOGD(TAG, "received %d B of headers", count);
      for (int i = 0; i < count; i++)
      {
        position++;
        if (Utils::EndsWith(&data[position], "\r\n"))
        {
          if (position - last_line_start == 2)
          {
            ESP_LOGD(TAG, "found empty line");
            content_data = data + position;
          }
          else
          {
            ESP_LOGD(TAG, "processing %d B line", position - last_line_start);
            const char *value = Utils::StartsWith(&data[last_line_start], "Content-Length: ");
            if (value)
            {
              content_size = strtol(value, NULL, 10);
              ESP_LOGD(TAG, "content size is %d B", content_size);
            }
          }
          last_line_start = position;
        }
      }
    }
    else
    {
      ESP_LOGD(TAG, "received %d B of content", count);
      position += count;
    }
    if (content_data)
    {
      if (data + position >= content_data + content_size)
      {
        ESP_LOGD(TAG, "request received");
        return position;
      }
    }
  }
}

void Webserver::ParseRequest(char *data, int size, const char *&method, const char *&url)
{
  method = Utils::NextString(data, size, " ");
  url = Utils::NextString(data, size, " ");
  ESP_LOGI(TAG, "request is %s %s", method, url);
}

Service *Webserver::CreateService(const char *method, const char *url)
{
  return factory->CreateService(method, url);
}

Service *Webserver::CreateErrorService(int code)
{
  char url[16];
  snprintf(url, sizeof(url), "/error%d.html", code);
  return CreateService("GET", url);
}

int Webserver::CreateHeader(char *data, int size, int code, const char *type, int length)
{
  const char *status;
  switch (code)
  {
    case 200:
      status = "OK";
      break;
    case 400:
      status = "Bad Request";
      break;
    case 403:
      status = "Forbidden";
      break;
    case 404:
      status = "Not Found";
      break;
    case 405:
      status = "Method Not Allowed";
      break;
    case 500:
      status = "Internal Server Error";
      break;
    default:
      status = "";
  }
  int count = 0;
  count += Utils::Print(data, size, "HTTP/1.1 %d %s\r\n", code, status);
  count += Utils::Print(data, size, "Content-Type: %s\r\n", type);
  count += Utils::Print(data, size, "Content-Length: %d\r\n", length);
  count += Utils::Print(data, size, "Connection: close\r\n\r\n");
  return count;
}

void Webserver::TaskEntryPoint(void *arg)
{
  Webserver *self = (Webserver *)arg;
  self->ServerTask();
}
