#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "command_handler.h"
#include "telemetry_service.h"
#include "time_service.h"

LOG_MODULE_REGISTER(cmd_Handler);

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
        LOG_INF("Temperature Command");
        break;
    case eSET_LOG_INTERVAL :
        LOG_INF("Set Log Interval Command");
        break;
    case eGET_LOG_INTERVAL :
        LOG_INF("Get Log Interval Command");
        break;
    case eSTREAM_LOGS :
        LOG_INF("Stream Logs Command");
        break;
    default:
        break;
    }
    
    telemetry_service_response(&response_msg);
}

void command_handler_init(void)
{
    telemetry_service_set_message_callback(command_handler_process);
}