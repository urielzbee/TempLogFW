#ifndef FLASH_MANAGER_H
#define FLASH_MANAGER_H

#include <zephyr/device.h>

#define FLASH_MANAGER_MAGIC_WORD        0xBADDCAFE

#define FLASH_MANAGER_LOG_CAPACITY      0x00200000 /* 2MBytes */
#define FLASH_MANAGER_HEADER_ADDRESS    0x00000000
#define FLASH_MANAGER_HEADER_SIZE       sizeof(flash_manager_header)
#define FLASH_MANAGER_START_ADDRESS     0x00001000
#define FLASH_MANAGER_ERASE_SIZE        0x00001000
#define FLASH_MANAGER_LOG_SIZE          0x40 /* 64 Bytes */
#define FLASH_MANAGER_MAX_LOGS          ((FLASH_MANAGER_LOG_CAPACITY - FLASH_MANAGER_START_ADDRESS) / FLASH_MANAGER_HEADER_SIZE)

typedef struct 
{
	uint32_t magicWord;
	uint32_t index;
}flash_manager_header;

int flash_manager_init(const struct device * dev);
int flash_manager_erase(void);
int flash_manager_write(uint8_t *  data, uint32_t len);
int flash_manager_read(uint8_t *  data, uint32_t index);
int flash_manager_get_index(void);
int flash_manager_suspend(void);
int flash_manager_resume(void);


#endif