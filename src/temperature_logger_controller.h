#ifndef TEMPERATURE_LOGGER_CONTROLLER_H
#define TEMPERATURE_LOGGER_CONTROLLER_H

#include <stdint.h>
#include <zephyr/drivers/sensor.h>

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

void temperature_logger_controller_init(void);
void temperature_logger_controller_start(void);
void temperature_logger_controller_stop(void);
void temperature_logger_controller_set_log_interval(uint16_t interval);
uint16_t temperature_logger_controller_get_log_interval(void);

#endif