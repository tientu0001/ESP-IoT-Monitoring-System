
#ifndef FUN_TIME_H
#define FUN_TIME_H
//************************************************************************************************
#include "config.h"
// #include <stdint.h>
//************************************************************************************************
class FunTime
{
public:
  struct tm sysTime;
  void setSysTime(int year, int month, int day, int hour, int minute, int sec);
  void getSysTime(void);

  float get_numberof_day(struct tm a, struct tm b);

  uint8_t u8_hourCur;

private:

};

//************************************************************************************************
extern FunTime ti;

//************************************************************************************************
#endif // FUN_TIME_H