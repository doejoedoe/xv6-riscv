#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int months[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

int 
is_leap(int year) 
{
  return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int
get_year(int* days)
{
  int days_remain = *days;
  int year = 1970;
  if (days_remain >= 0) {
    while (days_remain >= (is_leap(year) ? 366 : 365)) {
      days_remain -= is_leap(year) ? 366 : 365;
      year++;
    }
  } else {
    while (days_remain < 0) {
      year--;
      days_remain += is_leap(year) ? 366 : 365;
    }
  }
  *days = days_remain;
  return year;
}

int
get_month(int* days, int year) {
  int days_remain = *days;
  int month = 0;
  
  months[1] = is_leap(year) ? 29 : 28;
  while(days_remain >= months[month]) {
    days_remain -= months[month];
    month++;
  }

  *days = days_remain;
  return ++month;
}

void
print_date(uint64 nsec) 
{
  long sec = (long)nsec / (long)1e9;
  long snsec_rem = (long)nsec % (long)1e9;
  if(snsec_rem < 0) {
    sec--;
    snsec_rem += (long)1e9;
  };
  long msec = snsec_rem / (long)1e6;

  int sec_in_minute = 60;
  int sec_in_hour = sec_in_minute * 60;
  int sec_in_day = sec_in_hour * 24;
  
  int day = sec / sec_in_day;
  sec %= sec_in_day;
  if(sec < 0) {
    day--;
    sec += sec_in_day;
  };

  int year = get_year(&day);
  int month = get_month(&day, year);
  day++;

  int hour = sec / sec_in_hour;
  sec %= sec_in_hour;

  int minute = sec / sec_in_minute;
  sec %= sec_in_minute;

  printf("%d.%d.%d %d:%d:%ld.%ld\n", 
    day, month, year, hour, minute, sec, msec);
}

int
main()
{
  uint64 nsec;
  if(rtctime(&nsec) < 0) {
    fprintf(2, "unable to get rtc time");
    exit(1);
  }
  print_date(nsec);
  exit(0);
}