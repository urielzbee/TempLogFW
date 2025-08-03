#include "telemetry_service.h"
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/ring_buffer.h>

LOG_MODULE_REGISTER(telemetry);
K_MSGQ_DEFINE(telemetry_service_msgq, sizeof(uint8_t), 64, 1);

#define TELEMETRY_SERVICE_STACKSIZE 1024
#define TELEMETRY_SERVICE_PRIORITY 7
#define TELEMETRY_SERVICE_THEAT_INIT_DELAY 1000

#define SYNC_BYTE 0x7E


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

static telemetry_msg rxMsg = {0};

static uint8_t tx_buff[2 + sizeof(telemetry_msg) + 2] = {0};
static uint16_t tx_len = 0;
struct ring_buf tx_ringbuf;

static telemetry_service_message_callback message_callback = NULL;

/* Private functions  */
static void serial_cb(const struct device *dev, void *user_data);
static void telemetry_service_byte_feed(uint8_t rxByte);
static void telemetry_service(void);

static void serial_cb(const struct device *dev, void *user_data)
{
	uint8_t c;
	
	while (uart_irq_update(dev) && uart_irq_is_pending(dev)) 
	{
		if(uart_irq_rx_ready(dev)) 
		{
			// Read data from the UART
			if (uart_fifo_read(dev, &c, 1) == 1) {
				k_msgq_put(&telemetry_service_msgq, &c, K_NO_WAIT);
			}
			continue;
		}
		if (uart_irq_tx_ready(dev)) {

			int rb_len;
			rb_len = ring_buf_get(&tx_ringbuf, &c, 1);
			if (!rb_len) {
				LOG_DBG("Ring buffer empty, disable TX IRQ");
				uart_irq_tx_disable(dev);
				continue;
			}
			uart_fifo_fill(dev, &c, rb_len);
		}
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
			// Check CRC here if needed
			if(message_callback)
			{
				message_callback(&rxMsg);
			}
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

	ring_buf_init(&tx_ringbuf, sizeof(tx_buff), tx_buff);

    /* Enable reception interrupt */
    uart_irq_rx_enable(uart);

	uart_irq_tx_disable(uart);
}

void telemetry_service_set_message_callback(telemetry_service_message_callback cb)
{
    message_callback = cb;
}

void telemetry_service_response(telemetry_msg * msg)
{
	
	int ret;
	tx_buff[0] = SYNC_BYTE; // Start byte
	tx_buff[1] = SYNC_BYTE; // End byte
	tx_buff[2] = msg->cmd; // Command
	tx_buff[3] = msg->len; // Length
	if (msg->len > 0) {
		memcpy(&tx_buff[4], msg->data, msg->len); // Data
	}
	tx_buff[4 + msg->len] = 0; // CRC placeholder, can be replaced with actual CRC calculation
	tx_buff[5 + msg->len] = 0; // CRC placeholder, can be replaced with actual CRC calculation
	tx_len = 6 + msg->len; // Total length of the message

	ring_buf_put(&tx_ringbuf, tx_buff, tx_len);

	uart_irq_tx_enable(uart); // Enable TX interrupt
}

K_THREAD_DEFINE(telemetry_service_id, TELEMETRY_SERVICE_STACKSIZE, telemetry_service, NULL, NULL, NULL,
    TELEMETRY_SERVICE_PRIORITY, 0, TELEMETRY_SERVICE_THEAT_INIT_DELAY);