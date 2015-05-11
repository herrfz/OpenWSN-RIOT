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
   char        rxByte;
   char        txByte;
} uart_vars_t;

volatile uart_vars_t uart_vars;

//=========================== prototypes ======================================
void rx(void *ptr, char data);
int tx(void *ptr);
//=========================== public ==========================================

void uart_init_ow(void)
{
  // reset local variables
  memset((void*)&uart_vars,0,sizeof(uart_vars_t));

  //when this value is 0, we are send the first data
  uart_vars.startOrend = 0;
  //flag byte for start byte and end byte
  uart_vars.flagByte = 0x7E;

  printf("Initializing UART @ %i", BAUD);
  if (uart_init(DEV, BAUD, rx, tx, 0) >= 0) {
      puts("   ...done");
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
  uart_vars.txByte = (char)byteToWrite;

  //start or end byte?
  if(uart_vars.txByte == uart_vars.flagByte) {
    uart_vars.startOrend = (uart_vars.startOrend == 0)?1:0;
    //start byte
    if(uart_vars.startOrend == 1) {
      uart_tx_begin(DEV);
    }
  }
}

uint8_t uart_readByte(void)
{
  // char temp;
  // temp = USART_ReceiveData(USART1);
  return (uint8_t)uart_vars.rxByte;
}

//=========================== interrupt handlers ==============================

void rx(void *ptr, char data)
{
  uart_vars.rxByte = data;
  uart_rx_isr();
}

int tx(void *ptr)
{
  /* return 1 if there is still byte to tx, 0 otherwise */
  uart_write(DEV, uart_vars.txByte);
  uart_tx_isr();
  return uart_vars.startOrend;
}

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
