#include "timeCalc.h"

uint16_t minutesSinceMidnight()
{
    t = rtc.getTime();
    return t.hour * 60 + t.min;
}