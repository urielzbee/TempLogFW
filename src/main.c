#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>

#include <zephyr/drivers/sensor.h>

#include <zephyr/device.h>
#include <zephyr/drivers/rtc.h>
#include <zephyr/sys/util.h>

#include <zephyr/drivers/flash.h>

#include "sensor_manager.h"
#include "time_service.h"
#include "telemetry_service.h"
#include "flash_manager.h"
#include "command_handler.h"

LOG_MODULE_REGISTER(main);

typedef struct 
{
	uint32_t magicWord;
	uint32_t index;
}tempLogHeader;

typedef struct 
{
	uint32_t magicWord;
	struct rtc_time time;
	struct sensor_value temp;
	uint8_t reserved[12];
}tempLog;

#define TEMP_LOG_HEADER_ADDRESS 0x00000000
#define TEMP_LOG_START_ADDRESS 0x1000
#define TEMP_LOG_ERASE_SIZE 0x1000
#define TEMP_LOG_CAPACITY 0x200000 /* 2MBytes */
#define TEMP_LOG_SIZE sizeof(tempLog)
#define TEMP_LOG_HEADER_SIZE sizeof(tempLogHeader)
#define TEMP_LOG_MAX_LOGS ((TEMP_LOG_CAPACITY - TEMP_LOG_START_ADDRESS) / TEMP_LOG_HEADER_SIZE)
#define TEMP_LOG_MAGIC_WORD 0xBADDCAFE

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
const struct device *const telemetry_uart = DEVICE_DT_GET(DT_ALIAS(telemetry_uart));

bool read_data_flag = false;

tempLogHeader header = {0};

void hard_fault(void);
void board_init(void);
void print_logs(void);

void hard_fault(void)
{
	LOG_ERR("HARD FAULT");
	while(1)
	{
		k_msleep(SLEEP_TIME_MS);
	}
}
void board_init(void)
{
	if(!sensor_manager_init(dev))
	{
		hard_fault();
	}

	if(!time_service_init(rtc1))
	{
		hard_fault();
	}
	telemetry_service_init(telemetry_uart);

	if(!flash_manager_init(flash_dev))
	{
		hard_fault();
	}

	command_handler_init();

}

static int set_date_time(const struct device *rtc)
{
	int ret = 0;
	struct rtc_time tm = {
		.tm_year = 2025 - 1900,
		.tm_mon = 4 - 1,
		.tm_mday = 18,
		.tm_hour = 14,
		.tm_min = 57,
		.tm_sec = 0,
	};

	ret = rtc_set_time(rtc, &tm);
	if (ret < 0) {
		LOG_ERR("Cannot write date time: %d", ret);
		return ret;
	}
	return ret;
}

static int get_date_time(const struct device *rtc, struct rtc_time *tm)
{
	int ret = 0;

	ret = rtc_get_time(rtc, tm);
	if (ret < 0) {
		LOG_ERR("Cannot read date time: %d", ret);
		return ret;
	}

	LOG_INF("%04d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900,
	       tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

	return ret;
}

void rtc_callback(const struct device *dev, uint16_t id, void *user_data)
{
	read_data_flag = true;
}

void print_logs(void)
{
	tempLog temperatureLog;

	for(uint32_t x = 0; x < TEMP_LOG_MAX_LOGS; x++)
	{
		flash_manager_read(flash_dev, (uint8_t *)&temperatureLog, x);
		if(temperatureLog.magicWord == FLASH_MANAGER_MAGIC_WORD)
		{
			LOG_INF("%04d-%02d-%02d %02d:%02d:%02d", temperatureLog.time.tm_year + 1900,
				temperatureLog.time.tm_mon + 1, temperatureLog.time.tm_mday, temperatureLog.time.tm_hour, temperatureLog.time.tm_min, temperatureLog.time.tm_sec);
				LOG_INF(", %.2f", sensor_value_to_double(&temperatureLog.temp));
		}
		else
		{
			LOG_INF(".");
			break;
		}
	}
}

int main(void)
{
	int ret;
	bool led_state = true;
	
	
	// Initializing devices

	if (!gpio_is_ready_dt(&led0)) {
		LOG_ERR("Device is not ready");
		return 0;
	}
	if (!gpio_is_ready_dt(&led1)) {
		LOG_ERR("Device is not ready");
		return 0;
	}
	if (!gpio_is_ready_dt(&btn1)) {
		LOG_ERR("Device is not ready");
		return 0;
	}

	board_init();
	
	

	if (!device_is_ready(mco)) {
		LOG_ERR("MCO1 device not ready");
		return -1;
	}


	ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		LOG_ERR("Device is not ready");
		return 0;
	}

	ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		LOG_ERR("Device is not ready");
		return 0;
	}

	ret = gpio_pin_configure_dt(&btn1, GPIO_INPUT);
	if (ret < 0) {
		LOG_ERR("Device is not ready");
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

	tempLog temperatureLog;

	get_date_time(rtc1, &temperatureLog.time);
	sensor_manager_read(&temperatureLog.temp);
	//gpio_pin_set_dt(&led0, 0);
	//gpio_pin_set_dt(&led1, 0);
	while (1) {
		gpio_pin_set_dt(&led0, 1);
		k_msleep(70);
		gpio_pin_set_dt(&led0, 0);
		if(read_data_flag)
		{
			read_data_flag = false;
			get_date_time(rtc1, &temperatureLog.time);
			sensor_manager_read(&temperatureLog.temp);	
			temperatureLog.magicWord = FLASH_MANAGER_MAGIC_WORD;
			flash_manager_write(flash_dev, &temperatureLog, sizeof(tempLog));
		}
		if(gpio_pin_get_dt(&btn1))
		{
			print_logs();
		}
		k_msleep(SLEEP_TIME_MS);
	}
	return 0;
}
