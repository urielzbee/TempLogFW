#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <zephyr/device.h>

int sensor_manager_init(const struct device * dev);

int sensor_manager_read(double * temp);

#endif