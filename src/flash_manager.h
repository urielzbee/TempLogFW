#ifndef FLASH_MANAGER_H
#define FLASH_MANAGER_H

#include <zephyr/device.h>

#define FLASH_MANAGER_MAGIC_WORD        0xBADDCAFE

int flash_manager_init(const struct device * flash_dev);
int flash_manager_erase(const struct device * flash_dev);
int flash_manager_write(const struct device * flash_dev, uint8_t *  data, uint32_t len);
int flash_manager_read(const struct device * flash_dev, uint8_t *  data, uint32_t index);


#endif