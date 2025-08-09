#ifndef TEMPERATURE_LOGGER_CONTROLLER_H
#define TEMPERATURE_LOGGER_CONTROLLER_H

#include <stdint.h>

void temperature_logger_controller_init(void);
void temperature_logger_controller_start(void);
void temperature_logger_controller_stop(void);
void temperature_logger_controller_set_log_interval(uint16_t interval);
uint16_t temperature_logger_controller_get_log_interval(void);

#endif