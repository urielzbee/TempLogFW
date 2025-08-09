
#include <zephyr/device.h>
#include <zephyr/drivers/rtc.h>
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>
#include "temperature_logger_controller.h"
#include "time_service.h"
#include "sensor_manager.h"
#include "flash_manager.h"

LOG_MODULE_REGISTER(temp_log_ctl);

#define TEMPERATURE_LOGGER_CONTROLLER_STACKSIZE 768
#define TEMPERATURE_LOGGER_CONTROLLER_PRIORITY 7
#define TEMPERATURE_LOGGER_CONTROLLER_THREAD_INIT_DELAY K_MSEC(1000)

K_THREAD_STACK_DEFINE(temp_log_stack, TEMPERATURE_LOGGER_CONTROLLER_STACKSIZE);
static struct k_thread temp_log_thread;
static k_tid_t temp_log_tid = NULL;

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

bool read_data_flag = false;
tempLogHeader header = {0};
static uint16_t log_interval = 1; // Default log interval in seconds

static void temperature_logger_controller(void);
static void set_next_alarm_time(int16_t interval);

static void time_service_callback(const struct device *dev, uint16_t id, void *user_data)
{
	read_data_flag = true;
}

void temperature_logger_controller_init(void) {
    int ret;

    temp_log_tid = k_thread_create(
        &temp_log_thread,
        temp_log_stack,
        TEMPERATURE_LOGGER_CONTROLLER_STACKSIZE,
        (k_thread_entry_t)temperature_logger_controller,
        NULL, NULL, NULL,
        TEMPERATURE_LOGGER_CONTROLLER_PRIORITY,
        0,
        TEMPERATURE_LOGGER_CONTROLLER_THREAD_INIT_DELAY
    );
    temperature_logger_controller_stop();

	struct rtc_time tm = {
		.tm_year = 2025 - 1900,
		.tm_mon = 4 - 1,
		.tm_mday = 18,
		.tm_hour = 14,
		.tm_min = 57,
		.tm_sec = 0,
	};

	ret = time_service_set_date_time(&tm);
	if (ret < 0) {
		LOG_ERR("Cannot write date time: %d", ret);
		return ret;
	}

    // Set RTC Alarm
	set_next_alarm_time(log_interval);
	time_service_alarm_set_callback(0, time_service_callback, NULL);
}

void temperature_logger_controller_start(void) {
    if (temp_log_tid != NULL) {
        LOG_INF("Starting temperature logger controller thread");
        k_thread_resume(temp_log_tid);
    }
}

void temperature_logger_controller_stop(void) {
    if (temp_log_tid != NULL) {
        LOG_INF("Stopping temperature logger controller thread");
        k_thread_suspend(temp_log_tid);
    }
}

void temperature_logger_controller_set_log_interval(uint16_t interval) {
	LOG_INF("Setting log interval to %u min", interval);
	log_interval = interval;
	set_next_alarm_time(log_interval);
}

static void temperature_logger_controller(void)
{
    tempLog temperatureLog;

	time_service_get_date_time(&temperatureLog.time);
	sensor_manager_read(&temperatureLog.temp);

    while (1) {
		if(read_data_flag)
		{
            LOG_INF("Read data flag set, logging temperature data");
			read_data_flag = false;
			set_next_alarm_time(log_interval);
			time_service_get_date_time(&temperatureLog.time);
			sensor_manager_read(&temperatureLog.temp);
			temperatureLog.magicWord = FLASH_MANAGER_MAGIC_WORD;
			flash_manager_write(&temperatureLog, sizeof(tempLog));
		}
		k_msleep(1000);
	}
}

static void set_next_alarm_time(int16_t interval)
{
	struct rtc_time tm_alarm = {
		.tm_year = 0,
		.tm_mon = 0,
		.tm_mday = 0,
		.tm_hour = 0,
		.tm_min = 0,
		.tm_sec = 0,
	};
	time_service_get_date_time(&tm_alarm);
	tm_alarm.tm_min += interval;
	if (tm_alarm.tm_min >= 60) {
		tm_alarm.tm_hour += tm_alarm.tm_min / 60;
		tm_alarm.tm_min %= 60;
	}
	time_service_alarm_set_time(0, RTC_ALARM_TIME_MASK_MINUTE | RTC_ALARM_TIME_MASK_SECOND, &tm_alarm);
}

