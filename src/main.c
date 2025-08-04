#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>

#include <zephyr/drivers/sensor.h>

#include <zephyr/device.h>
#include <zephyr/drivers/rtc.h>
#include <zephyr/sys/util.h>

#include <zephyr/drivers/flash.h>

#include "sensor_manager.h"
#include "time_service.h"
#include "telemetry_service.h"
#include "flash_manager.h"
#include "command_handler.h"
#include "temperature_logger_controller.h"

LOG_MODULE_REGISTER(main);


/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define BTN1_NODE DT_ALIAS(btn1)

/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec btn1 = GPIO_DT_SPEC_GET(BTN1_NODE, gpios);

const struct device *const dev = DEVICE_DT_GET_ONE(ti_tmp1075);
const struct device *const flash_dev = DEVICE_DT_GET_ONE(jedec_spi_nor);

const struct device *const rtc1 = DEVICE_DT_GET(DT_ALIAS(rtc1));

const struct device *const mco = DEVICE_DT_GET(DT_NODELABEL(mco));
const struct device *const telemetry_uart = DEVICE_DT_GET(DT_ALIAS(telemetry_uart));


void hard_fault(void);
void board_init(void);
void print_logs(void);

void hard_fault(void)
{
	LOG_ERR("HARD FAULT");
	while(1)
	{
		k_msleep(SLEEP_TIME_MS);
	}
}
void board_init(void)
{
	int ret;

	if (!gpio_is_ready_dt(&led0)) {
		LOG_ERR("Device is not ready");
		hard_fault();
	}
	if (!gpio_is_ready_dt(&led1)) {
		LOG_ERR("Device is not ready");
		hard_fault();
	}
	if (!gpio_is_ready_dt(&btn1)) {
		LOG_ERR("Device is not ready");
		hard_fault();
	}

	if (!device_is_ready(mco)) {
		LOG_ERR("MCO1 device not ready");
		hard_fault();
	}

	ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		LOG_ERR("Device is not ready");
		hard_fault();
	}

	ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		LOG_ERR("Device is not ready");
		hard_fault();
	}

	ret = gpio_pin_configure_dt(&btn1, GPIO_INPUT);
	if (ret < 0) {
		LOG_ERR("Device is not ready");
		hard_fault();
	}

	if(!sensor_manager_init(dev))
	{
		hard_fault();
	}

	if(!time_service_init(rtc1))
	{
		hard_fault();
	}
	telemetry_service_init(telemetry_uart);

	if(!flash_manager_init(flash_dev))
	{
		hard_fault();
	}

	command_handler_init();

	temperature_logger_controller_init();
	temperature_logger_controller_start();

}

/*void print_logs(void)
{
	tempLog temperatureLog;

	for(uint32_t x = 0; x < TEMP_LOG_MAX_LOGS; x++)
	{
		flash_manager_read(flash_dev, (uint8_t *)&temperatureLog, x);
		if(temperatureLog.magicWord == FLASH_MANAGER_MAGIC_WORD)
		{
			LOG_INF("%04d-%02d-%02d %02d:%02d:%02d", temperatureLog.time.tm_year + 1900,
				temperatureLog.time.tm_mon + 1, temperatureLog.time.tm_mday, temperatureLog.time.tm_hour, temperatureLog.time.tm_min, temperatureLog.time.tm_sec);
				LOG_INF(", %.2f", sensor_value_to_double(&temperatureLog.temp));
		}
		else
		{
			LOG_INF(".");
			break;
		}
	}
}*/

int main(void)
{
	int ret;
	bool led_state = true;
	
	board_init();

	while (1) {
		gpio_pin_set_dt(&led0, 1);
		k_msleep(70);
		gpio_pin_set_dt(&led0, 0);
		k_msleep(SLEEP_TIME_MS);
	}
	return 0;
}
