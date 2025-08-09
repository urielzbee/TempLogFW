#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "command_handler.h"
#include "telemetry_service.h"
#include "time_service.h"
#include "sensor_manager.h"
#include "temperature_logger_controller.h"

LOG_MODULE_REGISTER(cmd_Handler);

static void stream_logs(void);

static struct rtc_time tm = {0};

enum eCMDs
{
	eFW_VER = 0x01,
	eHW_VER,
	eSET_TIME,
	eGET_TIME,
	eCPU_TEMP,
	eTEMP,
	eSET_LOG_INTERVAL,
    eGET_LOG_INTERVAL,
    eSTREAM_LOGS
};

static void command_handler_process(const telemetry_msg *msg)
{
    telemetry_msg response_msg;
    response_msg.cmd = msg->cmd;
    response_msg.len = 0; // Default length is 0

    // Process the telemetry message
    LOG_INF("Message received: cmd=0x%02X, len=%d", msg->cmd, msg->len);
    switch (msg->cmd)
    {
    case eFW_VER :
        LOG_INF("Firmware Version: %d.%d.%d", CONFIG_TL_FW_VER_MAJOR, CONFIG_TL_FW_VER_MINOR, CONFIG_TL_FW_VER_REV);
        response_msg.len = 3;
        response_msg.data[0] = CONFIG_TL_FW_VER_MAJOR;
        response_msg.data[1] = CONFIG_TL_FW_VER_MINOR;
        response_msg.data[2] = CONFIG_TL_FW_VER_REV;
        break;
    case eHW_VER :
        LOG_INF("Hardware Version: %d.%d.%d", CONFIG_TL_HW_VER_MAJOR, CONFIG_TL_HW_VER_MINOR, CONFIG_TL_HW_VER_REV);
        response_msg.len = 3;
        response_msg.data[0] = CONFIG_TL_HW_VER_MAJOR;
        response_msg.data[1] = CONFIG_TL_HW_VER_MINOR;
        response_msg.data[2] = CONFIG_TL_HW_VER_REV;
        break;
    case eSET_TIME :
        LOG_INF("Set Time Command");
        memset(&tm, 0, sizeof(tm));

        tm.tm_year = msg->data[0] + 100;
        tm.tm_mon = msg->data[1] - 1;
        tm.tm_mday = msg->data[2];
        tm.tm_hour = msg->data[3];
        tm.tm_min = msg->data[4];
        tm.tm_sec = msg->data[5];

        time_service_set_date_time(&tm);
        break;
    case eGET_TIME :
        
        memset(&tm, 0, sizeof(tm));
        time_service_get_date_time(&tm);

        LOG_INF("Get Time Command: %04d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900,
	       tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        
        response_msg.len = 6;
        response_msg.data[0] = tm.tm_year - 100;
        response_msg.data[1] = tm.tm_mon; // Month (1-12)
        response_msg.data[2] = tm.tm_mday; // Day of the month (1-31)
        response_msg.data[3] = tm.tm_hour; // Hour (0-23)
        response_msg.data[4] = tm.tm_min; // Minute (0-59)
        response_msg.data[5] = tm.tm_sec; // Second (0-59)

        break;
    case eCPU_TEMP :
        LOG_INF("CPU Temp Command");
        break;
    case eTEMP :
        double temp;
        sensor_manager_read(&temp);
        LOG_INF("Temperature Command: %.2f", temp);
        response_msg.len = 1;
        response_msg.data[0] = (uint8_t)temp;

        break;
    case eSET_LOG_INTERVAL :
        uint16_t log_interval = (msg->data[0] << 8) | msg->data[1]; // Combine two bytes into a uint16_t
        LOG_INF("Set Log Interval Command");
        temperature_logger_controller_set_log_interval(log_interval); // Combine two bytes into a uint16_t
        break;
    case eGET_LOG_INTERVAL :
        LOG_INF("Get Log Interval Command");
        uint16_t current_log_interval = 0;
        current_log_interval = temperature_logger_controller_get_log_interval();
        response_msg.len = 2;
        response_msg.data[0] = (current_log_interval >> 8) & 0xFF; // High byte
        response_msg.data[1] = current_log_interval & 0xFF; // Low byte
        break;
    case eSTREAM_LOGS :
        LOG_INF("Stream Logs Command");
        stream_logs();
        break;
    default:
        break;
    }
    
    telemetry_service_response(&response_msg);
}

static void stream_logs(void)
{
    telemetry_msg response_msg;
    response_msg.cmd = eSTREAM_LOGS;
    tempLog temperatureLog;
    response_msg.len = 8;

	for(uint32_t x = 0; x < TEMP_LOG_MAX_LOGS; x++)
	{
		flash_manager_read((uint8_t *)&temperatureLog, x);
		if(temperatureLog.magicWord == TEMP_LOG_MAGIC_WORD)
		{
			LOG_INF("%04d-%02d-%02d %02d:%02d:%02d", temperatureLog.time.tm_year + 1900,
				temperatureLog.time.tm_mon + 1, temperatureLog.time.tm_mday, temperatureLog.time.tm_hour, temperatureLog.time.tm_min, temperatureLog.time.tm_sec);
				LOG_INF(", %.2f", sensor_value_to_double(&temperatureLog.temp));
            response_msg.data[0] = temperatureLog.time.tm_year - 100; // Year - 2000
            response_msg.data[1] = temperatureLog.time.tm_mon + 1;
            response_msg.data[2] = temperatureLog.time.tm_mday;
            response_msg.data[3] = temperatureLog.time.tm_hour;
            response_msg.data[4] = temperatureLog.time.tm_min;
            response_msg.data[5] = temperatureLog.time.tm_sec;
            response_msg.data[6] = 0x00; // Type
            response_msg.data[7] = (uint8_t)sensor_value_to_double(&temperatureLog.temp);
            
            telemetry_service_response(&response_msg);
            k_msleep(50); // Small delay to ensure the message is sent before sending the next one
		}
		else
		{
			LOG_INF(".");
			break;
		}
	}
}

void command_handler_init(void)
{
    telemetry_service_set_message_callback(command_handler_process);
}