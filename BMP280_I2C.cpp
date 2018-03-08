/*
	BMP280.cpp
	
	Bosch BMP280 pressure sensor library for the Arduino microcontroller.
	This library uses I2C connection.

	Uses floating-point equations from BMP280 datasheet.

	modified by mhafuzul islam

	version 1.01		 16/9/2014 initial version
	
	Our example code uses the "pizza-eating" license. You can do anything
	you like with this code. No really, anything. If you find it useful,
	buy me italian pizza someday.
*/

#include "BMP280_I2C.h"
#include <Wire.h>
#include <stdio.h>
#include <math.h>


BMP280_I2C::BMP280_I2C(){
}

void BMP280_I2C::begin() {
	uint8_t osrs_t = 1;             //Temperature oversampling x 1
	uint8_t osrs_p = 1;             //Pressure oversampling x 1
	uint8_t osrs_h = 1;             //Humidity oversampling x 1
	uint8_t mode = 3;               //Normal mode
	uint8_t t_sb = 5;               //Tstandby 1000ms
	uint8_t filter = 0;             //Filter off
	uint8_t spi3w_en = 0;           //3-wire SPI Disable

	uint8_t ctrl_meas_reg = (osrs_t << 5) | (osrs_p << 2) | mode;
	uint8_t config_reg    = (t_sb << 5) | (filter << 2) | spi3w_en;
	uint8_t ctrl_hum_reg  = osrs_h;
	Wire.begin();

	writeReg(0xF2,ctrl_hum_reg);
	writeReg(0xF4,ctrl_meas_reg);
	writeReg(0xF5,config_reg);
	readTrim();
}

void BMP280_I2C::readTrim(){
	uint8_t data[32],i=0;
	Wire.beginTransmission(BME280_ADDRESS);
	Wire.write(0x88);
	Wire.endTransmission();
	Wire.requestFrom(BME280_ADDRESS,24);
	while(Wire.available()){
		data[i] = Wire.read();
		i++;
	}

	Wire.beginTransmission(BME280_ADDRESS);
	Wire.write(0xA1);
	Wire.endTransmission();
	Wire.requestFrom(BME280_ADDRESS,1);
	data[i] = Wire.read();
	i++;

	Wire.beginTransmission(BME280_ADDRESS);
	Wire.write(0xE1);
	Wire.endTransmission();
	Wire.requestFrom(BME280_ADDRESS,7);
	while(Wire.available()){
		data[i] = Wire.read();
		i++;
	}
	dig_T1 = (data[1] << 8) | data[0];
	dig_T2 = (data[3] << 8) | data[2];
	dig_T3 = (data[5] << 8) | data[4];
	dig_P1 = (data[7] << 8) | data[6];
	dig_P2 = (data[9] << 8) | data[8];
	dig_P3 = (data[11]<< 8) | data[10];
	dig_P4 = (data[13]<< 8) | data[12];
	dig_P5 = (data[15]<< 8) | data[14];
	dig_P6 = (data[17]<< 8) | data[16];
	dig_P7 = (data[19]<< 8) | data[18];
	dig_P8 = (data[21]<< 8) | data[20];
	dig_P9 = (data[23]<< 8) | data[22];
	dig_H1 = data[24];
	dig_H2 = (data[26]<< 8) | data[25];
	dig_H3 = data[27];
	dig_H4 = (data[28]<< 4) | (0x0F & data[29]);
	dig_H5 = (data[30] << 4) | ((data[29] >> 4) & 0x0F); // Fix 2014/04/06
	dig_H6 = data[31];
}

void BMP280_I2C::writeReg(uint8_t reg_address, uint8_t data)
{
	Wire.beginTransmission(BME280_ADDRESS);
	Wire.write(reg_address);
	Wire.write(data);
	Wire.endTransmission();
}

void BMP280_I2C::readData()
{
	int i = 0;
	uint32_t data[8];
	Wire.beginTransmission(BME280_ADDRESS);
	Wire.write(0xF7);
	Wire.endTransmission();
	Wire.requestFrom(BME280_ADDRESS,8);
	while(Wire.available()){
		data[i] = Wire.read();
		i++;
	}
	pres_raw = (data[0] << 12) | (data[1] << 4) | (data[2] >> 4);
	temp_raw = (data[3] << 12) | (data[4] << 4) | (data[5] >> 4);
	hum_raw  = (data[6] << 8) | data[7];
}

signed long int BMP280_I2C::calibration_T()
{
	signed long int var1, var2, T;
	signed long int adc_T = temp_raw;
	var1 = ((((adc_T >> 3) - ((signed long int)dig_T1<<1))) * ((signed long int)dig_T2)) >> 11;
	var2 = (((((adc_T >> 4) - ((signed long int)dig_T1)) * ((adc_T>>4) - ((signed long int)dig_T1))) >> 12) * ((signed long int)dig_T3)) >> 14;

	t_fine = var1 + var2;
	T = (t_fine * 5 + 128) >> 8;
	return T;
}

unsigned long int BMP280_I2C::calibration_P()
{
	signed long int var1, var2;
	unsigned long int P;
	signed long int adc_P = pres_raw;
	var1 = (((signed long int)t_fine)>>1) - (signed long int)64000;
	var2 = (((var1>>2) * (var1>>2)) >> 11) * ((signed long int)dig_P6);
	var2 = var2 + ((var1*((signed long int)dig_P5))<<1);
	var2 = (var2>>2)+(((signed long int)dig_P4)<<16);
	var1 = (((dig_P3 * (((var1>>2)*(var1>>2)) >> 13)) >>3) + ((((signed long int)dig_P2) * var1)>>1))>>18;
	var1 = ((((32768+var1))*((signed long int)dig_P1))>>15);
	if (var1 == 0)
	{
		return 0;
	}
	P = (((unsigned long int)(((signed long int)1048576)-adc_P)-(var2>>12)))*3125;
	if(P<0x80000000)
	{
		P = (P << 1) / ((unsigned long int) var1);
	}
	else
	{
		P = (P / (unsigned long int)var1) * 2;
	}
	var1 = (((signed long int)dig_P9) * ((signed long int)(((P>>3) * (P>>3))>>13)))>>12;
	var2 = (((signed long int)(P>>2)) * ((signed long int)dig_P8))>>13;
	P = (unsigned long int)((signed long int)P + ((var1 + var2 + dig_P7) >> 4));
	return P;
}

unsigned long int BMP280_I2C::calibration_H()
{
	signed long int v_x1;
	signed long int adc_H = hum_raw;

	v_x1 = (t_fine - ((signed long int)76800));
	v_x1 = (((((adc_H << 14) -(((signed long int)dig_H4) << 20) - (((signed long int)dig_H5) * v_x1)) +
			  ((signed long int)16384)) >> 15) * (((((((v_x1 * ((signed long int)dig_H6)) >> 10) *
													  (((v_x1 * ((signed long int)dig_H3)) >> 11) + ((signed long int) 32768))) >> 10) + (( signed long int)2097152)) *
												   ((signed long int) dig_H2) + 8192) >> 14));
	v_x1 = (v_x1 - (((((v_x1 >> 15) * (v_x1 >> 15)) >> 7) * ((signed long int)dig_H1)) >> 4));
	v_x1 = (v_x1 < 0 ? 0 : v_x1);
	v_x1 = (v_x1 > 419430400 ? 419430400 : v_x1);
	return (unsigned long int)(v_x1 >> 12);
}