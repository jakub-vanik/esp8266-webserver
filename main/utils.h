#ifndef __UTILS_H__
#define __UTILS_H__

#include "user_config.h"
#include "stdarg.h"
#include "stdio.h"
#include "string.h"

class Utils
{
public:
  static bool EndsWith(const char *str, const char *sub);
  static const char *StartsWith(const char *str, const char *sub);
  static const char *NextString(char *&data, int &size, const char *delim);
  static int Print(char *&data, int &size, const char *format, ...);
};

#endif
