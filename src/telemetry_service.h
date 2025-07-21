#ifndef TELEMETRY_SERVICE_H
#define TELEMETRY_SERVICE_H

#include <zephyr/device.h>

#define TELEMETRY_SERVICE_MAX_MESSAGE_SIZE 256

typedef struct
{
	uint8_t cmd;
	uint8_t len;
	uint8_t data[TELEMETRY_SERVICE_MAX_MESSAGE_SIZE];
}telemetry_msg;

typedef void (*telemetry_service_message_callback)(telemetry_msg *msg);

void telemetry_service_init(const struct device * uart_dev);
void telemetry_service_set_message_callback(telemetry_service_message_callback cb);

#endif