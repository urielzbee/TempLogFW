#include "sensor_manager.h"

#include <stdio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(sensor_manager);

static const struct device * temp_dev = NULL;

int sensor_manager_init(const struct device * dev)
{
	temp_dev = dev;
    if (!device_is_ready(temp_dev)) {
		LOG_ERR("sensor: device not ready.");
		return 0;
	}
    return 1;
}

int sensor_manager_read(struct sensor_value * temp)
{
    int ret;
	
	ret = sensor_sample_fetch(temp_dev);
	if (ret) {
		LOG_ERR("sensor_sample_fetch failed ret %d\n", ret);
		return 1;
	}

	ret = sensor_channel_get(temp_dev, SENSOR_CHAN_AMBIENT_TEMP, temp);
	if (ret) {
		LOG_ERR("sensor_channel_get failed ret %d\n", ret);
		return 1;
	}

	LOG_INF("Temperature: %.2f C", sensor_value_to_double(temp));

    return 0;
}