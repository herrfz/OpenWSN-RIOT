#include "uart.h"
#include "leds.h"

#include "periph/uart.h"

#include <stdio.h>

//=========================== defines =========================================
#define DEV             UART_0
#define BAUD            115200
//=========================== variables =======================================

typedef struct {
   uart_tx_cbt txCb;
   uart_rx_cbt rxCb;
   uint8_t     startOrend;
   uint8_t     flagByte;
} uart_vars_t;

volatile uart_vars_t uart_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

void uart_init_ow(void)
{
  // reset local variables
  memset((void*)&uart_vars,0,sizeof(uart_vars_t));

  //when this value is 0, we are send the first data
  uart_vars.startOrend = 0;
  //flag byte for start byte and end byte
  uart_vars.flagByte = 0x7E;

  printf("Setting up UART @ %i", BAUD);
  if (uart_init_blocking(DEV, BAUD) >= 0) {
      puts("   ...ok");
  } else {
      puts("   ...failed");
      while(1);
  }
}

void uart_setCallbacks(uart_tx_cbt txCb, uart_rx_cbt rxCb)
{
  uart_vars.txCb = txCb;
  uart_vars.rxCb = rxCb;
}

void uart_enableInterrupts(void)
{
  // USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
}

void uart_disableInterrupts(void)
{
  // USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
  // USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
}

void uart_clearRxInterrupts(void)
{
  // USART_ClearFlag(USART1, USART_FLAG_RXNE);
}

void uart_clearTxInterrupts(void)
{
  // USART_ClearFlag(USART1, USART_FLAG_TXE);
}

void uart_writeByte(uint8_t byteToWrite)
{
  // USART_SendData(USART1, byteToWrite);
  // while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
  uart_write_blocking(DEV, byteToWrite);

  // //start or end byte?
  // if(byteToWrite == uart_vars.flagByte) {
  //   uart_vars.startOrend = (uart_vars.startOrend == 0)?1:0;
  //   //start byte
  //   if(uart_vars.startOrend == 1) {
  //     USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
  //   } else {
  //     USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
  //   }
  // }
}

uint8_t uart_readByte(void)
{
  char temp;
  // temp = USART_ReceiveData(USART1);
  uart_read_blocking(DEV, &temp);
  return (uint8_t)temp;
}

//=========================== interrupt handlers ==============================

kick_scheduler_t uart_tx_isr(void)
{
  uart_vars.txCb();
  return DO_NOT_KICK_SCHEDULER;
}

kick_scheduler_t uart_rx_isr(void)
{
  uart_vars.rxCb();
  return DO_NOT_KICK_SCHEDULER;
}
