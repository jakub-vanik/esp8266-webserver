#include "utils.h"

bool Utils::EndsWith(const char *str, const char *sub)
{
  int len = strlen(sub);
  if (memcmp(str - len, sub, len) == 0)
  {
    return true;
  }
  return false;
}

const char *Utils::StartsWith(const char *str, const char *sub)
{
  int len = strlen(sub);
  if (memcmp(str, sub, len) == 0)
  {
    return str + len;
  }
  return NULL;
}

const char *Utils::NextString(char *&data, int &size, const char *delim)
{
  int len = strlen(delim);
  char *start = data;
  while (size >= len)
  {
    if (memcmp(data, delim, len) == 0)
    {
      data[0] = 0;
      data += len;
      size -= len;
      return start;
    }
    data++;
    size--;
  }
  return NULL;
}

int Utils::Print(char *&data, int &size, const char *format, ...)
{
  va_list args;
  va_start(args, format);
  int length = vsnprintf(data, size, format, args);
  if (length > 0 && length < size)
  {
    data += length;
    size -= length;
  }
  else
  {
    data += size;
    size = 0;
  }
  va_end(args);
  return length;
}
