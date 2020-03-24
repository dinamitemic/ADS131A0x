/*
 * ADS131A0x.h
 *
 *  Created on: 		Mar 17, 2020
 *  Last Updated on: 	xxx xx, xxxx
 *  Author: 			Michele Gazzarri
 *
 *  NOTES:
 *  	TODO: Add & correct comments
 *
 */

#ifndef ADS131A0x_H_
#define ADS131A0x_H_


// C Standard Libraries
#include <assert.h>
#include <stdint.h>
#include "cmsis_os.h"
#include "usbd_cdc_if.h"
#include "../stm32f7xx_hal.h"
#include "main.h"

#define ADS131A0x_LSB_VOLT	(2.442 / (1 << 24))

#define ADS131A0x_DRDY_Pin GPIO_PIN_11
#define ADS131A0x_CS_Pin GPIO_PIN_12
#define ADS131A0x_CS_GPIO_Port GPIOB

//this depends on how many bytes is composed a word
//in my case M1 tied to VDD so 32bits word
#define ADS131A0x_WORD_SIZE 					4

//number of words foreach data frame.
//|STATUS|ADC1|ADC2|ADC3|ADC4|CRC|
//in my case CRC is disabled so 5 words.
#define ADS131A0x_FRAME_NUM_WORDS 				5

// SELECT A DEVICE
//#define ADS131A02		//Standard definitions for both ADS131A042 and ADS131A04 devices
#define ADS131A04		//Additional definitions to support ADS131A04 additional features

typedef struct {
	int Ch1;
	int Ch2;
	int Ch3;
	int Ch4;
	uint16_t Status;
	uint16_t Checksum;
} t_ADS131A0xData;

//BEGIN ADC DEFINITIONS

#ifdef ADS131A02
	#define ADS131A0x_NUM_REG 					(0x00)			/* TODO: fix ADS131A02 has xx registers */
	#define ADS131A0x_NUM_CHANNELS 				2
#endif
#ifdef ADS131A04
	#define ADS131A0x_NUM_REG 					(0x00)			/* TODO: fix ADS131A04 has xx registers */
	#define ADS131A0x_NUM_CHANNELS 				4
#endif
//System Commands
#define ADS131A0x_CMD_NULL						(0x0000)
#define ADS131A0x_CMD_RESET						(0x0011)
#define ADS131A0x_CMD_STANDBY					(0x0022)
#define ADS131A0x_CMD_WAKEUP					(0x0033)
#define ADS131A0x_CMD_LOCK						(0x0555)
#define ADS131A0x_CMD_UNLOCK					(0x0655)

//Registers Read/Write Commands
#define ADS131A0x_CMD_RREG						(0x2000)
#define ADS131A0x_CMD_RREGS						(0x2000)
#define ADS131A0x_CMD_WREG						(0x4000)
#define ADS131A0x_CMD_WREGS						(0x6000)

//Read Only ID Registers
#define ADS131A0x_REG_ID_MSB					(0x00)
#define ADS131A0x_REG_ID_LSB					(0x01)
//Read Status Registers
#define ADS131A0x_REG_STAT_1					(0x02)
#define ADS131A0x_REG_STAT_P					(0x03)
#define ADS131A0x_REG_STAT_N					(0x04)
#define ADS131A0x_REG_STAT_S					(0x05)
#define ADS131A0x_REG_ERROR_CNT					(0x06)
#define ADS131A0x_REG_STAT_M2					(0x07)
//User Configuration Registers
#define ADS131A0x_REG_A_SYS_CFG					(0x0b)
#define ADS131A0x_REG_D_SYS_CFG					(0x0c)
#define ADS131A0x_REG_CLK1						(0x0d)
#define ADS131A0x_REG_CLK2						(0x0e)
#define ADS131A0x_REG_ADC_ENA					(0x0f)
#define ADS131A0x_REG_ADC1						(0x11)
#define ADS131A0x_REG_ADC2						(0x12)

/* Additional ADS131A04 Registers */
#ifdef ADS131A04

#define ADS131A0x_REG_ADC3						(0x13)
#define ADS131A0x_REG_ADC4						(0x14)

#endif /* ADS131A04 */


//END ADC DEFINITIONS


/* Function Prototypes */

// Low level
void ADS131A0xReset(void);
void ADS131A0xSetCS (uint8_t state);
void ADS131A0xXferWord(uint8_t* txData, uint8_t* rxData);
uint16_t ADS131A0xSendCmd(uint16_t cmd);

// Higher level

uint16_t ADS131A0xReadRegister(uint8_t RegAddress);
uint16_t ADS131A0xWriteRegister(uint8_t reg, uint8_t data);
void ADS131A0xWriteMultiRegister(uint8_t addr, int NumRegs, uint8_t* pdata);
t_ADS131A0xData ADS131A0xReadData(uint8_t NumBytes, uint8_t DataByteStartNum);
void ADS131A0xInit(void);
t_ADS131A0xData ADS131A0xGetChannels(void);
void StartExtADC(void);

#endif /* ADS131A0x_H_ */
