#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <zephyr/device.h>

int sensor_manager_init(const struct device * dev);

int sensor_manager_read(const struct device * dev, double * temp);

#endif