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

