#ifndef TIME_SERVICE_H
#define TIME_SERVICE_H

#include <zephyr/device.h>
#include <zephyr/drivers/rtc.h>

int time_service_init(const struct device * rtc);
int time_service_set_date_time(const struct device *rtc, struct rtc_time *tm);
int time_service_get_date_time(const struct device *rtc, struct rtc_time *tm);

#endif