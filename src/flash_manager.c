#include "flash_manager.h"
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/pm/device.h>
#include <zephyr/drivers/flash.h>

LOG_MODULE_REGISTER(flash_manager);

static flash_manager_header header = {0};
static const struct device * flash_dev = NULL;

int flash_manager_init(const struct device * dev)
{
    int ret;
    flash_dev = dev;
    if (!device_is_ready(flash_dev)) {
		LOG_ERR("Device not ready");
		return 0;
	}

    ret = flash_read(flash_dev, FLASH_MANAGER_HEADER_ADDRESS, &header, FLASH_MANAGER_HEADER_SIZE);
    if(ret != 0)
    {
        LOG_ERR("Read failed");
        return 0;
    }

    if(header.magicWord != FLASH_MANAGER_MAGIC_WORD)
	{
        LOG_INF("Magic word not found!!");
        flash_manager_erase();
    }
    else
    {
		LOG_INF("Index = %d",header.index);
    }

    return 1;
}
int flash_manager_erase(void)
{
    LOG_INF("Formating");
    flash_erase(flash_dev, FLASH_MANAGER_HEADER_ADDRESS, FLASH_MANAGER_ERASE_SIZE);
    header.magicWord = FLASH_MANAGER_MAGIC_WORD;
    header.index = 0x00;
    flash_write(flash_dev, FLASH_MANAGER_HEADER_ADDRESS, &header, FLASH_MANAGER_ERASE_SIZE);
}

int flash_manager_write(uint8_t * data, uint32_t len)
{
    uint32_t address = 0;

    /* Read current flash manager header index */
    flash_read(flash_dev, FLASH_MANAGER_HEADER_ADDRESS, &header, FLASH_MANAGER_HEADER_SIZE);

    address = FLASH_MANAGER_START_ADDRESS + (header.index * FLASH_MANAGER_LOG_SIZE);

    /* Validate address overflow */
	if(address >= FLASH_MANAGER_LOG_CAPACITY)
	{
		address = FLASH_MANAGER_START_ADDRESS;
		header.index = 0;
	}

    /* Check if sector start address*/
	if(address % FLASH_MANAGER_ERASE_SIZE == 0)
	{
		LOG_INF("Erasing sector at address %d ", address);
        flash_erase(flash_dev, address, FLASH_MANAGER_ERASE_SIZE);
	}

    /* Writing log to memory */
	LOG_INF("Log at 0x%04X\n",address);
	flash_write(flash_dev, address, data, len);

    /* Save header */
	header.index++;
	flash_erase(flash_dev, FLASH_MANAGER_HEADER_ADDRESS, FLASH_MANAGER_ERASE_SIZE);
	flash_write(flash_dev, FLASH_MANAGER_HEADER_ADDRESS, &header, FLASH_MANAGER_HEADER_SIZE);

    return 1;
}

int flash_manager_read(uint8_t *  data, uint32_t index)
{
    int address = 0;

    address = FLASH_MANAGER_START_ADDRESS + (index * FLASH_MANAGER_LOG_SIZE);

    flash_read(flash_dev, address, data, FLASH_MANAGER_LOG_SIZE);

    return 1;
}

int flash_manager_get_index(void)
{
    /* Read current flash manager header index */
    flash_read(flash_dev, FLASH_MANAGER_HEADER_ADDRESS, &header, FLASH_MANAGER_HEADER_SIZE);
    return header.index;
}

int flash_manager_suspend(void)
{
    int ret;
    ret = pm_device_action_run(flash_dev, PM_DEVICE_ACTION_SUSPEND);
    if (ret < 0)
    {
        LOG_ERR("Unable to suspend SPI NOR flash. (err: %d)", ret);
        return ret;
    }
    return 0;
}
int flash_manager_resume(void)
{
    int ret;
    ret = pm_device_action_run(flash_dev, PM_DEVICE_ACTION_RESUME);
    if (ret < 0)
    {
        LOG_ERR("Unable to resume SPI NOR flash. (err: %d)", ret);
        return ret;
    }
}