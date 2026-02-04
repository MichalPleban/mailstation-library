
#include <time.h>

#include "mailstation.h"

void ms_get_time(struct tm *timeinfo)
{
    // Set RTC page 0 (read time)
    
    ms_port_rtc_control = ms_port_rtc_control & 0xC0;
    timeinfo->tm_sec = ms_port_seconds_low + 10*ms_port_seconds_high;
    timeinfo->tm_min = ms_port_minutes_low + 10*ms_port_minutes_high;
    timeinfo->tm_hour = ms_port_hours_low + 10*ms_port_hours_high;
    timeinfo->tm_wday = ms_port_weekday;
    timeinfo->tm_mday = ms_port_day_low + 10*ms_port_day_high;
    timeinfo->tm_mon = ms_port_month_low + 10*ms_port_month_high;
    timeinfo->tm_year = ms_port_year_low + 10*ms_port_year_high + 2000 - 1900;
    timeinfo->tm_yday = (timeinfo->tm_year % 4 == 0) ? 366 : 365;
    timeinfo->tm_isdst = 0;
    timeinfo->tm_hundredth = 0;
}
