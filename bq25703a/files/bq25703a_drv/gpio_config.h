/**
*  @file      gpio_config.h
*  @brief     gpio_config 
*  @author    Zack Li
*  @date      11 -2019
*  @copyright
*/

#ifndef _GPIO_CONFIG_H_
#define _GPIO_CONFIG_H_

#include <stdio.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <error.h>
#include<string.h>
#include<stdlib.h>
#include <poll.h>
#include "bq25703a_drv.h"

int get_irq_gpiox(int pin_number, int do_what, int voltage, int current_mA);

int set_gpiox_high(int pin_number);
int set_gpiox_low(int pin_number);

int register_gpiox(int pin_number);
int unregister_gpiox(int pin_number);
int set_direction(int pin_number, char *direction);
int set_edge(int pin_number, char *edge);
int set_value(int pin_number, int value);


#endif







