#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#include <zephyr/drivers/sensor.h>

#include <zephyr/device.h>
#include <zephyr/drivers/rtc.h>
#include <zephyr/sys/util.h>

#include <zephyr/drivers/flash.h>

typedef struct 
{
	uint32_t magicWord;
	uint32_t index;
}tempLogHeader;

typedef struct 
{
	uint32_t magicWord;
	struct rtc_time time;
	struct sensor_value temp;
	uint8_t reserved[12];
}tempLog;

#define TEMP_LOG_HEADER_ADDRESS 0x00000000
#define TEMP_LOG_START_ADDRESS 0x1000
#define TEMP_LOG_ERASE_SIZE 0x1000
#define TEMP_LOG_CAPACITY 0x200000 /* 2MBytes */
#define TEMP_LOG_SIZE sizeof(tempLog)
#define TEMP_LOG_HEADER_SIZE sizeof(tempLogHeader)
#define TEMP_LOG_MAX_LOGS ((TEMP_LOG_CAPACITY - TEMP_LOG_START_ADDRESS) / TEMP_LOG_HEADER_SIZE)
#define TEMP_LOG_MAGIC_WORD 0xBADDCAFE

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

bool read_data_flag = false;

tempLogHeader header = {0};

void read_temp(const struct device *dev, struct sensor_value *val);
void temp_log_init(void);
void save_log(tempLog log_value);
void print_logs(void);

static int set_date_time(const struct device *rtc)
{
	int ret = 0;
	struct rtc_time tm = {
		.tm_year = 2025 - 1900,
		.tm_mon = 4 - 1,
		.tm_mday = 18,
		.tm_hour = 14,
		.tm_min = 57,
		.tm_sec = 0,
	};

	ret = rtc_set_time(rtc, &tm);
	if (ret < 0) {
		printf("Cannot write date time: %d\n", ret);
		return ret;
	}
	return ret;
}

static int get_date_time(const struct device *rtc, struct rtc_time *tm)
{
	int ret = 0;

	ret = rtc_get_time(rtc, tm);
	if (ret < 0) {
		printf("Cannot read date time: %d\n", ret);
		return ret;
	}

	printf("%04d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900,
	       tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

	return ret;
}

void rtc_callback(const struct device *dev, uint16_t id, void *user_data)
{
	read_data_flag = true;
}

void read_temp(const struct device *dev, struct sensor_value *val)
{
	int ret;
	
	ret = sensor_sample_fetch(dev);
	if (ret) {
		printf("sensor_sample_fetch failed ret %d\n", ret);
		return;
	}

	ret = sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, val);
	if (ret) {
		printf("sensor_channel_get failed ret %d\n", ret);
		return;
	}

	printf(", %.2f\n", sensor_value_to_double(val));
}

void temp_log_init(void)
{
	printf("FLASH Read %d \n",flash_read(flash_dev, TEMP_LOG_HEADER_ADDRESS, &header, TEMP_LOG_HEADER_SIZE));
	printf("Size of templog = %d\n", sizeof(tempLog));
	
	if(header.magicWord != TEMP_LOG_MAGIC_WORD)
	{
		printf("Magic word not found!!\n");
		printf("0x%04X\n",header.magicWord);
		printf("Reinitializaing FLASH\n");
		printf("FLASH Erase %d \n",flash_erase(flash_dev, TEMP_LOG_HEADER_ADDRESS, TEMP_LOG_ERASE_SIZE));
		header.magicWord = TEMP_LOG_MAGIC_WORD;
		header.index = 0x00;
		printf("FLASH Write %d \n",flash_write(flash_dev, TEMP_LOG_HEADER_ADDRESS, &header, TEMP_LOG_HEADER_SIZE));
	}
	else
	{
		printf("Magic word found!!\n");
		printf("Index = %d\n",header.index);
	}
	if(gpio_pin_get_dt(&btn1))
	{
		printf("Ressetig index\n");
		header.magicWord = TEMP_LOG_MAGIC_WORD;
		header.index = 0;
		flash_write(flash_dev, TEMP_LOG_HEADER_ADDRESS, &header, TEMP_LOG_HEADER_SIZE);
	}
}

void save_log(tempLog log_value)
{
	uint32_t address = 0;
	
	log_value.magicWord = TEMP_LOG_MAGIC_WORD;
	flash_read(flash_dev, TEMP_LOG_HEADER_ADDRESS, &header, TEMP_LOG_HEADER_SIZE);

	address = TEMP_LOG_START_ADDRESS + (header.index * TEMP_LOG_SIZE);

	/* Validate address overflow */
	if(address >= TEMP_LOG_CAPACITY)
	{
		address = TEMP_LOG_START_ADDRESS;
		header.index = 0;
	}

	/* Check if sector start address*/
	if(address % TEMP_LOG_ERASE_SIZE == 0)
	{
		printf("FLASH Erase %d \n",flash_erase(flash_dev, address, TEMP_LOG_ERASE_SIZE));
	}

	/* Writing log to memory */
	printf("Saving log in address 0x%04X\n",address);
	flash_write(flash_dev, address, &log_value, TEMP_LOG_SIZE);

	/* Save header */
	header.index++;
	flash_erase(flash_dev, TEMP_LOG_HEADER_ADDRESS, TEMP_LOG_ERASE_SIZE);
	flash_write(flash_dev, TEMP_LOG_HEADER_ADDRESS, &header, TEMP_LOG_HEADER_SIZE);

}

void print_logs(void)
{
	uint32_t address = 0;
	tempLog temperatureLog;

	flash_read(flash_dev, TEMP_LOG_HEADER_ADDRESS, &header, TEMP_LOG_HEADER_SIZE);
	for(uint32_t x = 0; x < TEMP_LOG_MAX_LOGS; x++)
	{
		address = TEMP_LOG_START_ADDRESS + (x * TEMP_LOG_SIZE);
		flash_read(flash_dev, address, &temperatureLog, TEMP_LOG_SIZE);
		if(temperatureLog.magicWord == TEMP_LOG_MAGIC_WORD)
		{
			printf("%04d-%02d-%02d %02d:%02d:%02d", temperatureLog.time.tm_year + 1900,
				temperatureLog.time.tm_mon + 1, temperatureLog.time.tm_mday, temperatureLog.time.tm_hour, temperatureLog.time.tm_min, temperatureLog.time.tm_sec);
			printf(", %.2f\n", sensor_value_to_double(&temperatureLog.temp));
		}
		else
		{
			printf(".");
			break;
		}
	}
}

int main(void)
{
	int ret;
	bool led_state = true;
	
	
	// Initializing devices

	if (!gpio_is_ready_dt(&led0)) {
		printf("Device is not ready\n");
		return 0;
	}
	if (!gpio_is_ready_dt(&led1)) {
		printf("Device is not ready\n");
		return 0;
	}
	if (!gpio_is_ready_dt(&btn1)) {
		printf("Device is not ready\n");
		return 0;
	}

	if (!device_is_ready(dev)) {
		printf("sensor: device not ready.\n");
		return 0;
	}
	if (!device_is_ready(rtc1)) {
		printf("RTC is not ready\n");
		return 0;
	}

	if (!device_is_ready(mco)) {
		printf("MCO1 device not ready\n");
		return -1;
	}

	if (!device_is_ready(flash_dev)) {
		printf("Flash device not ready\n");
		return -1;
	}

	ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		printf("Device is not ready\n");
		return 0;
	}

	ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		printf("Device is not ready\n");
		return 0;
	}

	ret = gpio_pin_configure_dt(&btn1, GPIO_INPUT);
	if (ret < 0) {
		printf("Device is not ready\n");
		return 0;
	}

	set_date_time(rtc1);


	// Set RTC Alarm
	struct rtc_time tm = {
		.tm_year = 0,
		.tm_mon = 0,
		.tm_mday = 0,
		.tm_hour = 0,
		.tm_min = 0,
		.tm_sec = 0,
	};
	rtc_alarm_set_time(rtc1, 0, RTC_ALARM_TIME_MASK_SECOND, &tm);
	rtc_alarm_set_callback(rtc1, 0, rtc_callback, NULL);

	tempLog temperatureLog;
	temp_log_init();

	get_date_time(rtc1, &temperatureLog.time);
	read_temp(dev, &temperatureLog.temp);
	//gpio_pin_set_dt(&led0, 0);
	//gpio_pin_set_dt(&led1, 0);
	while (1) {
		gpio_pin_set_dt(&led0, 1);
		k_msleep(70);
		gpio_pin_set_dt(&led0, 0);
		if(read_data_flag)
		{
			read_data_flag = false;
			get_date_time(rtc1, &temperatureLog.time);
			read_temp(dev, &temperatureLog.temp);
			save_log(temperatureLog);
		}
		if(gpio_pin_get_dt(&btn1))
		{
			print_logs();
		}
		k_msleep(SLEEP_TIME_MS);
	}
	return 0;
}
