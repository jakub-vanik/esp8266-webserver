#include "service_file.h"

static const char *TAG = "service_file";

ServiceFile::Builder::Builder() : Service::Builder(500)
{
}

ServiceFile::Builder::~Builder()
{
}

Service *ServiceFile::Builder::CtreateService(const char *method, const char *path)
{
  if (!strcmp(method, "GET"))
  {
    return new ServiceFile(path);
  }
  else
  {
    return new Service(405);
  }
}

ServiceFile::ServiceFile(const char *file_name) : Service(500)
{
  if (strlen(file_name) == 0)
  {
    file_name = "index.html";
  }
  ESP_LOGI(TAG, "openning file %s", file_name);
  unsigned intr = XTOS_DISABLE_EXCM_INTERRUPTS;
  fd = spiffs_open(file_name, O_RDWR, 0);
  if (fd > 0)
  {
    const char *extension = file_name + strlen(file_name);
    if (Utils::EndsWith(extension, ".txt"))
    {
      type = "text/plain";
    }
    else if (Utils::EndsWith(extension, ".html"))
    {
      type = "text/html";
    }
    else if (Utils::EndsWith(extension, ".css"))
    {
      type = "text/css";
    }
    else if (Utils::EndsWith(extension, ".js"))
    {
      type = "text/javascript";
    }
    else if (Utils::EndsWith(extension, ".gif"))
    {
      type = "image/gif";
    }
    else if (Utils::EndsWith(extension, ".png"))
    {
      type = "image/png";
    }
    else if (Utils::EndsWith(extension, ".jpg"))
    {
      type = "image/jpeg";
    }
    else
    {
      type = "application/octet-stream";
    }
    struct stat st;
    spiffs_fstat(fd, &st);
    length = st.st_size;
    ESP_LOGI(TAG, "file size is %d", length);
  }
  else
  {
    type = "application/octet-stream";
    length = 0;
    ESP_LOGW(TAG, "file was not found");
  }
  XTOS_RESTORE_INTLEVEL(intr);
}

ServiceFile::~ServiceFile()
{
  if (fd > 0)
  {
    ESP_LOGD(TAG, "closing file");
    unsigned intr = XTOS_DISABLE_EXCM_INTERRUPTS;
    spiffs_close(fd);
    XTOS_RESTORE_INTLEVEL(intr);
  }
}

void ServiceFile::SetReqData(char *data, int size)
{
}

const char *ServiceFile::GetType()
{
  return type;
}

int ServiceFile::GetRespCode()
{
  if (fd > 0)
  {
    return 200;
  }
  return 404;
}

int ServiceFile::GetLength()
{
  return length;
}

int ServiceFile::GetNextChunk(char *data, int size)
{
  if (fd > 0)
  {
    ESP_LOGD(TAG, "reading file");
    unsigned intr = XTOS_DISABLE_EXCM_INTERRUPTS;
    int ret = spiffs_read(fd, data, size);
    XTOS_RESTORE_INTLEVEL(intr);
    if (ret > 0)
    {
      return ret;
    }
  }
  return 0;
}
