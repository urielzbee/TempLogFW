/*
 * telemetry.h
 *
 *  Created on: Feb 25, 2025
 *      Author: uriel
 */

#ifndef TELEMETRY_TELEMETRY_H_
#define TELEMETRY_TELEMETRY_H_

#define TELEMETRY_MAX_MESSAGE_SIZE 256

#include "main.h"

typedef struct
{
	uint8_t cmd;
	uint8_t len;
	uint8_t data[TELEMETRY_MAX_MESSAGE_SIZE];
}telemetry_msg;


void telemetry_set_command_handler(void (*p_telemetry_command_handler)(telemetry_msg));
void telemetry_byte_feed(uint8_t rxByte);
void telemetry_msg_transmit(telemetry_msg * rx_msg);

#endif /* TELEMETRY_TELEMETRY_H_ */
