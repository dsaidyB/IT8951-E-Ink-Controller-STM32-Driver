/*
 * it8951.h
 *
 *  Created on: Dec 21, 2025
 *      Author: dhyey
 */

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_spi.h"
#include <stdint.h>


#ifndef INC_IT8951_H_
#define INC_IT8951_H_

// Built in I80 Command Code
#define IT8951_TCON_SYS_RUN 0x0001
#define IT8951_TCON_STANDBY 0x0002
#define IT8951_TCON_SLEEP 0x0003
#define IT8951_TCON_REG_RD 0x0010
#define IT8951_TCON_REG_WR 0x0011
#define IT8951_TCON_MEM_BST_RD _T 0x0012
#define IT8951_TCON_MEM_BST_RD_S 0x0013
#define IT8951_TCON_MEM_BST_WR 0x0014
#define IT8951_TCON_MEM_BST_END 0x0015
#define IT8951_TCON_LD_IMG 0x0020
#define IT8951_TCON_LD_IMG_AREA 0x0021
#define IT8951_TCON_LD_IMG_END 0x0022
#define USDEF_I80_CMD_LD_IMG_1BPP 0x0095

// I80 User defined command code
#define USDEF_I80_CMD_DPY_AREA 0x0034
#define USDEF_I80_CMD_GET_DEV_INFO 0x0302

//Memory Converter Registers
#define MCSR_BASE_ADDR 0x0200
#define MCSR (MCSR_BASE_ADDR + 0x0000)
#define LISAR (MCSR_BASE_ADDR + 0x0008)

//Panel
#define IT8951_PANEL_WIDTH 1448
#define IT8951_PANEL_HEIGHT 1072
#define cursor_size 16

//Rotate mode
#define IT8951_ROTATE_0 0
#define IT8951_ROTATE_90 1
#define IT8951_ROTATE_180 2
#define IT8951_ROTATE_270 3

//Pixel mode ,BPP - Bit per Pixel
#define IT8951_2BPP 0
#define IT8951_3BPP 1
#define IT8951_4BPP 2
#define IT8951_8BPP 3

//Waveform Mode
#define IT8951_MODE_0 0
#define IT8951_MODE_1 1
#define IT8951_MODE_2 2
#define IT8951_MODE_3 3
#define IT8951_MODE_4 4
//Mode name		Value		What it does						When to use
//INIT				0			Full reset waveform					Power-up only
//DU				1			Fast black/white					UI, buttons
//GC16				2			Full grayscale clean refresh		Images
//GL16				3			Gray update, slower					Photo updates
//A2				4			Fastest mono						Interactive UI


//Endian Type
#define IT8951_LDIMG_L_ENDIAN 0
#define IT8951_LDIMG_B_ENDIAN 1

//Auto LUT
#define IT8951_DIS_AUTO_LUT 0
#define IT8951_EN_AUTO_LUT 1
//LUT Engine Status
#define IT8951_ALL_LUTE_BUSY 0xFFFF

//-----------------------------------------------------------------------
// IT8951 TCon Registers defines
//-----------------------------------------------------------------------
//Register Base Address
#define DISPLAY_REG_BASE 0x1000 //Register RW access for I80 only
//Base Address of Basic LUT Registers
#define LUT0EWHR (DISPLAY_REG_BASE + 0x00) //LUT0 Engine Width Height Reg
#define LUT0XYR (DISPLAY_REG_BASE + 0x40) //LUT0 XY Reg
#define LUT0BADDR (DISPLAY_REG_BASE + 0x80) //LUT0 Base Address Reg
#define LUT0MFN (DISPLAY_REG_BASE + 0xC0) //LUT0 Mode and Frame number Reg
#define LUT01AF (DISPLAY_REG_BASE + 0x114) //LUT0 and LUT1 Active Flag Reg

//Update Parameter Setting Register
#define UP0SR (DISPLAY_REG_BASE + 0x134) //Update Parameter0 Setting Reg
#define UP1SR (DISPLAY_REG_BASE + 0x138) //Update Parameter1 Setting Reg
#define LUT0ABFRV (DISPLAY_REG_BASE + 0x13C) //LUT0 Alpha blend and Fill rectangle Value
#define UPBBADDR (DISPLAY_REG_BASE + 0x17C) //Update Buffer Base Address
#define LUT0IMXY (DISPLAY_REG_BASE + 0x180) //LUT0 Image buffer X/Y offset Reg
#define LUTAFSR (DISPLAY_REG_BASE + 0x224) //LUT Status Reg (status of All LUT Engines)
#define BGVR (DISPLAY_REG_BASE + 0x250) //Set BG and FG Color if Bitmap mode enable only

//-------System Registers----------------
#define SYS_REG_BASE 0x0000
//Address of System Registers
#define I80CPCR (SYS_REG_BASE + 0x04)

typedef struct {
	GPIO_TypeDef* pin_type;
	uint16_t pin_number;
} GPIO_pin;

// don't use NSS option in SPI handler. also, 8 bit width for each write/read transfer. CS_N and RST_N are GPIO_OUTPUT, HRDY_N is gpio input
typedef struct {
	SPI_HandleTypeDef* spi_handler;
	GPIO_pin* CS_N;
	GPIO_pin* HRDY_N;
	GPIO_pin* RST_N;
} it8951_spi;

typedef struct {
	uint16_t panel_width;
	uint16_t panel_height;
	uint32_t image_buffer_addr;
	uint8_t firmware_version[16];
	uint8_t LUT_version[16];
} eink_device_info;

// hmm maybe hide these and only give the init and write to reg functions in .h
int LCDWriteCmdCode(it8951_spi* eink_pins, uint16_t cmd_num);
int LCDWriteData(it8951_spi* eink_pins, uint16_t data);
int LCDWriteNData(it8951_spi* eink_pins, uint8_t* data_buffer, uint16_t buffer_word_count);
int LCDReadData(it8951_spi* eink_pins, uint8_t* read_buffer);
int LCDReadNData(it8951_spi* eink_pins, uint8_t* read_buffer, uint16_t read_buffer_word_size);

int getDeviceInfo(it8951_spi* eink_pins, eink_device_info* device_info_buff);
int it8951_init(it8951_spi* eink_pins, eink_device_info* device_info_buff);

int readReg(it8951_spi* eink_pins, uint16_t reg_addr, uint8_t* reg_data);
int writeReg(it8951_spi* eink_pins, uint16_t reg_addr, uint16_t data);

int setImgBufBaseAddr(it8951_spi* eink_pins, uint32_t img_buffer_addr);
int loadPartialImage(it8951_spi* eink_pins, eink_device_info* device_info_buff, uint8_t* pixels,
					uint16_t x_start_pos, uint16_t y_start_pos, uint16_t width, uint16_t height);

int displayPartialImage(it8951_spi* eink_pins,
						uint16_t x_start_pos, uint16_t y_start_pos, uint16_t width, uint16_t height,
						uint16_t display_mode);


int clearScreen(it8951_spi* eink_pins, eink_device_info* device_info_buff);

#endif /* INC_IT8951_H_ */
