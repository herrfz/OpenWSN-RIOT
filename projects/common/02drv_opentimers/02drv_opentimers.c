/**
\brief This is a program which shows how to use the "opentimers "driver module.

Since the driver modules for different platforms have the same declaration, you
can use this project with any platform.

This application allows you to verify the correct functioning of the opentimers
drivers. It starts 3 periodic timers, with periods APP_DLY_TIMER0_ms,
APP_DLY_TIMER1_ms and APP_DLY_TIMER2_ms. Each timer is attached an LED (error.
radio and sync). When you run the application, you should see the LEDs
"counting".

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2014.
*/

// bsp modules required
#include "board_ow.h"
#include "leds.h"
// driver modules required
#include "opentimers.h"

#include "riot.h"

//=========================== defines =========================================

#define APP_DLY_TIMER0_ms   15000

//=========================== variables =======================================

/*
typedef struct {
   
} app_vars_t;

app_vars_t app_vars;
*/

//=========================== prototypes ======================================

void cb_timer0(void);

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
void* mote_main(void* arg) {
   board_init();
   opentimers_init();
   
   opentimers_start(
      APP_DLY_TIMER0_ms,     // duration
      TIMER_PERIODIC,        // type
      TIME_MS,               // timetype
      cb_timer0              // callback
   );
   
   while(1) {
      thread_yield();
   }
}

//=========================== callbacks =======================================

void cb_timer0(void) {
   leds_sync_toggle();
}
