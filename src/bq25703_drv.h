/**
*  @file      bq25703a_drv.h
*  @brief     bq25703a i2c drv
*  @author    Zack Li and Link Lin
*  @date      11 -2019
*  @copyright
*/

#ifndef _BQ25703A_H_
#define _BQ25703A_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


#define CHARGE_CURRENT_1600mA               0x0640 //1600mA
#define CHARGE_CURRENT_832mA                0x0340 //832mA
#define CHARGE_CURRENT_384mA                0x0180 //384mA
#define CHARGE_CURRENT_0                    0x0000 //0mA


#define CHARGE_CURRENT_FOR_USB_Default      CHARGE_CURRENT_384mA
#define CHARGE_CURRENT_FOR_PD               CHARGE_CURRENT_1600mA


//12.592V for 3-cell
#define MAX_CHARGE_VOLTAGE  0x3130 //12592mV


#define INPUT_VOLTAGE_LIMIT_4V8   0x0640 //4800mV
#define INPUT_VOLTAGE_LIMIT_3V8   0x0280 //3840mV
#define INPUT_VOLTAGE_LIMIT_3V2   0x0000 //3200mV


#define CHG_OK_PIN      106


/*
* battery charge and discharge THRESHOLD,
* 2730 means 0°C
*/
//charge
#define BATTERY_CHARGE_STOP_TEMPERATURE_LOW_THRESHOLD               (2730)
#define BATTERY_CHARGE_ALLOW_TEMPERATURE_LOW_THRESHOLD           	(2730 + 50)

#define BATTERY_CHARGE_STOP_TEMPERATURE_HIGH_THRESHOLD              (2730 + 450)
#define BATTERY_CHARGE_ALLOW_TEMPERATURE_HIGH_THRESHOLD             (2730 + 400)


//discharge
#define BATTERY_DISCHARGE_STOP_TEMPERATURE_LOW_THRESHOLD            (2730 - 200)
#define BATTERY_DISCHARGE_ALLOW_TEMPERATURE_LOW_THRESHOLD           (2730 - 150)

#define BATTERY_DISCHARGE_STOP_TEMPERATURE_HIGH_THRESHOLD           (2730 + 600)
#define BATTERY_DISCHARGE_ALLOW_TEMPERATURE_HIGH_THRESHOLD          (2730 + 550)

#define BATTERY_DISCHARGE_ADJUST_EQ_TEMPERATURE_HIGH_THRESHOLD      (2730 + 500)
#define BATTERY_DISCHARGE_RECOVERY_EQ_TEMPERATURE_HIGH_THRESHOLD    (2730 + 450)



typedef struct
{
    unsigned int  Fault_OTG_UCP           :1;
    unsigned int  Fault_OTG_OVP           :1;
    unsigned int  Fault_Latchoff          :1;
    unsigned int  Reserved0               :1;
    unsigned int  SYSOVP_STAT             :1;
    unsigned int  Fault_ACOC              :1;
    unsigned int  Fault_BATOC             :1;
    unsigned int  Fault_ACOV              :1;

    unsigned int  IN_OTG                  :1;
    unsigned int  IN_PCHRG                :1;
    unsigned int  IN_FCHRG                :1;
    unsigned int  IN_IINDPM               :1;
    unsigned int  IN_VINDPM               :1;
    unsigned int  Reserved1               :1;
    unsigned int  ICO_DONE                :1;
    unsigned int  AC_STAT                 :1;

} s_BQ_Charger_Status;


int i2c_open_bq25703(void);


#endif

