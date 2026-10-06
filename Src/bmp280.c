/*
 * bmp280.c
 *
 *  Created on: 03-Oct-2026
 *      Author: LENOVO
 */


// bmp280.c
#include "bmp280.h"
#include "stm32f446xx.h"

uint8_t bmp280_read_chip_id(I2C_Handle_t *pI2CHandle)
{
    uint8_t reg_addr = BMP280_REG_CHIPID;
    uint8_t chip_id = 0;

    // Write the register address we want to read from, keep the bus held
    // with a repeated START instead of releasing it — this is the standard
    // "write register pointer, then read" I2C sequence
    I2C_MasterSendData(pI2CHandle, &reg_addr, 1, BMP280_I2C_ADDR, I2C_ENABLE_SR);

    // Now actually read the byte back, releasing the bus (STOP) afterward
    I2C_MasterReceiveData(pI2CHandle, &chip_id, 1, BMP280_I2C_ADDR, I2C_DISABLE_SR);

    return chip_id;
}



void bmp280_read_calibration(I2C_Handle_t *pI2CHandle, bmp280_calib_t *calib)
{
    uint8_t reg_addr = BMP280_REG_CALIB_START;
    uint8_t raw[BMP280_CALIB_LEN];

    I2C_MasterSendData(pI2CHandle, &reg_addr, 1, BMP280_I2C_ADDR, I2C_ENABLE_SR);
    I2C_MasterReceiveData(pI2CHandle, raw, BMP280_CALIB_LEN, BMP280_I2C_ADDR, I2C_DISABLE_SR);

    // Little-endian: low byte first, high byte second — combine into 16-bit values
    calib->dig_T1 = (uint16_t)(raw[1] << 8 | raw[0]);
    calib->dig_T2 = (int16_t)(raw[3] << 8 | raw[2]);
    calib->dig_T3 = (int16_t)(raw[5] << 8 | raw[4]);
    calib->dig_P1 = (uint16_t)(raw[7] << 8 | raw[6]);
    calib->dig_P2 = (int16_t)(raw[9] << 8 | raw[8]);
    calib->dig_P3 = (int16_t)(raw[11] << 8 | raw[10]);
    calib->dig_P4 = (int16_t)(raw[13] << 8 | raw[12]);
    calib->dig_P5 = (int16_t)(raw[15] << 8 | raw[14]);
    calib->dig_P6 = (int16_t)(raw[17] << 8 | raw[16]);
    calib->dig_P7 = (int16_t)(raw[19] << 8 | raw[18]);
    calib->dig_P8 = (int16_t)(raw[21] << 8 | raw[20]);
    calib->dig_P9 = (int16_t)(raw[23] << 8 | raw[22]);
}


void bmp280_set_mode(I2C_Handle_t *pI2CHandle)
{
    uint8_t data[2];
    data[0] = BMP280_REG_CTRL_MEAS;
    data[1] = 0x27;  // temp oversampling x1, pressure oversampling x1, normal mode
    I2C_MasterSendData(pI2CHandle, data, 2, BMP280_I2C_ADDR, I2C_DISABLE_SR);
}

void bmp280_read_raw(I2C_Handle_t *pI2CHandle, int32_t *raw_press, int32_t *raw_temp)
{
    uint8_t reg_addr = BMP280_REG_RAW_START;
    uint8_t raw[6];

    I2C_MasterSendData(pI2CHandle, &reg_addr, 1, BMP280_I2C_ADDR, I2C_ENABLE_SR);
    I2C_MasterReceiveData(pI2CHandle, raw, 6, BMP280_I2C_ADDR, I2C_DISABLE_SR);

    // Each value is 20 bits, packed across 3 bytes (MSB, LSB, XLSB) — top 4 bits of XLSB unused
    *raw_press = (int32_t)((raw[0] << 12) | (raw[1] << 4) | (raw[2] >> 4));
    *raw_temp  = (int32_t)((raw[3] << 12) | (raw[4] << 4) | (raw[5] >> 4));
}



//this is Bosch's official fixed-point formula, copied exactly from the datasheet
int32_t bmp280_compensate_temperature(bmp280_calib_t *calib, int32_t raw_temp, int32_t *t_fine)
{
    int32_t var1, var2, T;

    var1 = ((((raw_temp >> 3) - ((int32_t)calib->dig_T1 << 1))) * ((int32_t)calib->dig_T2)) >> 11;
    var2 = (((((raw_temp >> 4) - ((int32_t)calib->dig_T1)) * ((raw_temp >> 4) - ((int32_t)calib->dig_T1))) >> 12) * ((int32_t)calib->dig_T3)) >> 14;

    *t_fine = var1 + var2;
    T = (*t_fine * 5 + 128) >> 8;

    return T;   // temperature in units of 0.01°C — so 2534 means 25.34°C
}

uint32_t bmp280_compensate_pressure(bmp280_calib_t *calib, int32_t raw_press, int32_t t_fine)
{
    int64_t var1, var2, p;

    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)calib->dig_P6;
    var2 = var2 + ((var1 * (int64_t)calib->dig_P5) << 17);
    var2 = var2 + (((int64_t)calib->dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)calib->dig_P3) >> 8) + ((var1 * (int64_t)calib->dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)calib->dig_P1) >> 33;

    if (var1 == 0) {
        return 0;   // avoid divide-by-zero
    }

    p = 1048576 - raw_press;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)calib->dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)calib->dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)calib->dig_P7) << 4);

    return (uint32_t)p;   // pressure in Pa, as Q24.8 fixed point — divide by 256 for Pa
}
