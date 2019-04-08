#ifndef __SERVICE_FILE_H__
#define __SERVICE_FILE_H__

#include "user_config.h"
#include "fcntl.h"
#include "unistd.h"
#include "esp_log.h"
#include "xtensa/xtruntime.h"
#include "service.h"
#include "service_factory.h"
#include "utils.h"

#define spiffs_open(...) _spiffs_open_r(NULL, __VA_ARGS__);
#define spiffs_read(...) _spiffs_read_r(NULL, __VA_ARGS__);
#define spiffs_write(...) _spiffs_write_r(NULL, __VA_ARGS__);
#define spiffs_lseek(...) _spiffs_lseek_r(NULL, __VA_ARGS__);
#define spiffs_close(...) _spiffs_close_r(NULL, __VA_ARGS__);
#define spiffs_rename(...) _spiffs_rename_r(NULL, __VA_ARGS__);
#define spiffs_unlink(...) _spiffs_unlink_r(NULL, __VA_ARGS__);
#define spiffs_fstat(...) _spiffs_fstat_r(NULL, __VA_ARGS__);

extern "C"
{
  int _spiffs_open_r(struct _reent *r, const char *filename, int flags, int mode);
  _ssize_t _spiffs_read_r(struct _reent *r, int fd, void *buf, size_t len);
  _ssize_t _spiffs_write_r(struct _reent *r, int fd, void *buf, size_t len);
  _off_t _spiffs_lseek_r(struct _reent *r, int fd, _off_t where, int whence);
  int _spiffs_close_r(struct _reent *r, int fd);
  int _spiffs_rename_r(struct _reent *r, const char *from, const char *to);
  int _spiffs_unlink_r(struct _reent *r, const char *filename);
  int _spiffs_fstat_r(struct _reent *r, int fd, struct stat *s);
}

class ServiceFile : public Service
{
public:
  class Builder : public Service::Builder
  {
  public:
    Builder();
    ~Builder();
    Service *CtreateService(const char *method, const char *path);
  };
  ServiceFile(const char *file_name);
  ~ServiceFile();
  void SetReqData(char *data, int size);
  int GetRespCode();
  const char *GetType();
  int GetLength();
  int GetNextChunk(char *data, int size);
  static Service *CreateFileService(const char *method, const char *path);
private:
  int fd;
  const char *type;
  int length;
};

#endif
