/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "hwconfig.h"
#include "stm32f0xx.h"
/** @addtogroup STM32F0xx_StdPeriph_Templates
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
SPI_InitTypeDef    SPI_InitStructure;
GPIO_InitTypeDef   GPIO_InitStructure;
EXTI_InitTypeDef   EXTI_InitStructure;
NVIC_InitTypeDef   NVIC_InitStructure;
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
const uint8_t TX_ADDRESS[TX_ADR_WIDTH]={0x31,0x4E,0x6F,0x64,0x65}; 
const uint8_t RX_ADDRESS[RX_ADR_WIDTH]={0x31,0x4E,0x6F,0x64,0x65}; 
/* Private function prototypes -----------------------------------------------*/
static void SPI1_Config(void);
/* Private functions ---------------------------------------------------------*/
void NRF24L01_Init(void);
void NRF24L01_WriteReg(uint8_t address,uint8_t value);
void NRF24L01_Rx_Mode(void);
void NRF24L01_Tx_Mode(void);
void NRF24L01_TxPacket(uint8_t *txbuf);
void NRF24L01_Write_Buf(uint8_t reg ,uint8_t *pBuf ,uint8_t len);

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int main(void)
{
	const uint8_t ar[] = {0x31,0x32,0x33,0x34,0x35};
	SPI1_Config();
	SPI_Cmd(SPI1, ENABLE);
	NRF24L01_Init();
	//NRF24L01_Rx_Mode();
	NRF24L01_Tx_Mode();
  while (1)
  {
		NRF24L01_TxPacket((uint8_t*)ar);
		for(int i=0;i<10000;i++);
  }
}

void NRF24L01_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_AHBPeriph_GPIOA  ,ENABLE);
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_1 | GPIO_Pin_4 |GPIO_Pin_12 ;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT  ;   
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_3;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);  
	
	GPIO_SetBits(GPIOA,GPIO_Pin_4);
	GPIO_ResetBits(GPIOA,GPIO_Pin_12);
}

void NRF24L01_WriteReg(uint8_t address,uint8_t value)
{
	uint16_t Buffer = 0x0000;
	Buffer = (value << 8) + address ;
	GPIO_ResetBits(GPIOA,GPIO_Pin_4);
	SPI_I2S_SendData16(SPI1,Buffer);
	while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_BSY) == SET)
	{}
	GPIO_SetBits(GPIOA,GPIO_Pin_4);
}

void NRF24L01_Rx_Mode(void)
{
	GPIO_ResetBits(GPIOC,GPIO_Pin_4);
	NRF24L01_Write_Buf(NRF_WRITE_REG+RX_ADDR_P0,(uint8_t*)RX_ADDRESS,RX_ADR_WIDTH);
	
	NRF24L01_WriteReg(NRF_WRITE_REG+EN_AA,0x01);
	NRF24L01_WriteReg(NRF_WRITE_REG+EN_RXADDR,0x01);
	NRF24L01_WriteReg(NRF_WRITE_REG+RF_CH,83);
	NRF24L01_WriteReg(NRF_WRITE_REG+RX_PW_P0,RX_PLOAD_WIDTH);
	
	NRF24L01_WriteReg(NRF_WRITE_REG+RF_SETUP,0x0F);
	NRF24L01_WriteReg(NRF_WRITE_REG+CONFIG,0x0F);
	
	GPIO_SetBits(GPIOC,GPIO_Pin_4);
}

void NRF24L01_Tx_Mode(void)
{
	GPIO_ResetBits(GPIOA,GPIO_Pin_12);
	NRF24L01_Write_Buf(NRF_WRITE_REG+TX_ADDR,(uint8_t*)TX_ADDRESS,TX_ADR_WIDTH);
	NRF24L01_Write_Buf(NRF_WRITE_REG+RX_ADDR_P0,(uint8_t*)RX_ADDRESS,RX_ADR_WIDTH);
	
	NRF24L01_WriteReg(NRF_WRITE_REG+EN_AA,0x01);
	NRF24L01_WriteReg(NRF_WRITE_REG+EN_RXADDR,0x01);
	NRF24L01_WriteReg(NRF_WRITE_REG+SETUP_RETR,0x1A);
	NRF24L01_WriteReg(NRF_WRITE_REG+RF_CH,83);
	NRF24L01_WriteReg(NRF_WRITE_REG+RF_SETUP,0x0F);
	NRF24L01_WriteReg(NRF_WRITE_REG+CONFIG,0x0E);
	
	GPIO_SetBits(GPIOA,GPIO_Pin_12);
}

void NRF24L01_TxPacket(uint8_t *txbuf)
{
	GPIO_SetBits(GPIOA,GPIO_Pin_12);
	NRF24L01_Write_Buf(WR_TX_PLOAD,txbuf,TX_PLOAD_WIDTH);
	GPIO_ResetBits(GPIOA,GPIO_Pin_12);
}

void NRF24L01_Write_Buf(uint8_t address ,uint8_t *pBuf ,uint8_t len)
{    
  GPIO_ResetBits(GPIOA,GPIO_Pin_4);
	SPI_SendData8(SPI1,address);
	while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_BSY) == SET)
	{}
	for(int i = 0;i<len;i++)
	{
		SPI_SendData8(SPI1,*pBuf++);
		while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_BSY) == SET)
	  {}
	}
	GPIO_SetBits(GPIOA,GPIO_Pin_4);
} 


static void SPI1_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(SPIx_CLK, ENABLE);
	RCC_AHBPeriphClockCmd(SPIx_SCK_GPIO_CLK | SPIx_MISO_GPIO_CLK | SPIx_MOSI_GPIO_CLK , ENABLE);
	
	GPIO_PinAFConfig(SPIx_SCK_GPIO_PORT, SPIx_SCK_SOURCE, SPIx_SCK_AF);
	GPIO_PinAFConfig(SPIx_MOSI_GPIO_PORT, SPIx_MOSI_SOURCE, SPIx_MOSI_AF);
	GPIO_PinAFConfig(SPIx_MISO_GPIO_PORT, SPIx_MISO_SOURCE, SPIx_MISO_AF);
	
	GPIO_InitStructure.GPIO_Mode                 = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType                = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd                 = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed                = GPIO_Speed_Level_3;

	GPIO_InitStructure.GPIO_Pin                  = SPIx_SCK_PIN | SPIx_MOSI_PIN | SPIx_MISO_PIN;
  GPIO_Init(SPIx_SCK_GPIO_PORT, &GPIO_InitStructure);

  SPI_InitStructure.SPI_Direction              = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_DataSize               = SPI_DATASIZE;
  SPI_InitStructure.SPI_CPOL                   = SPI_CPOL_High;
  SPI_InitStructure.SPI_CPHA                   = SPI_CPHA_2Edge;
  SPI_InitStructure.SPI_NSS                    = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler      = SPI_BaudRatePrescaler_64;
	SPI_InitStructure.SPI_FirstBit               = SPI_FirstBit_MSB;
  /* Initializes the SPI communication */
  SPI_InitStructure.SPI_Mode                   = SPI_Mode_Master;
	//SPI_I2S_DeInit(SPI1);
  SPI_Init(SPIx, &SPI_InitStructure);
  
  /* Configure the SPI interrupt priority */
  NVIC_InitStructure.NVIC_IRQChannel           = SPIx_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPriority   = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd        = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

#ifdef  USE_FULL_ASSERT

void assert_failed(uint8_t* file, uint32_t line)
{ 

  while (1)
  {
  }
}
#endif

