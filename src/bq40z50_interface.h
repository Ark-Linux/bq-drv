/**
*  @file      bq40z50-drv.h
*  @brief     bq40z50
*  @author    Link Lin
*  @date      12 -2019
*  @copyright
*/

#ifndef _BQ40Z50_DRV_H_
#define _BQ40Z50_DRV_H_

#include<stdio.h>
#include<stdlib.h>


int i2c_open_fuelgauge(void);

int fuelgauge_get_Battery_Temperature(void);
int fuelgauge_get_Battery_Voltage(void);
int fuelgauge_get_Battery_Current(void);
int fuelgauge_get_RelativeStateOfCharge(void);
int fuelgauge_get_AbsoluteStateOfCharge(void);
int fuelgauge_get_Battery_ChargingCurrent(void);
int fuelgauge_get_Battery_ChargingVoltage(void);
int fuelgauge_get_BatteryStatus(void);
int fuelgauge_check_BatteryFullyCharged(void);

#endif
