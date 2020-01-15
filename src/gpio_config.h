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



int export_gpio(int pin_number);
int unexport_gpio(int pin_number);
int set_direction(int pin_number, char *direction);
int set_edge(int pin_number, char *edge);
int set_value(int pin_number, int value);
int set_battery_led(char color, int value);

#endif







