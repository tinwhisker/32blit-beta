#include "i2c-user.h"

/**
 * Request data from an i2c slave.
 *
 * @param address Address of slave.
 * @param reg Register value.
 * @param result Reference to array to store result.
 * @param result_size Expected size of result.
 */
int i2c_receive(uint8_t address, uint8_t reg, uint8_t &result, uint8_t result_size){
  HAL_StatusTypeDef resultStatus = HAL_I2C_Master_Transmit(user_i2c_handle, address, &reg, 1, HAL_TIMEOUT);
  if (resultStatus != HAL_OK) return resultStatus;
  HAL_Delay(1); //Shorter delay?
  return HAL_I2C_Master_Receive(user_i2c_handle, address, &result, result_size, HAL_TIMEOUT);
}

/**
 * Send data to an i2c slave.
 *
 * @param address Address of slave.
 * @param reg Register value.
 * @param data Reference to array of data to send.
 * @param data_size Size of data to send.
 */
int i2c_send(uint8_t address, uint8_t reg, uint8_t &data, uint8_t data_size){
    HAL_StatusTypeDef resultStatus = HAL_I2C_Master_Transmit(user_i2c_handle, address, &reg, 1, HAL_TIMEOUT);
    if (resultStatus != HAL_OK) return resultStatus;
    return HAL_I2C_Master_Transmit(user_i2c_handle, address, &data, data_size, HAL_TIMEOUT);
}
  /*****************************END OF FILE****/
  