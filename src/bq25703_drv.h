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


#define CHARGE_CURRENT_1536mA               0x0600 //1536mA
#define CHARGE_CURRENT_496mA                0x01F0 //496mA
#define CHARGE_CURRENT_256mA                0x0100 //256mA
#define CHARGE_CURRENT_0                    0x0000 //0mA


#define CHARGE_CURRENT_FOR_USB_Default      CHARGE_CURRENT_256mA
#define CHARGE_CURRENT_FOR_PD               CHARGE_CURRENT_1536mA


#define CHARGE_CURRENT_LEVEL_1              CHARGE_CURRENT_256mA
#define CHARGE_CURRENT_LEVEL_2              CHARGE_CURRENT_496mA
#define CHARGE_CURRENT_LEVEL_3              CHARGE_CURRENT_1536mA


//12.592V for 3-cell
#define MAX_CHARGE_VOLTAGE  0x3130 //12592mV




#define INPUT_VOLTAGE_LIMIT_4V8   0x0640 //4800mV
#define INPUT_VOLTAGE_LIMIT_3V8   0x0280 //3840mV
#define INPUT_VOLTAGE_LIMIT_3V2   0x0000 //3200mV


#define CHG_OK_PIN      106


/*
* battery charge and discharge THRESHOLD,
* unit: °C
*/
//charge
#define BATTERY_CHARGE_STOP_TEMPERATURE_LOW_THRESHOLD               (0)
#define BATTERY_CHARGE_ALLOW_TEMPERATURE_LOW_THRESHOLD              (5)

#define BATTERY_CHARGE_STOP_TEMPERATURE_HIGH_THRESHOLD              (45)
#define BATTERY_CHARGE_ALLOW_TEMPERATURE_HIGH_THRESHOLD             (40)



//discharge
#define BATTERY_DISCHARGE_STOP_TEMPERATURE_LOW_THRESHOLD            (-20)
#define BATTERY_DISCHARGE_ALLOW_TEMPERATURE_LOW_THRESHOLD           (-15)

#define BATTERY_DISCHARGE_STOP_TEMPERATURE_HIGH_THRESHOLD           (60)
#define BATTERY_DISCHARGE_ALLOW_TEMPERATURE_HIGH_THRESHOLD          (55)

#define BATTERY_DISCHARGE_ADJUST_EQ_TEMPERATURE_HIGH_THRESHOLD      (50)
#define BATTERY_DISCHARGE_RECOVERY_EQ_TEMPERATURE_HIGH_THRESHOLD    (45)


/*
* decide the Charge Current THRESHOLD
* unit: °C
*/

//Battery charge TEMPERATURE THRESHOLD
#define BATTERY_LOW_TEMPERATURE_THRESHOLD                           (10)

//Battery Voltage THRESHOLD
#define BATTERY_LOW_VOLTAGE_THRESHOLD                               9300 //mv
#define BATTERY_MAX_VOLTAGE_THRESHOLD                               12600 //mv


//led display UI
typedef enum
{
    LED_BATTERY_INVALID_VALUE = 0,

    LED_BATTERY_OFF,
    LED_BATTERY_FULLY_CHARGED,
    LED_BATTERY_CHARGEING,
    LED_BATTERY_LOW,

} LED_BATTERY_DISPLAY_STATE;



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

