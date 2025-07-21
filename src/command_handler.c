#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "command_handler.h"
#include "telemetry_service.h"
#include "time_service.h"

LOG_MODULE_REGISTER(cmd_Handler);

enum eCMDs
{
	eFW_VER = 0x01,
	eHW_VER,
	eSET_TIME,
	eGET_TIME,
	eCPU_TIME,
	eTEMP,
	eSET_LOG_INTERVAL,
    eGET_LOG_INTERVAL,
    eSTREAM_LOGS
};

static void command_handler_process(const telemetry_msg *msg)
{
    // Process the telemetry message
    LOG_INF("Message received: cmd=0x%02X, len=%d", msg->cmd, msg->len);
    switch (msg->cmd)
    {
    case eFW_VER :
        LOG_INF("Firmware Version: 1.0.0");
        break;
    case eHW_VER :
        LOG_INF("Hardware Version: 1.0.0");
        break;
    case eSET_TIME :
        LOG_INF("Set Time Command");
        struct rtc_time tm = {
            .tm_year = 2000 + msg->data[0] - 1900,
            .tm_mon = msg->data[1] - 1,
            .tm_mday = msg->data[2],
            .tm_hour = msg->data[3],
            .tm_min = msg->data[4],
            .tm_sec = msg->data[5],
        };
        time_service_set_date_time(&tm);
        break;
    case eGET_TIME :
        LOG_INF("Get Time Command");
        break;
    case eCPU_TIME :
        LOG_INF("CPU Time Command");
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
}

void command_handler_init(void)
{
    telemetry_service_set_message_callback(command_handler_process);
}