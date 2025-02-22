/*
 * gd25q16etigr.h
 *
 *  Created on: Feb 5, 2025
 *      Author: uriel Zazueta
 */

#ifndef GD25Q16ETIGR_GD25Q16ETIGR_H_
#define GD25Q16ETIGR_GD25Q16ETIGR_H_

void gd25q16etigr_init(void);
void gd25q16etigr_deInit(void);
void gd25q16etigr_eraseSector(uint32_t address);
void gd25q16etigr_pageProgram(uint32_t address, uint8_t * buff, uint32_t len);
void gd25q16etigr_readDataBytes(uint32_t address, uint8_t * buff, uint32_t len);


#endif /* GD25Q16ETIGR_GD25Q16ETIGR_H_ */
