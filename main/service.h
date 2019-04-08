#ifndef __SERVICE_H__
#define __SERVICE_H__

#include "user_config.h"

class Service
{
public:
  class Builder
  {
  public:
    Builder(int code);
    virtual ~Builder();
    virtual Service *CtreateService(const char *method, const char *path);
  private:
    int code;
  };
  Service(int code);
  virtual ~Service();
  virtual void SetReqData(char *data, int size);
  virtual int GetRespCode();
  virtual const char *GetType();
  virtual int GetLength();
  virtual int GetNextChunk(char *data, int size);
private:
  int code;
};

#endif
