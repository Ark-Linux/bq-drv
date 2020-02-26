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


#define Temperature_UNVALID     (-1000)


int i2c_open_fuelgauge(void);

void fuelgauge_read_FirmwareVersion(void);
void fuelgauge_read_Chemical_ID(void);

void fuelgauge_disable_communication(void);
void fuelgauge_enable_communication(void);
int fuelgauge_battery_enter_shutdown_mode(void);

void check_fuelgauge_iic_readErrCnt(void);

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
