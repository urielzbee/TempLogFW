/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#include <zephyr/drivers/sensor.h>

#include <zephyr/device.h>
#include <zephyr/drivers/rtc.h>
#include <zephyr/sys/util.h>

#include <zephyr/drivers/flash.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define BTN1_NODE DT_ALIAS(btn1)

/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec btn1 = GPIO_DT_SPEC_GET(BTN1_NODE, gpios);

const struct device *const dev = DEVICE_DT_GET_ONE(ti_tmp1075);
const struct device *const flash_dev = DEVICE_DT_GET_ONE(jedec_spi_nor);

const struct device *const rtc1 = DEVICE_DT_GET(DT_ALIAS(rtc1));

const struct device *const mco = DEVICE_DT_GET(DT_NODELABEL(mco));

struct sensor_value temp_val;
bool read_data_flag = false;

void read_temp(void);

static int set_date_time(const struct device *rtc)
{
	int ret = 0;
	struct rtc_time tm = {
		.tm_year = 2025 - 1900,
		.tm_mon = 4 - 1,
		.tm_mday = 5,
		.tm_hour = 13,
		.tm_min = 26,
		.tm_sec = 0,
	};

	ret = rtc_set_time(rtc, &tm);
	if (ret < 0) {
		printf("Cannot write date time: %d\n", ret);
		return ret;
	}
	return ret;
}

static int get_date_time(const struct device *rtc)
{
	int ret = 0;
	struct rtc_time tm;

	ret = rtc_get_time(rtc, &tm);
	if (ret < 0) {
		printf("Cannot read date time: %d\n", ret);
		return ret;
	}

	printf("%04d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900,
	       tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

	return ret;
}

void rtc_callback(const struct device *dev, uint16_t id, void *user_data)
{
	read_data_flag = true;
}

void read_temp(void)
{
	int ret;
	
	ret = sensor_sample_fetch(dev);
	if (ret) {
		printf("sensor_sample_fetch failed ret %d\n", ret);
		return;
	}

	ret = sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &temp_val);
	if (ret) {
		printf("sensor_channel_get failed ret %d\n", ret);
		return;
	}

	printf(", %.2f\n", sensor_value_to_double(&temp_val));
}

int main(void)
{
	int ret;
	bool led_state = true;
	
	
	// Initializing devices

	if (!gpio_is_ready_dt(&led0)) {
		printf("Device is not ready\n");
		return 0;
	}
	if (!gpio_is_ready_dt(&led1)) {
		printf("Device is not ready\n");
		return 0;
	}
	if (!gpio_is_ready_dt(&btn1)) {
		printf("Device is not ready\n");
		return 0;
	}

	if (!device_is_ready(dev)) {
		printf("sensor: device not ready.\n");
		return 0;
	}
	if (!device_is_ready(rtc1)) {
		printf("RTC is not ready\n");
		return 0;
	}

	if (!device_is_ready(mco)) {
		printf("MCO1 device not ready\n");
		return -1;
	}

	if (!device_is_ready(flash_dev)) {
		printf("Flash device not ready\n");
		return -1;
	}

	ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		printf("Device is not ready\n");
		return 0;
	}

	ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		printf("Device is not ready\n");
		return 0;
	}

	ret = gpio_pin_configure_dt(&btn1, GPIO_INPUT);
	if (ret < 0) {
		printf("Device is not ready\n");
		return 0;
	}

	set_date_time(rtc1);


	// Set RTC Alarm
	struct rtc_time tm = {
		.tm_year = 0,
		.tm_mon = 0,
		.tm_mday = 0,
		.tm_hour = 0,
		.tm_min = 0,
		.tm_sec = 0,
	};
	rtc_alarm_set_time(rtc1, 0, RTC_ALARM_TIME_MASK_SECOND, &tm);
	rtc_alarm_set_callback(rtc1, 0, rtc_callback, NULL);

	while (1) {
		gpio_pin_toggle_dt(&led0);
		if(read_data_flag)
		{
			read_data_flag = false;
			get_date_time(rtc1);
			read_temp();
		}
		k_msleep(SLEEP_TIME_MS);
	}
	return 0;
}
