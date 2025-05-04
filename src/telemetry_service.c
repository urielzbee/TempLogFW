#include "telemetry_service.h"
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(telemetry);
K_MSGQ_DEFINE(telemetry_service_msgq, sizeof(uint8_t), 64, 1);

#define TELEMETRY_SERVICE_STACKSIZE 1024
#define TELEMETRY_SERVICE_PRIORITY 7
#define TELEMETRY_SERVICE_THEAT_INIT_DELAY 1000

#define TELEMETRY_SERVICE_MAX_MESSAGE_SIZE 256

#define SYNC_BYTE 0x7E

#define RX_CHUNK_LEN 32

struct device * uart;

enum eTelemetryState
{
	eSYNC_1 = 0,
	eSYNC_2,
	eCMD,
	eDATA_LEN,
	eDATA,
	eCRC_1,
	eCRC_2
};

typedef struct
{
	uint8_t cmd;
	uint8_t len;
	uint8_t data[TELEMETRY_SERVICE_MAX_MESSAGE_SIZE];
}telemetry_msg;

static telemetry_msg rxMsg = {0};

/* Private functions  */
static void serial_cb(const struct device *dev, void *user_data);
static void telemetry_service_byte_feed(uint8_t rxByte);
static void telemetry_service(void);

static void serial_cb(const struct device *dev, void *user_data)
{
    uint8_t c;
    if (!uart_irq_update(uart)) {
		return;
	}

	if (!uart_irq_rx_ready(uart)) {
		return;
	}

    while (uart_fifo_read(uart, &c, 1) == 1) 
    {
        k_msgq_put(&telemetry_service_msgq, &c, K_NO_WAIT);
    }
}

static void telemetry_service_byte_feed(uint8_t rxByte)
{
    static uint8_t telemetryState = 0;
    static uint16_t dataIndex = 0;
    static uint16_t dataLen = 0;

    switch(telemetryState)
	{
		case eSYNC_1:
			if(rxByte == SYNC_BYTE)
			{
				telemetryState = eSYNC_2;
			}
			break;
		case eSYNC_2:
			if(rxByte == SYNC_BYTE)
			{
				telemetryState = eCMD;
			}
			else
			{
				telemetryState = eSYNC_1;
			}
			break;
		case eCMD:
			telemetryState = eDATA_LEN;
			rxMsg.cmd = rxByte;
			break;
		case eDATA_LEN:
			dataIndex = 0;
			dataLen = rxByte;
			rxMsg.len = rxByte;
			if(rxByte == 0)
			{
				telemetryState = eCRC_1;
			}
			else
			{
				telemetryState = eDATA;
			}
			break;
		case eDATA:
			rxMsg.data[dataIndex] = rxByte;
			dataIndex++;
			if(dataIndex >= dataLen)
			{
				telemetryState = eCRC_1;
			}
			break;
		case eCRC_1:
			telemetryState = eCRC_2;
			break;
		case eCRC_2:
			telemetryState = eSYNC_1;
            LOG_INF("Message received\n");
			/*if(telemetry_command_handler)
			{
				telemetry_command_handler(rxMsg);
			}*/
			break;
		default:
			telemetryState = eSYNC_1;
			break;
	}
}

static void telemetry_service(void)
{
    uint8_t inByte;
    while(1)
    {
        /* Wait for uart rx byte */
        k_msgq_get(&telemetry_service_msgq, &inByte, K_FOREVER);
        telemetry_service_byte_feed(inByte);
    }
}

/* Public funtions*/

void telemetry_service_init(const struct device * uart_dev)
{
    int ret;

    /* Register the async interrupt handler */
    LOG_DBG("Module initialization");

    uart = uart_dev;

    if (!device_is_ready(uart)) {
		LOG_WRN("UART device not found!");
		return 0;
	}

	/* configure interrupt and callback to receive data */
	ret = uart_irq_callback_user_data_set(uart, serial_cb, NULL);
    if(ret != 0)
    {
        LOG_WRN("UART CALLBACK SET: %d",ret);
        return 0;
    }
    
    /* Enable reception interrupt */
    uart_irq_rx_enable(uart);
}

K_THREAD_DEFINE(telemetry_service_id, TELEMETRY_SERVICE_STACKSIZE, telemetry_service, NULL, NULL, NULL,
    TELEMETRY_SERVICE_PRIORITY, 0, TELEMETRY_SERVICE_THEAT_INIT_DELAY);