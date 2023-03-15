#pragma once

#include <Arduino.h>
#include <DS3231.h>

DS3231 rtc;
Time t;

uint16_t minutesSinceMidnight();