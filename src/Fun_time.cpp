//************************************************************************************************

// #include <Arduino.h>
#include "config.h"
#include "Fun_time.h"

#include <sys/time.h>
#include <../include/time.h> // See issue #6714
FunTime ti;

// time_sec = (uint32_t) (millis() / 1000) % 60 ;
// time_min = (uint32_t) ((millis() / (1000*60)) % 60);
// time_hour = (uint32_t) ((millis() / (1000*60*60)) % 24);

//************************************************************************************************
void FunTime::setSysTime(int year, int month, int day, int hour, int minute, int sec)
{
  struct tm t_tm;

  t_tm.tm_year = year - 1900;
  t_tm.tm_mon = month - 1;
  t_tm.tm_mday = day;
  t_tm.tm_hour = hour;
  t_tm.tm_min = minute;
  t_tm.tm_sec = sec;

  time_t t = mktime(&t_tm);
  struct timeval now = {.tv_sec = t};
  settimeofday(&now, NULL);
}
//************************************************************************************************
void FunTime::getSysTime(void)
{
  struct timeval tv;

  gettimeofday(&tv, NULL); // get Epoch(Unix, POSIX) time

  time_t nowtime = tv.tv_sec;

  struct tm *now;
  now = localtime(&nowtime);
  this->sysTime = *now;

  this->sysTime.tm_year += 1900;
  this->sysTime.tm_mon += 1;
}
//************************************************************************************************
float FunTime::get_numberof_day(struct tm a, struct tm b)
{
  time_t x = mktime(&a);
  time_t y = mktime(&b);

  double difference = 0.0f;
  if (x != (time_t)(-1) && y != (time_t)(-1))
  {
    difference = abs(difftime(y, x) / (60 * 60 * 24));
  }

  return difference;
}
//************************************************************************************************
//************************************************************************************************
//************************************************************************************************
//************************************************************************************************
//************************************************************************************************
//************************************************************************************************