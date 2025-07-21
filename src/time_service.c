#include "time_service.h"
#include <stdio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(time_service);

static const struct device * time_rtc;

static void time_service_callback(const struct device *dev, uint16_t id, void *user_data)
{

}

int time_service_init(const struct device * rtc)
{
    int ret;

	time_rtc = rtc;

	struct rtc_time init_tm = {
		.tm_year = 2025 - 1900,
		.tm_mon = 4 - 1,
		.tm_mday = 18,
		.tm_hour = 14,
		.tm_min = 57,
		.tm_sec = 0,
	};

    // Set RTC Alarm
	struct rtc_time tm = {
		.tm_year = 0,
		.tm_mon = 0,
		.tm_mday = 0,
		.tm_hour = 0,
		.tm_min = 0,
		.tm_sec = 0,
	};

    if (!device_is_ready(time_rtc)) {
		LOG_INF("RTC is not ready\n");
		return 0;
	}

    ret = rtc_set_time(time_rtc, &init_tm);
	if (ret < 0) {
		LOG_INF("Init - Cannot write date time: %d\n", ret);
		return 0;
	}

    ret = rtc_alarm_set_time(time_rtc, 0, RTC_ALARM_TIME_MASK_SECOND, &tm);
    if (ret < 0) {
		LOG_INF("Cannot set alarm: %d\n", ret);
		return 0;
	}

    ret = rtc_alarm_set_callback(time_rtc, 0, time_service_callback, NULL);
    if (ret < 0) {
		LOG_INF("Cannot set alarm callback: %d\n", ret);
		return 0;
	}

    return 1;
}

int time_service_set_date_time( struct rtc_time *tm)
{
	int ret = 0;

	ret = rtc_set_time(time_rtc, tm);
	if (ret < 0) {
		LOG_INF("Cannot write date time: %d\n", ret);
		return ret;
	}
	return ret;
}

int time_service_get_date_time( struct rtc_time *tm)
{
	int ret = 0;

	ret = rtc_get_time(time_rtc, tm);
	if (ret < 0) {
		LOG_INF("Cannot read date time: %d\n", ret);
		return ret;
	}

	LOG_INF("%04d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900,
	       tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

	return ret;
}