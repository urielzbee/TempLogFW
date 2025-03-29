/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   100

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

int main(void)
{
	int ret;
	bool led_state = true;

	if (!gpio_is_ready_dt(&led0)) {
		return 0;
	}
	if (!gpio_is_ready_dt(&led1)) {
		return 0;
	}
	if (!gpio_is_ready_dt(&btn1)) {
		return 0;
	}

	ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return 0;
	}

	ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return 0;
	}

	ret = gpio_pin_configure_dt(&btn1, GPIO_INPUT);
	if (ret < 0) {
		return 0;
	}

	while (1) {
		gpio_pin_toggle_dt(&led0);
		if(gpio_pin_get_dt(&btn1) == 1)
		{
			gpio_pin_set_dt(&led1,1);
		}
		else
		{
			gpio_pin_set_dt(&led1,0);
		}
		
		led_state = !led_state;
		printf("LED state: %s\n", led_state ? "ON" : "OFF");
		k_msleep(SLEEP_TIME_MS);
	}
	return 0;
}
