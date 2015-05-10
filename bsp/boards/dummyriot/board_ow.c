#include "board_ow.h"
#include "uart.h"
#include "radiotimer.h"
#include "radio.h"
#include "debugpins.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

void board_init_ow(void)
{
    uart_init_ow();
    radio_init();
    radiotimer_init();
    debugpins_init();
}

void board_sleep(void)
{
}

void board_reset(void)
{
}
