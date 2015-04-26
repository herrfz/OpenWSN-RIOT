#include "board_ow.h"
#include "radio.h"
#include "board.h"
#include "dummyradio.h"
#include "dummyradio_spi.h"
#include "radiotimer.h"
#include "debugpins.h"
#include "leds.h"
#include "periph_conf.h"

#define ENABLE_DEBUG (0)
#include "debug.h"


//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
   radiotimer_capture_cbt    startFrame_cb;
   radiotimer_capture_cbt    endFrame_cb;
   radio_state_t             state;
} radio_vars_t;

radio_vars_t radio_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

//===== admin

void radio_init(void) {
   DEBUG("%s\n", __PRETTY_FUNCTION__);
   dummyradio_initialize(&dummyradio_netdev);
   dummyradio_set_monitor(1);
}

void radio_setOverflowCb(radiotimer_compare_cbt cb) {
   radiotimer_setOverflowCb(cb);
}

void radio_setCompareCb(radiotimer_compare_cbt cb) {
   radiotimer_setCompareCb(cb);
}

void radio_setStartFrameCb(radiotimer_capture_cbt cb) {
   radio_vars.startFrame_cb  = cb;
}

void radio_setEndFrameCb(radiotimer_capture_cbt cb) {
   radio_vars.endFrame_cb    = cb;
}

//===== reset

void radio_reset(void) {
   dummyradio_off();
   dummyradio_on();
}

//===== timer

void radio_startTimer(PORT_TIMER_WIDTH period) {
   radiotimer_start(period);
}

PORT_TIMER_WIDTH radio_getTimerValue(void) {
   return radiotimer_getValue();
}

void radio_setTimerPeriod(PORT_TIMER_WIDTH period) {
   radiotimer_setPeriod(period);
}

PORT_TIMER_WIDTH radio_getTimerPeriod(void) {
   return radiotimer_getPeriod();
}

//===== RF admin

void radio_setFrequency(uint8_t frequency) {
   // change state
   radio_vars.state = RADIOSTATE_SETTING_FREQUENCY;

   // configure the radio to the right frequecy
   dummyradio_set_channel(frequency);

   // change state
   radio_vars.state = RADIOSTATE_FREQUENCY_SET;
}

void radio_rfOn(void) {
   dummyradio_on();
}

void radio_rfOff(void) {
   leds_radio_off();
}

//===== TX

void radio_loadPacket(uint8_t* packet, uint8_t len) {
   DEBUG("rf load\n");
   // change state
   radio_vars.state = RADIOSTATE_LOADING_PACKET;

   // load packet in TXFIFO
   dummyradio_write_fifo(packet, len);

   // change state
   radio_vars.state = RADIOSTATE_PACKET_LOADED;
}

void radio_txEnable(void) {
   // change state
   radio_vars.state = RADIOSTATE_ENABLING_TX;
   DEBUG("rf tx en\n");

   leds_radio_on();

   /* Go to state PLL_ON */
   dummyradio_reg_write(DUMMYRADIO_REG__TRX_STATE, DUMMYRADIO_TRX_STATE__PLL_ON);

   // change state
   radio_vars.state = RADIOSTATE_TX_ENABLED;
}

void radio_txNow(void) {
   PORT_TIMER_WIDTH val;
   // change state
   radio_vars.state = RADIOSTATE_TRANSMITTING;
   leds_radio_toggle();

   dummyradio_transmit_tx_buf(&dummyradio_netdev);

   leds_radio_toggle();
   // AT86RF231 does not generate an interrupt when the radio transmits the
   // SFD, which messes up the MAC state machine. The danger is that, if we leave
   // this funtion like this, any radio watchdog timer will expire.
   // Instead, we cheat an mimick a start of frame event by calling
   // ieee154e_startOfFrame from here. This also means that software can never catch
   // a radio glitch by which #radio_txEnable would not be followed by a packet being
   // transmitted (I've never seen that).
   if (radio_vars.startFrame_cb!=NULL) {
      // call the callback
      dummyradio_reg_read(DUMMYRADIO_REG__IRQ_STATUS);
      val=radiotimer_getCapturedTime();
      radio_vars.startFrame_cb(val);
   }
   // DUMMYRADIO, being a dummy, doesn't generate end-of-frame interrupt either
   if (radio_vars.endFrame_cb!=NULL) {
      // call the callback
      dummyradio_reg_read(DUMMYRADIO_REG__IRQ_STATUS);
      val=radiotimer_getCapturedTime();
      radio_vars.endFrame_cb(val);
   }
   DEBUG("SENT");
}

//===== RX

void radio_rxEnable(void) {
   // change state
   radio_vars.state = RADIOSTATE_ENABLING_RX;

   // put radio in reception mode
   dummyradio_switch_to_rx();

   leds_radio_on();

   // change state
   radio_vars.state = RADIOSTATE_LISTENING;
}

void radio_rxNow(void) {
   // nothing to do

}

void radio_getReceivedFrame(uint8_t* pBufRead,
                            uint8_t* pLenRead,
                            uint8_t  maxBufLen,
                             int8_t* pRssi,
                            uint8_t* pLqi,
                               bool* pCrc) {
   uint8_t fcs_rssi = 0xfc;;

   //===== rssi crc
   /* dummy fcs and rssi */
   *pCrc                   = (fcs_rssi & 0x80)>>7;  // msb is whether packet passed CRC
   *pRssi                  = (fcs_rssi & 0x0f);

   //===== packet
   dummyradio_read_fifo(pLenRead, 1); // dummyradio_read_fifo returns fake data
   dummyradio_read_fifo(pBufRead, *pLenRead);
   *pLqi = pBufRead[(*pLenRead)-1];
}

//=========================== private =========================================

//=========================== callbacks =======================================

//=========================== interrupt handlers ==============================

void dummyradio_rx_irq(void) {
   PORT_TIMER_WIDTH capturedTime;
   uint8_t  irq_status;

   // capture the time
   capturedTime = radiotimer_getCapturedTime();

   // reading IRQ_STATUS causes radio's IRQ pin to go low
   irq_status = dummyradio_reg_read(DUMMYRADIO_REG__IRQ_STATUS);

   // start of frame event
   if (irq_status & DUMMYRADIO_IRQ_STATUS_MASK__RX_START) {
       DEBUG("Start of frame.\n");
      // change state
      radio_vars.state = RADIOSTATE_RECEIVING;
      if (radio_vars.startFrame_cb!=NULL) {
         // call the callback
         radio_vars.startFrame_cb(capturedTime);
         // kick the OS
         return;
      } else {
         while(1);
      }
   }
   // end of frame event
   if (irq_status & DUMMYRADIO_IRQ_STATUS_MASK__TRX_END) {
       DEBUG("End of Frame.\n");
      // change state
      radio_vars.state = RADIOSTATE_TXRX_DONE;
      if (radio_vars.endFrame_cb!=NULL) {
         // call the callback
         radio_vars.endFrame_cb(capturedTime);
         // kick the OS
         return;
      } else {
         while(1);
      }
   }
}
