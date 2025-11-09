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
#define LED1_NODE DT_ALIAS(led1)
#define BTN1_NODE DT_ALIAS(btn1)

/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */

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

int main(void)
{
	int ret;
	bool led_state = true;
	
	board_init();

	gpio_pin_set_dt(&led1, 0);

	while (1) {
		k_msleep(SLEEP_TIME_MS);
	}
	return 0;
}
