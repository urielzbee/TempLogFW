#ifndef TELEMETRY_SERVICE_H
#define TELEMETRY_SERVICE_H

#include <zephyr/device.h>

void telemetry_service_init(const struct device * uart_dev);

#endif