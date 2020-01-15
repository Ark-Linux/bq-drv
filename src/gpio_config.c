/**
*  @file      gpio_config.c
*  @brief     gpio_config
*  @author    Zack Li
*  @date      11 -2019
*  @copyright
*/

#include "gpio_config.h"

int export_gpio(int pin_number)
{
    FILE *p=NULL;
    char *open_path = "/sys/class/gpio/export";
    p = fopen(open_path,"w");
    if (p == NULL)
    {
        printf("open '%s' error\n",open_path);
        return -1;
    }

    if (fprintf(p,"%d",pin_number) < 0)
    {
        printf("write '%s' error\n",open_path);
        return -1;
    }
    fclose(p);

    return 0;
}

int unexport_gpio(int pin_number)
{
    FILE *p=NULL;
    char *open_path = "/sys/class/gpio/unexport";
    p = fopen(open_path,"w");
    if (p == NULL)
    {
        printf("open '%s' error\n",open_path);
        return -1;
    }

    if (fprintf(p,"%d",pin_number) < 0)
    {
        printf("write '%s' error\n",open_path);
        return -1;
    }
    fclose(p);

    return 0;
}

int set_direction(int pin_number, char *direction)
{
    FILE *p=NULL;
    char open_path[64]= {0};

    sprintf(open_path, "/sys/class/gpio/gpio%d/direction",pin_number);
    p = fopen(open_path,"w");
    if (p == NULL)
    {
        printf("open '%s' error\n",open_path);
        return -1;
    }

    if (fprintf(p,"%s",direction) < 0)
    {
        printf("write '%s' error\n",open_path);
        return -1;
    }
    fclose(p);

    return 0;
}

int set_edge(int pin_number, char *edge)
{
    FILE *p=NULL;
    char open_path[64]= {0};

    sprintf(open_path, "/sys/class/gpio/gpio%d/edge",pin_number);
    p = fopen(open_path,"w");
    if (p == NULL)
    {
        printf("open '%s' error\n",open_path);
        return -1;
    }

    if (fprintf(p,"%s",edge) < 0)
    {
        printf("write '%s' error\n",open_path);
        return -1;
    }
    fclose(p);

    return 0;
}

int set_value(int pin_number, int value)
{
    FILE *p=NULL;
    char open_path[64]= {0};

    sprintf(open_path, "/sys/class/gpio/gpio%d/value", pin_number);
    p = fopen(open_path,"w");
    if (p == NULL)
    {
        printf("open '%s' error\n", open_path);
        return -1;
    }

    if (fprintf(p,"%d",value) < 0)
    {
        printf("write '%s' error\n", open_path);
        return -1;
    }
    fclose(p);

    return 0;
}


int set_battery_led(char color, int value)
{
    FILE *p=NULL;
    char open_path[64]= {0};

    sprintf(open_path, "/sys/class/leds/power_led_%c/brightness", color);
    p = fopen(open_path,"w");
    if (p == NULL)
    {
        printf("open '%s' error\n", open_path);
        return -1;
    }

    if (fprintf(p,"%d",value) < 0)
    {
        printf("write '%s' error\n", open_path);
        return -1;
    }
    fclose(p);

    return 0;
}



