#ifndef FLASH_MANAGER_H
#define FLASH_MANAGER_H

#include <zephyr/device.h>

#define FLASH_MANAGER_MAGIC_WORD        0xBADDCAFE

int flash_manager_init(const struct device * dev);
int flash_manager_erase(void);
int flash_manager_write(uint8_t *  data, uint32_t len);
int flash_manager_read(uint8_t *  data, uint32_t index);


#endif