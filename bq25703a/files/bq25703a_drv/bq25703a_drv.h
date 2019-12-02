/**
*  @file      bq25703a_drv.h
*  @brief     bq25703a i2c drv
*  @author    Zack Li
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



#define CHARGE_VOL_1    11  // 11008 mv
#define CHARGE_VOL_2    12  // 12000 mv
#define CHARGE_VOL_3    13  // 13008 mv
#define CHARGE_CUR_1    1   // 1024 mA 
#define CHARGE_CUR_2    2   // 2048 mA
#define CHARGE_CUR_3    3   // 3008 mA


#define OTG_VOL_1   5   // 4992 mv
#define OTG_VOL_2   9   // 9024 mv
#define OTG_VOL_3   15  // 15040 mv
#define OTG_CUR_1   1   // 1024 mA
#define OTG_CUR_2   2   //  2048 mA
#define OTG_CUR_3   3   // 3008 mA

//do_what
#define CHARGE_FUNCTION 0
#define OTG_FUNCTION    1

#define CHG_OK_PIN      36

//REGISTER_DDR
#define CHARGE_OPTION_0_WR                              0x00
#define CHARGE_CURRENT_REGISTER_WR                      0x02
#define CHARGE_VOLTAGE_REGISTER_WR                      0x04
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


uint16_t CHARGE_REGISTER_DDR_VALUE_BUF[]=
{
    /*0*/   	CHARGE_OPTION_0_WR,         0x020E,
    /*2*/   	CHARGE_CURRENT_REGISTER_WR, 0x0800,
    /*4*/   	CHARGE_VOLTAGE_REGISTER_WR, 0x3070,
    /*6*/       OTG_VOLTAGE_REGISTER_WR,    0x0000,
    /*8*/       OTG_CURRENT_REGISTER_WR,    0x0000,
    /*10*/      INPUT_VOLTAGE_REGISTER_WR,  0x0000,
    /*12*/      MINIMUM_SYSTEM_VOLTAGE_WR,  0x1800,
    /*14*/      INPUT_CURRENT_REGISTER_WR,  0x4100,
    /*16*/      CHARGE_OPTION_1_WR,         0x0210,
    /*18*/      CHARGE_OPTION_2_WR,         0x02B7,
    /*20*/      CHARGE_OPTION_3_WR,         0x0000,
    /*22*/      PROCHOT_OPTION_0_WR,        0x4A54,
    /*24*/      PROCHOT_OPTION_1_WR,        0x8120,
    /*26*/      ADC_OPTION_WR,              0xE0FF
};

uint16_t OTG_REGISTER_DDR_VALUE_BUF[]=
{
    /*0*/       CHARGE_OPTION_0_WR,         0xE20E,
    /*2*/       CHARGE_CURRENT_REGISTER_WR, 0x0000,
    /*4*/       CHARGE_VOLTAGE_REGISTER_WR, 0x0000,
    /*6*/       OTG_VOLTAGE_REGISTER_WR,    0x0200,
    /*8*/       OTG_CURRENT_REGISTER_WR,    0x0A00,
    /*10*/      INPUT_VOLTAGE_REGISTER_WR,  0x0000,
    /*12*/      MINIMUM_SYSTEM_VOLTAGE_WR,  0x0000,
    /*14*/      INPUT_CURRENT_REGISTER_WR,  0x4100,
    /*16*/      CHARGE_OPTION_1_WR,         0x0211,
    /*18*/      CHARGE_OPTION_2_WR,         0x02B7,
    /*20*/      CHARGE_OPTION_3_WR,         0x1000,
    /*22*/      PROCHOT_OPTION_0_WR,        0x4A54,
    /*24*/      PROCHOT_OPTION_1_WR,        0x8120,
    /*26*/      ADC_OPTION_WR,              0xE0FF
};



// 参数结构体
struct thread_argument
{
    int pin_number;
    //int do_what;
    //int voltage;
    //int current_mA;
    //char string[30];
};




#endif

