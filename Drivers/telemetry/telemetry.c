/*
 * telemetry.c
 *
 *  Created on: Feb 25, 2025
 *      Author: uriel
 */
#include "telemetry.h"

#define SYNC_BYTE 0x7E

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
extern UART_HandleTypeDef huart1;

static uint8_t telemetryState = 0;
static uint16_t dataIndex = 0;
static uint16_t dataLen = 0;
static telemetry_msg rxMsg = {0};

void (*telemetry_command_handler)(telemetry_msg rx_msg);

void telemetry_set_command_handler(void (*p_telemetry_command_handler)(telemetry_msg rx_msg))
{
	telemetry_command_handler = p_telemetry_command_handler;
}

void telemetry_byte_feed(uint8_t rxByte)
{

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
			if(telemetry_command_handler)
			{
				telemetry_command_handler(rxMsg);
			}
			break;
		default:
			telemetryState = eSYNC_1;
			break;
	}

}

void telemetry_msg_transmit(telemetry_msg * rx_msg)
{
	uint8_t txBuff[256] = {0};
	uint8_t index = 0;
	uint16_t checksum = 0;
	txBuff[index++] = SYNC_BYTE;
	txBuff[index++] = SYNC_BYTE;
	txBuff[index++] = rx_msg->cmd;
	txBuff[index++] = rx_msg->len;
	for(uint8_t x = 0; x < rx_msg->len; x++)
	{
		txBuff[index++] = rx_msg->data[x];
	}
	txBuff[index++] = (checksum >> 8) & 0xFF;
	txBuff[index++] = checksum & 0xFF;
	HAL_UART_Transmit(&huart1, txBuff, index, 100);

}
