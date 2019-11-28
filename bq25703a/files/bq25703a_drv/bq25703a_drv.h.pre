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



#define CHARGE_VOL_1	11	// 11008 mv
#define CHARGE_VOL_2	12	// 12000 mv
#define CHARGE_VOL_3	13	// 13008 mv
#define CHARGE_CUR_1	1	// 1024 mA 
#define CHARGE_CUR_2	2	// 2048 mA
#define CHARGE_CUR_3	3	// 3008 mA


#define OTG_VOL_1	5	// 4992 mv
#define OTG_VOL_2	9	// 9024 mv
#define OTG_VOL_3	15	// 15040 mv
#define OTG_CUR_1	1	// 1024 mA
#define OTG_CUR_2	2	//  2048 mA
#define OTG_CUR_3	3	// 3008 mA

//do_what
#define CHARGE_FUNCTION 0
#define OTG_FUNCTION 	1

// 参数结构体 
struct thread_argument
{
  int pin_number;
  int do_what;
  int voltage;
  int current_mA;
  //char string[30];
};


void *gpiox_irq_thread_function(void *arg);

void little_to_big(short int val, unsigned char *res_val);

int i2c_open();
int i2c_write(unsigned char dev_addr, unsigned char *val, unsigned char len);
int i2c_read(unsigned char addr, unsigned char reg, unsigned char *val, unsigned char len);
int bq25703a_i2c_write(unsigned char dev_addr, unsigned char reg, unsigned char *val, unsigned char data_len);
int bq25703a_i2c_read(unsigned char addr, unsigned char reg, unsigned char *val, unsigned char len);
int bq25703a_charge_function_init(int voltage, int current_mA);
int set_charge_voltage_current(int voltage, int current_mA);
int bq25703a_otg_function_init(int voltage, int current_mA);
int set_otg_vol_current(int voltage, int current_mA);
int bq25703a_battery_system_vol_read( int bs_vol[] );
//int set_gpiox(int pin_number, char *direction, int value);

#endif 

