#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>

int sensor_manager_init(const struct device * dev);

int sensor_manager_read(struct sensor_value * temp);

#endif