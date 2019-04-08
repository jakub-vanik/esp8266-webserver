#include "service.h"

Service::Builder::Builder(int code)
{
  this->code = code;
}

Service::Builder::~Builder()
{
}

Service *Service::Builder::CtreateService(const char *method, const char *path)
{
  return new Service(code);
}

Service::Service(int code)
{
  this->code = code;
}

Service::~Service()
{
}

void Service::SetReqData(char *data, int size)
{
}

const char *Service::GetType()
{
  return "text/plain";
}

int Service::GetRespCode()
{
  return code;
}

int Service::GetLength()
{
  return 0;
}

int Service::GetNextChunk(char *data, int size)
{
  return 0;
}
