/*
 * it8951.c
 *
 *  Created on: Dec 21, 2025
 *      Author: dhyey
 */

#include "it8951.h"

void waitForHRDY(GPIO_pin* hrdy_pin) {
	while (HAL_GPIO_ReadPin(hrdy_pin->pin_type, hrdy_pin->pin_number) != 1){
			// wait for I80 bus ready
	}
}

int LCDWriteCmdCode(it8951_spi* eink_pins, uint16_t cmd_num) {
	//	1. Send Preamble 0x6000 for Write command code
	waitForHRDY(eink_pins->HRDY_N);
	HAL_GPIO_WritePin(eink_pins->CS_N->pin_type, eink_pins->CS_N->pin_number, 0);
	uint8_t write_cmd_preamble[2] = {0x60, 0x00};
	HAL_SPI_Transmit(eink_pins->spi_handler, (uint8_t*)write_cmd_preamble, 2, 1000);

	//	2. Send I80 Command code
	waitForHRDY(eink_pins->HRDY_N);
	uint8_t command[2] = {(cmd_num >> 8) & 0xFF, cmd_num & 0xFF};
	HAL_SPI_Transmit(eink_pins->spi_handler, (uint8_t*)command, 2, 1000);
	HAL_GPIO_WritePin(eink_pins->CS_N->pin_type, eink_pins->CS_N->pin_number, 1);

	return 0;
}

int LCDWriteData(it8951_spi* eink_pins, uint16_t data) {
	//	1. Send Preamble 0x0000 for write data
	waitForHRDY(eink_pins->HRDY_N);
	HAL_GPIO_WritePin(eink_pins->CS_N->pin_type, eink_pins->CS_N->pin_number, 0);
	uint8_t write_cmd_preamble[2] = {0x00, 0x00};
	HAL_SPI_Transmit(eink_pins->spi_handler, (uint8_t*)write_cmd_preamble, 2, 1000);

	//	2. Send 1 Word Data
	waitForHRDY(eink_pins->HRDY_N);
	uint8_t data_chunks[2] = {(data >> 8) & 0xFF, data & 0xFF};
	HAL_SPI_Transmit(eink_pins->spi_handler, (uint8_t*)data_chunks, 2, 1000);
	HAL_GPIO_WritePin(eink_pins->CS_N->pin_type, eink_pins->CS_N->pin_number, 1);

	return 0;
}

// not recommended, max can send 2048 bytes = 1024 "words" 0x0000 at a time
int LCDWriteNData(it8951_spi* eink_pins, uint8_t* data_buffer, uint16_t buffer_word_count) {
	if (buffer_word_count > 1024) {
		return -1;
	}

	//	1. Send Preamble 0x0000 for write data
	waitForHRDY(eink_pins->HRDY_N);
	HAL_GPIO_WritePin(eink_pins->CS_N->pin_type, eink_pins->CS_N->pin_number, 0);
	uint8_t write_cmd_preamble[2] = {0x00, 0x00};
	HAL_SPI_Transmit(eink_pins->spi_handler, (uint8_t*)write_cmd_preamble, 2, 1000);

	// 2. send N words of data
	waitForHRDY(eink_pins->HRDY_N);
	HAL_SPI_Transmit(eink_pins->spi_handler, (uint8_t*)data_buffer, 2*buffer_word_count, 1000);
	HAL_GPIO_WritePin(eink_pins->CS_N->pin_type, eink_pins->CS_N->pin_number, 1);

	return 0;
}

int LCDReadData(it8951_spi* eink_pins, uint8_t* read_buffer) {
	// 1. Send Preamble 0x1000 for read data
	waitForHRDY(eink_pins->HRDY_N);
	HAL_GPIO_WritePin(eink_pins->CS_N->pin_type, eink_pins->CS_N->pin_number, 0);
	uint8_t write_cmd_preamble[2] = {0x10, 0x00};
	HAL_SPI_Transmit(eink_pins->spi_handler, (uint8_t*)write_cmd_preamble, 2, 1000);

	// 2. Read 1 Dummy Word first
	waitForHRDY(eink_pins->HRDY_N);
	uint8_t dummy_read_buffer[2];
	HAL_SPI_Receive(eink_pins->spi_handler, dummy_read_buffer, 2, 1000);

	// 3. Get 1 Word Read Data
	waitForHRDY(eink_pins->HRDY_N);
	HAL_SPI_Receive(eink_pins->spi_handler, read_buffer, 2, 1000);
	HAL_GPIO_WritePin(eink_pins->CS_N->pin_type, eink_pins->CS_N->pin_number, 1);

	return 0;
}

// max can send 2048 bytes = 1024 "words" 0x0000 at a time
int LCDReadNData(it8951_spi* eink_pins, uint8_t* read_buffer, uint16_t read_buffer_word_size) {
	if (read_buffer_word_size > 1024) {
		return -1;
	}

	// 1. Send Preamble 0x1000 for read data
	waitForHRDY(eink_pins->HRDY_N);
	HAL_GPIO_WritePin(eink_pins->CS_N->pin_type, eink_pins->CS_N->pin_number, 0);
	uint8_t write_cmd_preamble[2] = {0x10, 0x00};
	HAL_SPI_Transmit(eink_pins->spi_handler, (uint8_t*)write_cmd_preamble, 2, 1000);

	// 2. Read 1 Dummy Word first
	waitForHRDY(eink_pins->HRDY_N);
	uint8_t dummy_read_buffer[2];
	HAL_SPI_Receive(eink_pins->spi_handler, dummy_read_buffer, 2, 1000);

	// 3. get n words of data
	for (int i=0; i<2*read_buffer_word_size; i+=2){
		waitForHRDY(eink_pins->HRDY_N);
		HAL_SPI_Receive(eink_pins->spi_handler, read_buffer+i, 2, 1000);
	}
	HAL_GPIO_WritePin(eink_pins->CS_N->pin_type, eink_pins->CS_N->pin_number, 1);

	return 0;
}

int getDeviceInfo(it8951_spi* eink_pins, eink_device_info* device_info_buff) {
	LCDWriteCmdCode(eink_pins, USDEF_I80_CMD_GET_DEV_INFO);

	uint16_t wordsReturned = 20;
	uint8_t temp_device_info_arr[40];
	LCDReadNData(eink_pins, temp_device_info_arr, wordsReturned);

	device_info_buff->panel_width = ( (temp_device_info_arr[0] << 8) | temp_device_info_arr[1] );
	device_info_buff->panel_height = ( (temp_device_info_arr[2] << 8) | temp_device_info_arr[3] );
	device_info_buff->image_buffer_addr = (temp_device_info_arr[6] << 24) | (temp_device_info_arr[7] << 16) | (temp_device_info_arr[4] << 8) | temp_device_info_arr[5];

	for (int i=0; i<16; i++) {
		device_info_buff->firmware_version[i] = temp_device_info_arr[i+8];
		device_info_buff->LUT_version[i] = temp_device_info_arr[i+24];
	}

	return 0;
}

int it8951_init(it8951_spi* eink_pins, eink_device_info* device_info_buff) {
	HAL_GPIO_WritePin(eink_pins->RST_N->pin_type, eink_pins->RST_N->pin_number, 1);

	//Set to Enable I80 Packed mode
	writeReg(eink_pins, I80CPCR, 0x0001);

	getDeviceInfo(eink_pins, device_info_buff);
}

int readReg(it8951_spi* eink_pins, uint16_t reg_addr, uint8_t* reg_data) {
	LCDWriteCmdCode(eink_pins, IT8951_TCON_REG_RD);
	LCDWriteData(eink_pins, reg_addr);
	LCDReadData(eink_pins, reg_data);

	return 0;
}

int writeReg(it8951_spi* eink_pins, uint16_t reg_addr, uint16_t data) {
	LCDWriteCmdCode(eink_pins, IT8951_TCON_REG_WR);
	LCDWriteData(eink_pins, reg_addr);
	LCDWriteData(eink_pins, data);

	return 0;
}

// LISAR = “Load Image Start Address Register”
// "When I send image data next, where in YOUR internal RAM should you store it?”
int setImgBufBaseAddr(it8951_spi* eink_pins, uint32_t img_buffer_addr) {
	uint16_t img_buffer_addr_H = (img_buffer_addr >> 16) & 0xFFFF;
	uint16_t img_buffer_addr_L = img_buffer_addr & 0xFFFF;
	writeReg(eink_pins, LISAR, img_buffer_addr_L);
	writeReg(eink_pins, LISAR + 2, img_buffer_addr_H);

	return 0;
}

int loadPartialImage(it8951_spi* eink_pins, eink_device_info* device_info_buff, uint8_t* pixels,
						uint16_t x_start_pos, uint16_t y_start_pos, uint16_t width, uint16_t height) {

	setImgBufBaseAddr(eink_pins, device_info_buff->image_buffer_addr);

	// There are 2 different commands for load image process start
	// 1. Load image full start command (0x20 LD_IMG)
	// 2. Load image area start command (0x21 LD_IMG_AREA)
	/* The partial picture area is denoted by a rectangle with a top left
	   pos(X0,Y0) and width , height of sub picture. */

	LCDWriteCmdCode(eink_pins, IT8951_TCON_LD_IMG_AREA);

	// 000110000
	uint16_t memory_converter_setting = (IT8951_LDIMG_L_ENDIAN << 9) + (IT8951_8BPP << 4) + IT8951_ROTATE_0;
	LCDWriteData(eink_pins, memory_converter_setting);

	LCDWriteData(eink_pins, x_start_pos);
	LCDWriteData(eink_pins, y_start_pos);
	LCDWriteData(eink_pins, width);
	LCDWriteData(eink_pins, height);

	for (int i=0; i<height; i++) {
		LCDWriteNData(eink_pins, pixels, width/2); // divide by 2 since each sends two bytes and 8bpp
		pixels += width;
	}

	LCDWriteCmdCode(eink_pins, IT8951_TCON_LD_IMG_END);

	return 0;
}

void pollLUTEngine(it8951_spi* eink_pins) {
	uint8_t LUT_status_buffer[2];
	uint16_t LUT_status = 1;
	while (LUT_status != 0) {
		// wait for LUT engine to be ready
		readReg(eink_pins, LUTAFSR, LUT_status_buffer);
		LUT_status = (LUT_status_buffer[0] << 8) | LUT_status_buffer[1];
	}
}

int displayPartialImage(it8951_spi* eink_pins,
						uint16_t x_start_pos, uint16_t y_start_pos, uint16_t width, uint16_t height,
						uint16_t display_mode){

	pollLUTEngine(eink_pins);

	LCDWriteCmdCode(eink_pins, USDEF_I80_CMD_DPY_AREA);
	LCDWriteData(eink_pins, x_start_pos);
	LCDWriteData(eink_pins, y_start_pos);
	LCDWriteData(eink_pins, width);
	LCDWriteData(eink_pins, height);
	LCDWriteData(eink_pins, display_mode);

	return 0;
}

int clearScreen(it8951_spi* eink_pins, eink_device_info* device_info_buff) {
	setImgBufBaseAddr(eink_pins, device_info_buff->image_buffer_addr);
	LCDWriteCmdCode(eink_pins, IT8951_TCON_LD_IMG);
	uint16_t memory_converter_setting = (IT8951_LDIMG_L_ENDIAN << 9) + (IT8951_8BPP << 4) + IT8951_ROTATE_0;
	LCDWriteData(eink_pins, memory_converter_setting);

	uint8_t white_arr[IT8951_PANEL_WIDTH];
	for (int i=0; i<IT8951_PANEL_WIDTH; i++) {
		white_arr[i] = 0xFF;
	}

	for (int i=0; i<IT8951_PANEL_HEIGHT; i++) {
		LCDWriteNData(eink_pins, white_arr, IT8951_PANEL_WIDTH/2); // divide by 2 since each sends two bytes and 8bpp
	}

	displayPartialImage(eink_pins, 0, 0, IT8951_PANEL_WIDTH, IT8951_PANEL_HEIGHT, IT8951_MODE_0);
	return 0;
}
