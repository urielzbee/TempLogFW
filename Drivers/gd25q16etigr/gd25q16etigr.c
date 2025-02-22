/*
 * gd25q16etigr.c
 *
 *  Created on: Feb 5, 2025
 *      Author: uriel
 */
#include "main.h"

#define GD25_PAGE_PROGRAM				0x02
#define GD25_READ_DATA_BYTES			0x03
#define GD25_WRITE_DIS 					0x04
#define GD25_READ_STATUS_REGISTER_1 	0x05
#define GD25_WRITE_EN 					0x06
#define GD25_ERASE_SECTOR			 	0x20
#define GD25_READ_STATUS_REGISTER_2 	0x35


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

/**
 * @brief Write a state to the nCS (Chip Select) pin.
 *
 * This function is used to set the state (HIGH or LOW) of the nCS pin for the Flash memory.
 *
 * @param PinState The state to set the pin to. It can be:
 *                 - GPIO_PIN_SET: Set the pin to high
 *                 - GPIO_PIN_RESET: Set the pin to low
 *
 * @note This function directly interacts with the hardware and is part of the GPIO abstraction layer.
 */
static void nCS_Pin_Write(GPIO_PinState PinState)
{
	HAL_GPIO_WritePin(FLASH_nCS_GPIO_Port, FLASH_nCS_Pin, PinState);
}

/**
 * @brief Write a state to the nWP (Write Protect) pin.
 *
 * This function is used to set the state (HIGH or LOW) of the nWP pin for the Flash memory,
 * which controls the write protection feature.
 *
 * @param PinState The state to set the pin to. It can be:
 *                 - GPIO_PIN_SET: Enable write (set the pin to high)
 *                 - GPIO_PIN_RESET: Disable write (set the pin to low)
 *
 * @note This function directly interacts with the hardware and is part of the GPIO abstraction layer.
 */
static void nWP_Pin_Write( GPIO_PinState PinState)
{
	HAL_GPIO_WritePin(FLASH_nWP_GPIO_Port, FLASH_nWP_Pin, PinState);
}

/**
 * @brief Transmit data over SPI.
 *
 * This function sends a specified number of bytes from a buffer over the SPI interface.
 * It uses the `HAL_SPI_Transmit` function to transmit data through the `hspi1` SPI peripheral.
 *
 * @param buff Pointer to the buffer containing the data to be transmitted.
 * @param len  The number of bytes to transmit from the buffer.
 *
 * @note This function performs a blocking transmission, waiting up to 1000 ms for the transfer to complete.
 */
static void spi_write(uint8_t * buff, uint32_t len)
{
	HAL_SPI_Transmit(&hspi1, buff, len, 1000);
}

/**
 * @brief Receive data over SPI.
 *
 * This function receives a specified number of bytes into a buffer from the SPI interface.
 * It uses the `HAL_SPI_Receive` function to receive data through the `hspi1` SPI peripheral.
 *
 * @param buff Pointer to the buffer where the received data will be stored.
 * @param len  The number of bytes to receive into the buffer.
 *
 * @note This function performs a blocking reception, waiting up to 1000 ms for the transfer to complete.
 */
static void spi_read(uint8_t * buff, uint32_t len)
{
	HAL_SPI_Receive(&hspi1, buff, len, 1000);
}

/**
 * @brief Enable write operations on the GD25Q16ETIGR Flash memory.
 *
 * This function sends a "Write Enable" command to the GD25Q16ETIGR Flash memory,
 * which allows subsequent write operations to the memory. It performs the following steps:
 * 1. Sets the chip select (nCS) pin to low to select the Flash memory.
 * 2. Sends the "Write Enable" command over SPI.
 * 3. Sets the chip select (nCS) pin back to high to deselect the Flash memory.
 *
 * @note The "Write Enable" command must be issued before performing write operations on the memory.
 */
static void gd25q16etigr_writeEnable(void)
{
	uint8_t cmd = GD25_WRITE_EN;

	nCS_Pin_Write(GPIO_PIN_RESET);

	spi_write(&cmd,1);

	nCS_Pin_Write(GPIO_PIN_SET);
}

/**
 * @brief Disable write operations on the GD25Q16ETIGR Flash memory.
 *
 * This function sends a "Write Disable" command to the GD25Q16ETIGR Flash memory,
 * which prevents subsequent write operations to the memory. It performs the following steps:
 * 1. Sets the chip select (nCS) pin to low to select the Flash memory.
 * 2. Sends the "Write Disable" command over SPI.
 * 3. Sets the chip select (nCS) pin back to high to deselect the Flash memory.
 *
 * @note The "Write Disable" command must be issued to disable write operations after write enable has been performed.
 */
static void gd25q16etigr_writeDisable(void)
{
	uint8_t cmd = GD25_WRITE_DIS;

	nCS_Pin_Write(GPIO_PIN_RESET);

	spi_write(&cmd,1);

	nCS_Pin_Write(GPIO_PIN_SET);
}

/**
 * @brief Read the Status Registers of the GD25Q16ETIGR Flash memory.
 *
 * This function reads both the Status Register 1 and Status Register 2 of the GD25Q16ETIGR Flash memory
 * and extracts the relevant status bits. The function performs the following steps:
 * 1. Reads Status Register 1 and updates the `status` structure with the extracted values.
 * 2. Reads Status Register 2 and updates the `status` structure with the extracted values.
 *
 * The status bits in both registers provide important information about the state of the Flash memory,
 * such as Write In Progress (WIP), Write Enable Latch (WEL), and various protection and status flags.
 *
 * @note The function uses SPI to communicate with the memory and updates the `status` structure with the values
 *       read from the status registers.
 */
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

/**
 * @brief Read the Manufacturer and Device ID of the GD25Q16ETIGR Flash memory.
 *
 * This function sends the "Read Manufacturer/Device ID" command to the GD25Q16ETIGR Flash memory
 * and retrieves the Manufacturer ID and Device ID, which are used to identify the Flash memory chip.
 * The function performs the following steps:
 * 1. Sends the command to the Flash memory to initiate the ID read process.
 * 2. Reads the 2-byte Manufacturer and Device ID into the buffer.
 * 3. Deselects the Flash memory after the read operation is completed.
 *
 * @note The Manufacturer and Device ID can be used for identification, validation, and ensuring
 *       that the correct chip is being accessed.
 */
static void gd25q16etigr_readManufacturerID(void)
{
	uint8_t buff[2] = {0};
	uint8_t cmd[4] = {0x90, 0x00, 0x00, 0x00}; //Read Manufacturer/Device ID

	nCS_Pin_Write(GPIO_PIN_RESET);
	spi_write(cmd,4);
	spi_read(buff, 2);
	nCS_Pin_Write(GPIO_PIN_SET);
}


/**
 * @brief Read data bytes from the GD25Q16ETIGR Flash memory.
 *
 * This function reads a specified number of data bytes from the GD25Q16ETIGR Flash memory
 * starting from a given address. It performs the following steps:
 * 1. Sends the "Read Data Bytes" command to the Flash memory.
 * 2. Sends the 3-byte address from which to begin reading.
 * 3. Reads the specified number of bytes of data into the provided buffer.
 * 4. Deselects the Flash memory after the read operation is completed.
 *
 * @param address The starting address in the Flash memory from where to begin reading.
 * @param buff Pointer to the buffer where the read data will be stored.
 * @param len The number of bytes to read from the Flash memory.
 *
 * @note This function uses SPI to communicate with the Flash memory and performs a blocking read.
 */
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
}

/**
 * @brief Program (write) data to a specified page in the GD25Q16ETIGR Flash memory.
 *
 * This function performs a page program operation on the GD25Q16ETIGR Flash memory. It writes data
 * to a specified address in the Flash memory, sending the data in pages. The function performs the
 * following steps:
 * 1. Sends the "Page Program" command to the Flash memory.
 * 2. Sends the 3-byte address where data will be written.
 * 3. Sends the data to be written from the buffer.
 * 4. Deselects the Flash memory after the operation is completed.
 *
 * The Flash memory allows writing data to it in pages (typically 256 bytes per page). The number of bytes
 * to be written is specified by the `len` parameter.
 *
 * @param address The starting address in the Flash memory where the data will be written.
 * @param buff Pointer to the buffer containing the data to be written to the Flash memory.
 * @param len The number of bytes to write from the buffer to the Flash memory.
 *
 * @note This function uses SPI for communication and assumes the write enable operation is performed
 *       beforehand via the `gd25q16etigr_writeEnable()` function.
 */

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
}

/**
 * @brief Erase a sector of the GD25Q16ETIGR Flash memory.
 *
 * This function erases a specified sector in the GD25Q16ETIGR Flash memory. The Flash memory is divided
 * into sectors, and this operation will erase all data within the sector specified by the `address` parameter.
 * The function performs the following steps:
 * 1. Sends the "Erase Sector" command to the Flash memory.
 * 2. Sends the 3-byte address of the sector to be erased.
 * 3. Deselects the Flash memory after the erase operation is requested.
 *
 * The Flash memory typically supports sector erasure (e.g., 4KB sectors), and this function erases the data
 * in the sector at the given address.
 *
 * @param address The starting address of the sector in the Flash memory to be erased.
 *
 * @note This function uses SPI to communicate with the Flash memory and assumes that the write enable
 *       operation has been performed beforehand via the `gd25q16etigr_writeEnable()` function.
 */
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
}

/**
 * @brief Initialize the GD25Q16ETIGR Flash memory interface.
 *
 * This function initializes the interface for the GD25Q16ETIGR Flash memory by setting the necessary
 * control pins to their default states. Specifically, it performs the following operations:
 * 1. Sets the nCS (chip select) pin to high, deselecting the Flash memory.
 * 2. Sets the nWP (write protect) pin to high, enabling write operations.
 *
 * This function should be called to ensure that the Flash memory is in a ready state for normal
 * operations such as read, write, and erase.
 */
void gd25q16etigr_init(void)
{
	nCS_Pin_Write(GPIO_PIN_SET);
	nWP_Pin_Write(GPIO_PIN_SET);
}

/**
 * @brief Deinitialize the GD25Q16ETIGR Flash memory interface.
 *
 * This function deinitializes the interface for the GD25Q16ETIGR Flash memory by setting the
 * control pins to their default state. Specifically, it performs the following operation:
 * 1. Sets the nWP (write protect) pin to low, disabling write operations to the Flash memory.
 *
 * This function should be called to disable write protection and return the system to a state
 * where it no longer interacts with the Flash memory.
 */
void gd25q16etigr_deInit(void)
{
	nWP_Pin_Write(GPIO_PIN_RESET);
}

