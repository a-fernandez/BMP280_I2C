/*
	BMP280.h
	Bosch BMP280 pressure sensor library for the Arduino microcontroller.
	This library uses I2C connection.

	Uses floating-point equations from BMP280 datasheet.

	modified by Alvaro Fernandez

	version 0.01		 03/3/2017 initial version

*/

#ifndef BMP280_h
#define BMP280_h

#include "Arduino.h"

// #define _debugSerial
// #define _debugTestData
#define BME280_ADDRESS 0x76

class BMP280_I2C {
public:
	BMP280_I2C();
	void begin();
	void readData();
	signed long int calibration_T();
	unsigned long int calibration_P();
	unsigned long int calibration_H();

private:
	void readTrim();
	void writeReg(uint8_t reg_address, uint8_t data);

	unsigned long int hum_raw,temp_raw,pres_raw;
	signed long int t_fine;

	uint16_t dig_T1;
	int16_t dig_T2;
	int16_t dig_T3;
	uint16_t dig_P1;
	int16_t dig_P2;
	int16_t dig_P3;
	int16_t dig_P4;
	int16_t dig_P5;
	int16_t dig_P6;
	int16_t dig_P7;
	int16_t dig_P8;
	int16_t dig_P9;
	int8_t  dig_H1;
	int16_t dig_H2;
	int8_t  dig_H3;
	int16_t dig_H4;
	int16_t dig_H5;
	int8_t  dig_H6;
};


#endif
