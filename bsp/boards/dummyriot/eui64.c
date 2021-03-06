#include "eui64.h"

//=========================== defines =========================================
// stm32f411 (nucleo), 96-bit unique ID address
#define UNIQUE_ID_BASE_ADDRESS          0x1FFF7A10

//=========================== variables =======================================

const uint8_t const *uid = (const uint8_t *const) UNIQUE_ID_BASE_ADDRESS;

//=========================== prototypes ======================================

//=========================== public ==========================================

void eui64_get(uint8_t* addressToWrite)
{
  addressToWrite[0] = uid[0];
  addressToWrite[1] = uid[1];
  addressToWrite[2] = uid[2];
  addressToWrite[3] = uid[3];
  addressToWrite[4] = uid[4];
  addressToWrite[5] = uid[5];
  addressToWrite[6] = uid[6];
  addressToWrite[7] = uid[7];
}
