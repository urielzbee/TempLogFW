#ifndef TIME_SERVICE_H
#define TIME_SERVICE_H

#include <zephyr/device.h>
#include <zephyr/drivers/rtc.h>

int time_service_init(const struct device * rtc);
int time_service_set_date_time( struct rtc_time *tm);
int time_service_get_date_time( struct rtc_time *tm);
int time_service_alarm_set_time( uint16_t id, uint16_t mask, const struct rtc_time * timeptr);
int time_service_alarm_set_callback( uint16_t id, rtc_alarm_callback cb, void *user_data);

#endif