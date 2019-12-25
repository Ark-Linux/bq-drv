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
#include <fcntl.h>
#include <error.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <pthread.h>
#include <poll.h>
#include <stdint.h>


#define CHARGE_CURRENT_1600mA               0x0640 //1600mA
#define CHARGE_CURRENT_832mA                0x0340 //832mA
#define CHARGE_CURRENT_384mA                0x0180 //384mA
#define CHARGE_CURRENT_0                    0x0000 //0mA


#define CHARGE_CURRENT_FOR_USB_Default      CHARGE_CURRENT_384mA
#define CHARGE_CURRENT_FOR_PD               CHARGE_CURRENT_1600mA


#define MAX_CHARGE_VOLTAGE  0x3130 //12592mV


#define INPUT_VOLTAGE_LIMIT_4V8   0x0640 //4800mV
#define INPUT_VOLTAGE_LIMIT_3V8   0x0280 //3840mV
#define INPUT_VOLTAGE_LIMIT_3V2   0x0000 //3200mV


#define CHG_OK_PIN      106


//REGISTER_DDR
#define CHARGE_OPTION_0_WR                              0x00
#define CHARGE_CURRENT_REGISTER_WR                      0x02
#define MaxChargeVoltage_REGISTER_WR                    0x04
#define OTG_VOLTAGE_REGISTER_WR                         0x06
#define OTG_CURRENT_REGISTER_WR                         0x08
#define INPUT_VOLTAGE_REGISTER_WR                       0x0A
#define MINIMUM_SYSTEM_VOLTAGE_WR                       0x0C
#define INPUT_CURRENT_REGISTER_WR                       0x0E
#define CHARGE_STATUS_REGISTER_R                        0x20
#define PROCHOT_STATUS_REGISTER_R                       0x22
#define INPUT_CURRENT_LIMIT_IN_USE_R                    0x24
#define VBUS_AND_PSYS_VOLTAGE_READ_BACK_R               0x26
#define CHARGE_AND_DISCHARGE_CURRENT_READ_BACK_R        0x28
#define INPUT_CURRENT_AND_CMPIN_VOLTAGE_READ_BACK_R     0x2A
#define SYSTEM_AND_BATTERY_VOLTAGE_READ_BACK_R          0x2C
#define MANUFACTURE_ID_AND_DEVICE_ID_READ_BACK_R        0x2E
#define DEVICE_ID_READ_BACK_R                           0x2F
#define CHARGE_OPTION_1_WR                              0x30
#define CHARGE_OPTION_2_WR                              0x32
#define CHARGE_OPTION_3_WR                              0x34
#define PROCHOT_OPTION_0_WR                             0x36
#define PROCHOT_OPTION_1_WR                             0x38
#define ADC_OPTION_WR                                   0x3A


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

