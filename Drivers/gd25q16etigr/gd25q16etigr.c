/*
 * gd25q16etigr.c
 *
 *  Created on: Feb 5, 2025
 *      Author: uriel
 */
#include "main.h"

#define GD25_PAGE_PROGRAM				0x02
#define GD25_READ_DATA_BYTES			0x03
#define GD25_WRITE_EN 					0x06
#define GD25_WRITE_DIS 					0x04
#define GD25_READ_STATUS_REGISTER_1 	0x05
#define GD25_READ_STATUS_REGISTER_2 	0x35
#define GD25_ERASE_SECTOR			 	0x20

extern SPI_HandleTypeDef hspi1;

struct gd25q16etigr_status
{
	uint8_t WIP; 	/* S0 - Erase/Write In Progress */
	uint8_t WEL; 	/* S1 - Write Enable Latch */
	uint8_t BP0_4; 	/* S2_S6 - SBlock protect Bit */
	uint8_t SRP0; 	/* S7 - Status Register Protection Bit */

	uint8_t SRP1;	/* S8 - Suspend Bit */
	uint8_t QE;		/* S9 - Complement Protect Bit */
	uint8_t LB0;	/* S10 - Dummy Configuration Bit */
	uint8_t LB1;	/* S11 - Security Register Lock Bit */
	uint8_t DC;		/* S12 - Security Register Lock Bit */
	uint8_t CMP;	/* S14 - Quad Enable Bit */
	uint8_t SUS;	/* S15 - Status Register Protection Bit */
};

static struct gd25q16etigr_status status;

static void nCS_Pin_Write( GPIO_PinState PinState)
{
	HAL_GPIO_WritePin(FLASH_nCS_GPIO_Port, FLASH_nCS_Pin, PinState);
}

static void nWP_Pin_Write( GPIO_PinState PinState)
{
	HAL_GPIO_WritePin(FLASH_nWP_GPIO_Port, FLASH_nWP_Pin, PinState);
}

static void spi_write(uint8_t * buff, uint32_t len)
{
	HAL_SPI_Transmit(&hspi1, buff, len, 1000);
}

static void spi_read(uint8_t * buff, uint32_t len)
{
	HAL_SPI_Receive(&hspi1, buff, len, 1000);
}

static void gd25q16etigr_writeEnable(void)
{
	uint8_t cmd = GD25_WRITE_EN;

	nCS_Pin_Write(GPIO_PIN_RESET);

	spi_write(&cmd,1);

	nCS_Pin_Write(GPIO_PIN_SET);
}

static void gd25q16etigr_writeDisable(void)
{
	uint8_t cmd = GD25_WRITE_DIS;

	nCS_Pin_Write(GPIO_PIN_RESET);

	spi_write(&cmd,1);

	nCS_Pin_Write(GPIO_PIN_SET);
}

static void gd25q16etigr_readStatusRegister(void)
{
	uint8_t cmd = GD25_READ_STATUS_REGISTER_1;
	uint8_t buff = 0;

	/* Read Status Register 1*/
	nCS_Pin_Write(GPIO_PIN_RESET);

	spi_write(&cmd,1);
	spi_read(&buff,1);

	status.WIP = buff & 0x01;
	status.WEL = (buff >> 1) & 0x01;
	status.BP0_4 = (buff >> 2) & 0x1F;
	status.SRP0 = (buff >> 7) & 0x01;

	nCS_Pin_Write(GPIO_PIN_SET);

	/* Read Status Register 2*/
	cmd = GD25_READ_STATUS_REGISTER_2;
	buff = 0;
	nCS_Pin_Write(GPIO_PIN_RESET);

	spi_write(&cmd,1);
	spi_read(&buff,1);

	status.SRP1 = buff & 0x01;
	status.QE = (buff >> 1) & 0x01;
	status.LB0 = (buff >> 2) & 0x01;
	status.LB1 = (buff >> 3) & 0x01;
	status.DC = (buff >> 4) & 0x01;
	status.CMP = (buff >> 6) & 0x01;
	status.SUS = (buff >> 7) & 0x01;

	nCS_Pin_Write(GPIO_PIN_SET);
}

void gd25q16etigr_readManufacturerID(void)
{
	uint8_t buff[2] = {0};
	uint8_t cmd[4] = {0x90, 0x00, 0x00, 0x00}; //Read Manufacturer/Device ID

	nCS_Pin_Write(GPIO_PIN_RESET);
	spi_write(cmd,4);
	spi_read(buff, 2);
	nCS_Pin_Write(GPIO_PIN_SET);
	HAL_Delay(100);
}

void gd25q16etigr_readDataBytes(uint32_t address, uint8_t * buff, uint32_t len)
{
	uint8_t cmd = GD25_READ_DATA_BYTES;
	uint8_t addr[3] = {0};

	addr[0] = address & 0xFF;
	addr[1] = (address >> 8) & 0xFF;
	addr[2] = (address >> 16) & 0xFF;

	nCS_Pin_Write(GPIO_PIN_RESET);
	spi_write(&cmd,1);
	spi_write(addr,3);
	spi_read(buff, len);
	nCS_Pin_Write(GPIO_PIN_SET);
	HAL_Delay(100);
}

void gd25q16etigr_pageProgram(uint32_t address, uint8_t * buff, uint32_t len)
{
	uint8_t cmd = GD25_PAGE_PROGRAM;
	uint8_t addr[3] = {0};

	addr[0] = address & 0xFF;
	addr[1] = (address >> 8) & 0xFF;
	addr[2] = (address >> 16) & 0xFF;

	gd25q16etigr_writeEnable();

	nCS_Pin_Write(GPIO_PIN_RESET);
	spi_write(&cmd,1);
	spi_write(addr,3);
	spi_write(buff, len);
	nCS_Pin_Write(GPIO_PIN_SET);
	HAL_Delay(100);
}

void gd25q16etigr_eraseSector(uint32_t address)
{
	uint8_t cmd = GD25_ERASE_SECTOR;
	uint8_t addr[3] = {0};

	addr[0] = address & 0xFF;
	addr[1] = (address >> 8) & 0xFF;
	addr[2] = (address >> 16) & 0xFF;

	gd25q16etigr_writeEnable();

	nCS_Pin_Write(GPIO_PIN_RESET);
	spi_write(&cmd,1);
	spi_write(addr,3);
	nCS_Pin_Write(GPIO_PIN_SET);
	HAL_Delay(100);
}


void gd25q16etigr_init(void)
{
	uint8_t wBuff[256] = {0};
	uint8_t rBuff[256] = {0};
	uint32_t addr = 0x00;
	nCS_Pin_Write(GPIO_PIN_SET);
	nWP_Pin_Write(GPIO_PIN_SET);
	HAL_Delay(100);

	wBuff[0] = 'U';
	wBuff[1] = 'r';
	wBuff[2] = 'i';
	wBuff[3] = 'e';
	wBuff[4] = 'l';

	gd25q16etigr_readStatusRegister();
	gd25q16etigr_eraseSector(addr);
	gd25q16etigr_pageProgram(addr, wBuff, 5);
	gd25q16etigr_readDataBytes(addr, rBuff, sizeof(rBuff));
	HAL_Delay(100);
}
