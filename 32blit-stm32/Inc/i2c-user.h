#ifndef __USERI2C_H__
#define __USERI2C_H__

#include "stm32h7xx_hal.h"

I2C_HandleTypeDef *user_i2c_handle;

#ifdef __cplusplus
extern "C" {
#endif

    extern int i2c_receiver(uint8_t address, uint8_t reg, uint8_t result[], uint8_t result_size);
    extern int i2c_sender(uint8_t address, uint8_t reg, uint8_t data[], uint8_t data_size);

#ifdef __cplusplus
}
#endif

#endif
/*****************************END OF FILE****/
