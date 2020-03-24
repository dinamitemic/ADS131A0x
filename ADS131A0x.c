/*
 * ADS131A0x.c
 *
 *  Created on: Apr 25, 2014
 *      Author: a0406726
 */

#include "ADS131A0x.h"	// All other required source files are declared here



extern SPI_HandleTypeDef hspi2;
uint8_t emptyTxBuffer[ADS131A0x_WORD_SIZE*5] = {0};
uint8_t ADS131A0x_DataBuf[ADS131A0x_WORD_SIZE*5];
volatile uint8_t ADS131A0x_Ready_flag = 0;
uint8_t ADS131A0x_Init_Done = 0;
t_ADS131A0xData ExternalADCData[ADS131A0x_NUM_CHANNELS];


/*****begin low level functions*************************************************/
// merform hardware reset of ADC
void ADS131A0xReset(void) {
	HAL_GPIO_WritePin(GPIOD, ADC_RST__Pin, GPIO_PIN_RESET);
	HAL_Delay(5);
	HAL_GPIO_WritePin(GPIOD, ADC_RST__Pin, GPIO_PIN_SET);
	HAL_Delay(20);
}
// manage CS pin
void ADS131A0xSetCS(uint8_t state)
{
	if (0 == state) {
		HAL_GPIO_WritePin(ADS131A0x_CS_GPIO_Port, ADS131A0x_CS_Pin, GPIO_PIN_RESET);
	}
	else if (1 == state) {
		HAL_GPIO_WritePin(ADS131A0x_CS_GPIO_Port, ADS131A0x_CS_Pin, GPIO_PIN_SET);
	}
	else
		assert(0);		//Aborts program
}

// receive 32 bit, simultaneously send data
void ADS131A0xXferWord(uint8_t* txData, uint8_t* rxData) {
	ADS131A0xSetCS(0);
	while(hspi2.State == HAL_SPI_STATE_BUSY);
	HAL_SPI_TransmitReceive(&hspi2, txData, rxData, ADS131A0x_WORD_SIZE, 100);
	ADS131A0xSetCS(1);
}
uint16_t ADS131A0xSendCmd(uint16_t cmd) {

	uint8_t txData[ADS131A0x_WORD_SIZE] = {0};
	uint8_t zeros[ADS131A0x_WORD_SIZE] = {0};
	static uint8_t rxData[ADS131A0x_WORD_SIZE];
	uint16_t res = 0;
	//split 16bit cmd in 8bit array
	txData[1] = (cmd & 0xff);
	txData[0] = (cmd >> 8);
	//Send the command
	ADS131A0xXferWord(txData, rxData);
	//The response of the previous cmd is in the next response
	//So send another empty cmd to get the response
	ADS131A0xXferWord(zeros, rxData);
	//Collapse response
	res = (((uint16_t)rxData[0] << 8) | rxData[1]);

	//uint8_t ucBuf[128] = {};
	//snprintf((char*)ucBuf, sizeof(ucBuf), "CMD %lu Cmd: %x, Response: %x ;\r\n", osKernelSysTick(), cmd, res);
	//CDC_Transmit_FS(ucBuf, strlen((const char *)ucBuf));

	return res;

}
/*****end low level functions***************************************************/

/*****begin higher level functions***********************************************/



// read one register and return the result
uint16_t ADS131A0xReadRegister(uint8_t RegAddress) {

	static uint16_t value = 0;

	uint16_t word = ADS131A0x_CMD_RREG | (RegAddress << 8);

	value = ADS131A0xSendCmd(word);

	//uint8_t ucBuf[128] = {};
	//snprintf((char*)ucBuf, sizeof(ucBuf), "RREG %lu Addr: %x, Word: %x, Value: %x ;\r\n", osKernelSysTick(), RegAddress, word, value);
	//CDC_Transmit_FS(ucBuf, strlen((const char *)ucBuf));

	return value;
}
//write single register function
uint16_t ADS131A0xWriteRegister(uint8_t addr, uint8_t data) {

	uint16_t word = ADS131A0x_CMD_WREG | (addr<<8) | data;

	uint16_t value = ADS131A0xSendCmd(word);

	//uint8_t ucBuf[128] = {};
	//snprintf((char*)ucBuf, sizeof(ucBuf), "WREG %lu Addr: %x,Value: %x, ret_val: %x;\r\n", osKernelSysTick(), addr, data, value);
	//CDC_Transmit_FS(ucBuf, strlen((const char *)ucBuf));

	return value;

}
// write a number of consecutive registers from a given array pointer
void ADS131A0xWriteMultiRegister(uint8_t addr, int NumRegs, uint8_t* pdata) {
	//TODO: implement function
}

void ADS131A0xInit(void){

	uint16_t res_unlocked;

	ADS131A0xReset();

	uint16_t status = ADS131A0xSendCmd(0x0000);

	//device ready
	if(status == 0xff04) {
		//Unlock device
		res_unlocked = ADS131A0xSendCmd(ADS131A0x_CMD_UNLOCK);

		if(res_unlocked == ADS131A0x_CMD_UNLOCK) {
			//Read/write regs and check results
			ADS131A0xReadRegister(ADS131A0x_REG_A_SYS_CFG);

			ADS131A0xWriteRegister(ADS131A0x_REG_A_SYS_CFG, 0xE8);

			ADS131A0xReadRegister(ADS131A0x_REG_A_SYS_CFG);

			ADS131A0xWriteRegister(ADS131A0x_REG_CLK1, 0x02);

			ADS131A0xWriteRegister(ADS131A0x_REG_CLK2, 0x25);

			ADS131A0xWriteRegister(ADS131A0x_REG_ADC_ENA, 0x0F);

			ADS131A0xSendCmd(ADS131A0x_CMD_WAKEUP);

			ADS131A0xSendCmd(ADS131A0x_CMD_LOCK);
		}

	}
	else {
		//TODO: report error
	}


}
t_ADS131A0xData ADS131A0xGetChannels(void){
	t_ADS131A0xData ch;

	ch.Status = ADS131A0x_DataBuf[0]<<8 | ADS131A0x_DataBuf[1];
	ch.Ch1 = (ADS131A0x_DataBuf[4] << 16) | (ADS131A0x_DataBuf[5] << 8) | (ADS131A0x_DataBuf[6]);
	ch.Ch2 = (ADS131A0x_DataBuf[8] << 16) | (ADS131A0x_DataBuf[9] << 8) | (ADS131A0x_DataBuf[10]);
	ch.Ch3 = (ADS131A0x_DataBuf[12] << 16) | (ADS131A0x_DataBuf[13] << 8) | (ADS131A0x_DataBuf[14]);
	ch.Ch4 = (ADS131A0x_DataBuf[16] << 16) | (ADS131A0x_DataBuf[17] << 8) | (ADS131A0x_DataBuf[18]);

	//uint8_t ucBuf[128] = {};
	//snprintf((char*)ucBuf, sizeof(ucBuf), "%lu, status: %x, ch1: %x, ch2: %x, ch3: %x, ch4: %x ;\r\n", osKernelSysTick(), ch.Status, ch.Ch1, ch.Ch2, ch.Ch3, ch.Ch4);
	//CDC_Transmit_FS(ucBuf, strlen((const char *)ucBuf));

	return ch;
}
void StartExtADC(void) {
	ADS131A0xInit();
	ADS131A0x_Init_Done = 1;
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
	if(hspi -> Instance == SPI2){
		// Nothing to do here
	}
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if(GPIO_Pin == ADC_DRDY__Pin && ADS131A0x_Init_Done == 1) {
		ADS131A0x_Ready_flag = 1;
		ADS131A0xSetCS(0);
		HAL_SPI_TransmitReceive(&hspi2, &emptyTxBuffer[0], &ADS131A0x_DataBuf[0], ADS131A0x_WORD_SIZE*5, 100);
		ADS131A0xSetCS(1);
	}
}

/*****end higher level functions*************************************************/
