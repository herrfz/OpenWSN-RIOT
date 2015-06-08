#include "uart.h"
#include "leds.h"

#include "board.h"
#include "periph/uart.h"

#include <stdio.h>

//=========================== defines =========================================
#define BAUD            115200
//=========================== variables =======================================

typedef struct {
   uart_tx_cbt txCb;
   uart_rx_cbt rxCb;
   uint8_t     startOrend;
   uint8_t     flagByte;
   char        rxByte;
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
  if (uart_init(UART_0_DEV, BAUD, rx, tx, 0) >= 0) {
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
  UART_0_ENABLE_RXINTERRUPT;
}

void uart_disableInterrupts(void)
{
  UART_0_DISABLE_RXINTERRUPT;
  UART_0_DISABLE_TXINTERRUPT;
}

void uart_clearRxInterrupts(void)
{
  UART_0_CLEAR_RXFLAG;
}

void uart_clearTxInterrupts(void)
{
  UART_0_CLEAR_TXFLAG;
}

void uart_writeByte(uint8_t byteToWrite)
{
  uart_write(UART_0_DEV, (char)byteToWrite);
  //do nothing while uart stil transmitting
  //see $(RIOT)/boards/$(BOARD)/include/board.h
  while(UART_0_TXBUSY);

  //start or end byte?
  if(uart_vars.txByte == uart_vars.flagByte) {
    //flip startOrend
    uart_vars.startOrend = (uart_vars.startOrend == 0)?1:0;
    if(uart_vars.startOrend == 1) {
      //start byte
      uart_tx_begin(UART_0_DEV);
    }
  }
}

uint8_t uart_readByte(void)
{
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
  uart_tx_isr();
  // return 1 if there is still byte to tx, 0 otherwise
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
