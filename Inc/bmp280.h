/*
 * bmp280.h
 *
 *  Created on: 03-Oct-2026
 *      Author: LENOVO
 */




// bmp280.h
#ifndef BMP280_H_
#define BMP280_H_

#include "stm32f446xx_i2c_driver.h"

#define BMP280_I2C_ADDR     0x76   // 0x77 if your board's SDO pin is tied high instead of GND
#define BMP280_REG_CHIPID   0xD0   // should read back as 0x58

#define BMP280_REG_CALIB_START  0x88
#define BMP280_CALIB_LEN        24   // 12 values x 2 bytes each

#define BMP280_REG_CTRL_MEAS  0xF4
#define BMP280_REG_RAW_START  0xF7

typedef struct {
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;
    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;
} bmp280_calib_t;


uint8_t bmp280_read_chip_id(I2C_Handle_t *pI2CHandle);
void bmp280_read_calibration(I2C_Handle_t *pI2CHandle, bmp280_calib_t *calib);
void bmp280_set_mode(I2C_Handle_t *pI2CHandle);
void bmp280_read_raw(I2C_Handle_t *pI2CHandle, int32_t *raw_press, int32_t *raw_temp);
int32_t bmp280_compensate_temperature(bmp280_calib_t *calib, int32_t raw_temp, int32_t *t_fine);
uint32_t bmp280_compensate_pressure(bmp280_calib_t *calib, int32_t raw_press, int32_t t_fine);




#endif /* BMP280_H_ */
