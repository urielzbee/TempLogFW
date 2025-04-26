#include "sensor_manager.h"

#include <stdio.h>
#include <zephyr/drivers/sensor.h>

int sensor_manager_init(const struct device * dev)
{
    if (!device_is_ready(dev)) {
		printf("sensor: device not ready.\n");
		return 0;
	}
    return 1;
}

int sensor_manager_read(const struct device * dev, double * temp)
{
    int ret;
    struct sensor_value val;
	
	ret = sensor_sample_fetch(dev);
	if (ret) {
		printf("sensor_sample_fetch failed ret %d\n", ret);
		return 1;
	}

	ret = sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &val);
	if (ret) {
		printf("sensor_channel_get failed ret %d\n", ret);
		return 1;
	}

    *temp = sensor_value_to_double(&val);

    return 0;
}