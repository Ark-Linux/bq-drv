/**
*  @file      bq25703_drv.c
*  @brief     bq25703_drv
*  @author    Zack Li and Link Lin
*  @date      11 -2019
*  @copyright
*/

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
#include <time.h>


#include "bq25703_drv.h"
#include "gpio_config.h"

#include "tps65987_interface.h"
#include "bq40z50_interface.h"


#define I2C_FILE_NAME   "/dev/i2c-2"
#define BQ_I2C_ADDR        0x6B

static int fd_i2c;

int fd_chg_ok_pin;
struct pollfd fds_chg_ok_pin[1];


FILE *fp_batt_temp;
int log_batt_temp_flag = 0;


//BQ25703 REGISTER_ADDR
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



#define     CHARGE_OPTION_0_SETTING         0x020E

#define     EN_LEARN                        0x0020


uint16_t CHARGE_REGISTER_DDR_VALUE_BUF[]=
{
    CHARGE_OPTION_0_WR,         CHARGE_OPTION_0_SETTING,
    CHARGE_CURRENT_REGISTER_WR, CHARGE_CURRENT_0,
    MaxChargeVoltage_REGISTER_WR, MAX_CHARGE_VOLTAGE,
    OTG_VOLTAGE_REGISTER_WR,    0x0000,
    OTG_CURRENT_REGISTER_WR,    0x0000,
    INPUT_VOLTAGE_REGISTER_WR,  INPUT_VOLTAGE_LIMIT_3V2, //here should use the default value:0x0000, means 3200mV
    MINIMUM_SYSTEM_VOLTAGE_WR,  0x2400, //The charger provides minimum system voltage, means 9216mV
    INPUT_CURRENT_REGISTER_WR,  0x4100,
    CHARGE_OPTION_1_WR,         0x0210,
    CHARGE_OPTION_2_WR,         0x02B7,
    CHARGE_OPTION_3_WR,         0x0000,
    PROCHOT_OPTION_0_WR,        0x4A54,
    PROCHOT_OPTION_1_WR,        0x8120,
    ADC_OPTION_WR,              0xE0FF
};

uint16_t OTG_REGISTER_DDR_VALUE_BUF[]=
{
    CHARGE_OPTION_0_WR,         0xE20E,
    CHARGE_CURRENT_REGISTER_WR, 0x0000,
    MaxChargeVoltage_REGISTER_WR, 0x0000,
    OTG_VOLTAGE_REGISTER_WR,    0x0200,
    OTG_CURRENT_REGISTER_WR,    0x0A00,
    INPUT_VOLTAGE_REGISTER_WR,  0x0000,
    MINIMUM_SYSTEM_VOLTAGE_WR,  0x0000,
    INPUT_CURRENT_REGISTER_WR,  0x4100,
    CHARGE_OPTION_1_WR,         0x0211,
    CHARGE_OPTION_2_WR,         0x02B7,
    CHARGE_OPTION_3_WR,         0x1000,
    PROCHOT_OPTION_0_WR,        0x4A54,
    PROCHOT_OPTION_1_WR,        0x8120,
    ADC_OPTION_WR,              0xE0FF
};


struct BATTERY_MANAAGE_PARA
{
    unsigned char battery_fully_charged;
    unsigned char need_charge_flag;

    unsigned char stop_charge_flag;
    unsigned char can_charge_flag;

    unsigned char adjust_eq_flag;

    unsigned char low_battery_flag;

    unsigned char battery_is_charging;

    unsigned char charger_is_plug_in;

    LED_BATTERY_DISPLAY_STATE led_battery_display_state;

    int battery_temperature;
    int battery_voltage;

} batteryManagePara;



int i2c_open_bq25703(void)
{
    int ret;
    int val;

    fd_i2c = open(I2C_FILE_NAME, O_RDWR);

    if(fd_i2c < 0)
    {
        perror("Unable to open bq25703 i2c control file");

        return -1;
    }

    printf("open bq25703 i2c file success,fd is %d\n",fd_i2c);

    ret = ioctl(fd_i2c, I2C_SLAVE_FORCE, BQ_I2C_ADDR);
    if (ret < 0)
    {
        perror("i2c: Failed to set i2c device address\n");
        return -1;
    }

    printf("i2c: set i2c device address success\n");

    val = 3;
    ret = ioctl(fd_i2c, I2C_RETRIES, val);
    if(ret < 0)
    {
        printf("i2c: set i2c retry times err\n");
    }

    printf("i2c: set i2c retry times %d\n",val);

    return 0;
}


static int i2c_write(unsigned char dev_addr, unsigned char *val, unsigned char len)
{
    int ret;
    int i;

    struct i2c_rdwr_ioctl_data data;

    struct i2c_msg messages;


    messages.addr = dev_addr;  //device address
    messages.flags = 0;    //write
    messages.len = len;
    messages.buf = val;  //data

    data.msgs = &messages;
    data.nmsgs = 1;

    if(ioctl(fd_i2c, I2C_RDWR, &data) < 0)
    {
        printf("write ioctl err %d\n",fd_i2c);
        return -1;
    }

    /*printf("i2c write buf = ");
    for(i=0; i< len; i++)
    {
        printf("%02x ",val[i]);
    }
    printf("\n");*/

    return 0;
}


static int i2c_read(unsigned char addr, unsigned char reg, unsigned char *val, unsigned char len)
{
    int ret;
    int i;

    struct i2c_rdwr_ioctl_data data;
    struct i2c_msg messages[2];

    messages[0].addr = addr;  //device address
    messages[0].flags = 0;    //write
    messages[0].len = 1;
    messages[0].buf = &reg;  //reg address

    messages[1].addr = addr;       //device address
    messages[1].flags = I2C_M_RD;  //read
    messages[1].len = len;
    messages[1].buf = val;

    data.msgs = messages;
    data.nmsgs = 2;

    if(ioctl(fd_i2c, I2C_RDWR, &data) < 0)
    {
        perror("---");
        printf("read ioctl err %d\n",fd_i2c);

        return -1;
    }

    /*printf("i2c read buf = ");
    for(i = 0; i < len; i++)
    {
        printf("%02x ",val[i]);
    }
    printf("\n");*/

    return 0;
}


static int bq25703a_i2c_write(unsigned char dev_addr, unsigned char reg, unsigned char *val, unsigned char data_len)
{
    unsigned char buf[80] = {0};
    int i;

    if(data_len + 1 >= 80)
    {
        printf("data_len_exceed\n");
        return 1;
    }

    buf[0] = reg;

    for(i = 0; i<data_len; i++)
    {
        buf[1+i] = val[i];
    }

    return i2c_write(dev_addr, buf, data_len+1);
}


static int bq25703a_i2c_read(unsigned char addr, unsigned char reg, unsigned char *val, unsigned char len)
{
    return i2c_read(addr, reg, val, len);
}



int bq25703a_otg_function_init()
{
    int i = 0;

    printf("OTG_REGISTER_DDR_VALUE_BUF:\n");
    for (i = 0; i < sizeof(OTG_REGISTER_DDR_VALUE_BUF)/sizeof(uint16_t); i = i + 2)
    {
        printf("%02x, %04x\n",OTG_REGISTER_DDR_VALUE_BUF[i],OTG_REGISTER_DDR_VALUE_BUF[i+1]);

        if(bq25703a_i2c_write(BQ_I2C_ADDR,OTG_REGISTER_DDR_VALUE_BUF[i],((unsigned char*)(&OTG_REGISTER_DDR_VALUE_BUF[i+1])),2) != 0)
        {
            printf("write reg %02x eer\n",OTG_REGISTER_DDR_VALUE_BUF[i]);
            return -1;
        }
    }

    printf("bq25703a OTG function init success");

    return 0;
}


/*
*   set otg voltage and current
*   voltage：
*   current_mA :
*/
int bq25703a_set_otg_vol_and_current()
{
    if(0 != bq25703a_i2c_write(
           BQ_I2C_ADDR,
           OTG_REGISTER_DDR_VALUE_BUF[2],
           ((unsigned char*)(&OTG_REGISTER_DDR_VALUE_BUF[3])),
           2)
      )
    {
        printf("write %d eer\n",OTG_REGISTER_DDR_VALUE_BUF[2]);
        return -1;
    }

    if(0 != bq25703a_i2c_write(
           BQ_I2C_ADDR,
           OTG_REGISTER_DDR_VALUE_BUF[4],
           ((unsigned char*)(&OTG_REGISTER_DDR_VALUE_BUF[5])),
           2)
      )
    {
        printf("write register addr %d eer\n",OTG_REGISTER_DDR_VALUE_BUF[4]);
        return -1;
    }
    return 0;
}


int bq25703a_charge_function_init()
{
    int i = 0;

    printf("CHARGE_REGISTER_DDR_VALUE_BUF:\n");
    for (i = 0; i < sizeof(CHARGE_REGISTER_DDR_VALUE_BUF)/sizeof(uint16_t); i = i + 2)
    {
        printf("%02x, %04x\n",CHARGE_REGISTER_DDR_VALUE_BUF[i],CHARGE_REGISTER_DDR_VALUE_BUF[i+1]);

        if(bq25703a_i2c_write(BQ_I2C_ADDR,CHARGE_REGISTER_DDR_VALUE_BUF[i],((unsigned char*)(&CHARGE_REGISTER_DDR_VALUE_BUF[i+1])),2) != 0)
        {
            printf("write reg %x eer\n",CHARGE_REGISTER_DDR_VALUE_BUF[i]);
            return -1;
        }
    }

    printf("bq25703a charge_function init success\n");

    return 0;
}


int bq25703_set_ChargeCurrent(unsigned int charge_current_set)
{
    int charge_current = charge_current_set;
    int charge_vol = MAX_CHARGE_VOLTAGE;

    printf("set charge current: %dmA\n",charge_current);

    if(0 != bq25703a_i2c_write(
           BQ_I2C_ADDR,
           CHARGE_CURRENT_REGISTER_WR,
           ((unsigned char*)(&charge_current)),
           2)
      )
    {
        printf("write Current eer\n");
        return -1;
    }


    /*printf("set charge voltage: %dmA\n\n",charge_vol);

    if(0 != bq25703a_i2c_write(
           BQ_I2C_ADDR,
           MaxChargeVoltage_REGISTER_WR,
           ((unsigned char*)(&charge_vol)),
           2)
      )
    {
        printf("write VOLTAGE eer\n");
        return -1;
    }*/

    return 0;
}


int bq25703a_get_ChargeCurrentSetting(void)
{
    unsigned char buf[2] = {0};

    int charge_current = 0;

    if(bq25703a_i2c_read(BQ_I2C_ADDR, CHARGE_CURRENT_REGISTER_WR, buf, 2) != 0)
    {
        return -1;
    }
    else
    {
        printf("read charge_current_reg: 0x%02x 0x%02x\n",buf[0],buf[1]);

        if(buf[0] & 0x40)
        {
            charge_current += 64;
        }

        if(buf[0] & 0x80)
        {
            charge_current += 128;
        }

        if(buf[1] & 0x01)
        {
            charge_current += 256;
        }

        if(buf[1] & 0x02)
        {
            charge_current += 512;
        }

        if(buf[1] & 0x04)
        {
            charge_current += 1024;
        }

        if(buf[1] & 0x08)
        {
            charge_current += 2048;
        }

        if(buf[1] & 0x10)
        {
            charge_current += 4096;
        }

        printf("Charge Current Max: %dmA\n\n",charge_current);

        return charge_current;
    }

}


int bq25703_set_InputVoltageLimit(unsigned int input_voltage_limit_set)
{
    int input_voltage_limit = input_voltage_limit_set;

    printf("set charge input voltage limit: %dmA\n",input_voltage_limit + 3200);

    if(0 != bq25703a_i2c_write(
           BQ_I2C_ADDR,
           CHARGE_CURRENT_REGISTER_WR,
           ((unsigned char*)(&input_voltage_limit)),
           2)
      )
    {
        printf("write Current eer\n");
        return -1;
    }

    return 0;
}


int bq25703a_get_InputVoltageLimit(void)
{
    unsigned char buf[2] = {0};

    int input_voltage_limit = 3200;

    if(bq25703a_i2c_read(BQ_I2C_ADDR, INPUT_VOLTAGE_REGISTER_WR, buf, 2) != 0)
    {
        return -1;
    }
    else
    {
        printf("read input voltage limit reg: 0x%02x 0x%02x\n",buf[0],buf[1]);

        if(buf[0] & 0x40)
        {
            input_voltage_limit += 64;
        }

        if(buf[0] & 0x80)
        {
            input_voltage_limit += 128;
        }

        if(buf[1] & 0x01)
        {
            input_voltage_limit += 256;
        }

        if(buf[1] & 0x02)
        {
            input_voltage_limit += 512;
        }

        if(buf[1] & 0x04)
        {
            input_voltage_limit += 1024;
        }

        if(buf[1] & 0x08)
        {
            input_voltage_limit += 2048;
        }

        if(buf[1] & 0x10)
        {
            input_voltage_limit += 4096;
        }

        printf("Input Voltage Limit: %dmV\n\n",input_voltage_limit);

        return input_voltage_limit;
    }

}


int bq25703a_get_BatteryVol_and_SystemVol(unsigned int *p_BatteryVol, unsigned int *p_SystemVol)
{
    unsigned char buf[2] = {0};

    if(bq25703a_i2c_read(BQ_I2C_ADDR, SYSTEM_AND_BATTERY_VOLTAGE_READ_BACK_R, buf, 2) != 0)
    {
        return -1;
    }
    else
    {
        printf("read SYSTEM_AND_BATTERY_VOL reg: 0x%02x 0x%02x\n",buf[0],buf[1]);

        //vol = 2880mv + buf[1]*64
        *p_BatteryVol = 2880 + buf[0] * 64;
        *p_SystemVol = 2880 + buf[1] * 64;

        printf("Battery Voltage: %dmV\n",*p_BatteryVol);
        printf("System Voltage: %dmV\n\n",*p_SystemVol);
    }

    return 0;
}


int bq25703a_get_PSYS_and_VBUS(unsigned int *p_PSYS_vol, unsigned int *p_VBUS_vol)
{
    unsigned char buf[2] = {0};

    if(bq25703a_i2c_read(BQ_I2C_ADDR, VBUS_AND_PSYS_VOLTAGE_READ_BACK_R, buf, 2) != 0)
    {
        return -1;
    }
    else
    {
        printf("read PSYS_and_VBUS reg: 0x%02x 0x%02x\n",buf[0],buf[1]);

        //psys = value*12
        *p_PSYS_vol = buf[0] * 12;

        //vbus = 3200mv + value*64
        *p_VBUS_vol = 3200 + buf[1] * 64;

        printf("PSYS: %dmV\n",*p_PSYS_vol);
        printf("VBUS: %dmV\n\n",*p_VBUS_vol);
    }

    return 0;
}


int bq25703a_get_CMPINVol_and_InputCurrent(unsigned int *p_CMPIN_vol, unsigned int *p_input_current)
{
    unsigned char buf[2] = {0};

    if(bq25703a_i2c_read(BQ_I2C_ADDR, INPUT_CURRENT_AND_CMPIN_VOLTAGE_READ_BACK_R, buf, 2) != 0)
    {
        return -1;
    }
    else
    {
        printf("read CMPINVol_and_InputCurrent reg: 0x%02x 0x%02x\n",buf[0],buf[1]);

        //CMPIN: Full range: 3.06 V, LSB: 12 mV
        *p_CMPIN_vol = buf[0] * 12;

        //Iuput Current: Full range: 12.75 A, LSB: 50 mA
        *p_input_current = buf[1] * 50;

        printf("CMPIN Voltage: %dmV\n",*p_CMPIN_vol);
        printf("Input Current: %dmA\n\n",*p_input_current);
    }

    return 0;
}


int bq25703a_get_Battery_Current(unsigned int *p_battery_discharge_current, unsigned int *p_battery_charge_current)
{
    unsigned char buf[2] = {0};

    if(bq25703a_i2c_read(BQ_I2C_ADDR, CHARGE_AND_DISCHARGE_CURRENT_READ_BACK_R, buf, 2) != 0)
    {
        return -1;
    }
    else
    {
        printf("read Battery_Current reg: 0x%02x 0x%02x\n",buf[0],buf[1]);

        //IDCHG: Full range: 32.512 A, LSB: 256 mA
        *p_battery_discharge_current = buf[0] * 256;

        //ICHG: Full range: 8.128 A, LSB: 64 mA
        *p_battery_charge_current = buf[1] * 64;

        printf("Battery discharge current: %dmA\n",*p_battery_discharge_current);
        printf("Battery charge current: %dmA\n\n",*p_battery_charge_current);
    }

    return 0;
}


int bq25703a_get_Charger_Status(void)
{
    s_BQ_Charger_Status bq_charger_status = {0};

    s_BQ_Charger_Status *p_bq_charger_status = NULL;

    p_bq_charger_status = &bq_charger_status;

    if(bq25703a_i2c_read(BQ_I2C_ADDR, CHARGE_STATUS_REGISTER_R, (unsigned char*)p_bq_charger_status, 2) != 0)
    {
        return -1;
    }

    printf("get bq25703 Charger Status: \n");
    printf("Fault_OTG_UCP: %d\n", p_bq_charger_status->Fault_OTG_UCP);
    printf("Fault_OTG_OVP: %d\n", p_bq_charger_status->Fault_OTG_OVP);
    printf("Fault_Latchoff: %d\n", p_bq_charger_status->Fault_Latchoff);
    printf("SYSOVP_STAT: %d\n", p_bq_charger_status->SYSOVP_STAT);
    printf("Fault_ACOC: %d\n", p_bq_charger_status->Fault_ACOC);
    printf("Fault_BATOC: %d\n", p_bq_charger_status->Fault_BATOC);
    printf("Fault_ACOV: %d\n", p_bq_charger_status->Fault_ACOV);
    printf("IN_OTG: %d\n", p_bq_charger_status->IN_OTG);
    printf("IN_PCHRG: %d\n", p_bq_charger_status->IN_PCHRG);
    printf("IN_FCHRG: %d\n", p_bq_charger_status->IN_FCHRG);
    printf("IN_IINDPM: %d\n", p_bq_charger_status->IN_IINDPM);
    printf("IN_VINDPM: %d\n", p_bq_charger_status->IN_VINDPM);
    printf("ICO_DONE: %d\n", p_bq_charger_status->ICO_DONE);
    printf("AC_STAT: %d\n\n", p_bq_charger_status->AC_STAT);

    return 0;
}


int bq25703_init_ChargeOption_0(void)
{
    int charge_option_0_setting = CHARGE_OPTION_0_SETTING;

    printf("charge_option_0_setting: %04x\n",charge_option_0_setting);

    if(0 != bq25703a_i2c_write(
           BQ_I2C_ADDR,
           CHARGE_OPTION_0_WR,
           ((unsigned char*)(&charge_option_0_setting)),
           2)
      )
    {
        printf("write reg eer\n");
        return -1;
    }

    return 0;
}


/*
* LEARN function allows the battery to discharge while the adapter is present. It
* calibrates the battery gas gauge over a complete discharge/charge cycle. When
* the battery voltage is below battery depletion threshold, the system switches
* back to adapter input by the host. When CELL_BATPRESZ pin is LOW, the
* device exits LEARN mode and this bit is set back to 0
*/
int bq25703_enter_LEARN_Mode(void)
{
    int charge_option_0_setting = CHARGE_OPTION_0_SETTING | EN_LEARN;

    printf("charge_option_0_setting: %04x\n",charge_option_0_setting);

    if(0 != bq25703a_i2c_write(
           BQ_I2C_ADDR,
           CHARGE_OPTION_0_WR,
           ((unsigned char*)(&charge_option_0_setting)),
           2)
      )
    {
        printf("write reg eer\n");
        return -1;
    }

    printf("\nbq25703 enter LEARN_Mode\n\n\n");

    return 0;
}


int bq25703_stop_charge(void)
{
    if(bq25703_set_ChargeCurrent(CHARGE_CURRENT_0) == 0)
    {
        batteryManagePara.battery_is_charging = 0;

        return 0;
    }

    return -1;
}


int decide_the_ChargeCurrent(void)
{
    int charge_current = CHARGE_CURRENT_0;

    if(batteryManagePara.battery_voltage <= BATTERY_LOW_VOLTAGE_THRESHOLD)
    {
        if((batteryManagePara.battery_temperature >= BATTERY_CHARGE_ALLOW_TEMPERATURE_LOW_THRESHOLD)
           && (batteryManagePara.battery_temperature < BATTERY_LOW_TEMPERATURE_THRESHOLD))
        {
            charge_current = CHARGE_CURRENT_LEVEL_1;
        }
        else if((batteryManagePara.battery_temperature >= BATTERY_LOW_TEMPERATURE_THRESHOLD)
                && (batteryManagePara.battery_temperature <= BATTERY_CHARGE_ALLOW_TEMPERATURE_HIGH_THRESHOLD))
        {
            charge_current = CHARGE_CURRENT_LEVEL_2;
        }
        else
        {
            charge_current = CHARGE_CURRENT_0;
        }
    }

    if((batteryManagePara.battery_voltage > BATTERY_LOW_VOLTAGE_THRESHOLD) && (batteryManagePara.battery_voltage <= BATTERY_MAX_VOLTAGE_THRESHOLD))
    {
        if((batteryManagePara.battery_temperature >= BATTERY_CHARGE_ALLOW_TEMPERATURE_LOW_THRESHOLD)
           && (batteryManagePara.battery_temperature < BATTERY_LOW_TEMPERATURE_THRESHOLD))
        {
            charge_current = CHARGE_CURRENT_LEVEL_2;
        }
        else if((batteryManagePara.battery_temperature >= BATTERY_LOW_TEMPERATURE_THRESHOLD)
                && (batteryManagePara.battery_temperature <= BATTERY_CHARGE_ALLOW_TEMPERATURE_HIGH_THRESHOLD))
        {
            charge_current = CHARGE_CURRENT_LEVEL_3;
        }
        else
        {
            charge_current = CHARGE_CURRENT_0;
        }
    }

    if(batteryManagePara.battery_voltage > BATTERY_MAX_VOLTAGE_THRESHOLD)
    {
        charge_current = CHARGE_CURRENT_0;
    }

    printf("decide_the_ChargeCurrent: %dmA by voltage: %dmV, temperature: %dC\n",charge_current, batteryManagePara.battery_voltage, batteryManagePara.battery_temperature);

    return charge_current;
}


int bq25703_enable_charge(void)
{
    int ret;

    unsigned int VBus_vol = 0;
    unsigned int PSys_vol = 0;

    int tps65987_TypeC_current_type;

    int charge_current = CHARGE_CURRENT_0;

    bq25703a_get_PSYS_and_VBUS(&PSys_vol, &VBus_vol);
    printf("get VBus_vol = %d\n",VBus_vol);


    if(bq25703_init_ChargeOption_0() != 0)
    {
        return -1;
    }


    //check TypeC Current type to decide the charge current
    tps65987_TypeC_current_type = tps65987_get_TypeC_Current();

    switch(tps65987_TypeC_current_type)
    {
        case USB_Default_Current:

            batteryManagePara.charger_is_plug_in = 0;

            //disable USB default Current charger, use the Battery to discharge directly to the system
            ret = bq25703_enter_LEARN_Mode();

            if(ret == 0)
            {
                batteryManagePara.battery_is_charging = 0;
            }
            break;

        case C_1d5A_Current:

            batteryManagePara.charger_is_plug_in = 1;

            if(batteryManagePara.battery_fully_charged)
            {
                ret = 0;
                break;
            }

            ret = bq25703_set_ChargeCurrent(CHARGE_CURRENT_FOR_USB_Default);

            if(ret == 0)
            {
                batteryManagePara.battery_is_charging = 1;
            }
            break;

        case C_3A_Current:
        case PD_contract_negotiated:

            batteryManagePara.charger_is_plug_in = 1;

            if(batteryManagePara.battery_fully_charged)
            {
                ret = 0;
                break;
            }

            charge_current = decide_the_ChargeCurrent();

            ret = bq25703_set_ChargeCurrent(charge_current);

            if(ret == 0)
            {
                batteryManagePara.battery_is_charging = 1;
            }
            break;

        default:
            ret = -1;
            break;

    }

    return ret;

}


int init_Chg_OK_Pin(void)
{
    char file_path[64]= {0};

    int pin_number = CHG_OK_PIN;

    export_gpio(pin_number);
    set_direction(pin_number, "in");
    //set_edge(pin_number, "rising");
    set_edge(pin_number, "both");

    sprintf(file_path, "/sys/class/gpio/gpio%d/value", pin_number);

    fd_chg_ok_pin = open(file_path, O_RDONLY);
    if(fd_chg_ok_pin < 0)
    {
        printf("can't open %s!\n", file_path);
        return -1;
    }

    return 0;
}


int get_Chg_OK_Pin_value(void)
{
    unsigned char value[4];
    int n;

    if(lseek(fd_chg_ok_pin, 0, SEEK_SET) == -1)
    {
        printf("lseek failed!\n");
        return -1;
    }

    n = read(fd_chg_ok_pin, value, sizeof(value));
    printf("read %d bytes %c %c\n", n, value[0],value[1]);

    return value[0];
}


void batteryManagePara_init(void)
{
    batteryManagePara.battery_fully_charged = 0;
    batteryManagePara.need_charge_flag = 0;

    batteryManagePara.stop_charge_flag = 0;
    batteryManagePara.can_charge_flag = 0;

    batteryManagePara.adjust_eq_flag = 0;

    batteryManagePara.low_battery_flag = 0;

    batteryManagePara.battery_is_charging = 0;

    batteryManagePara.charger_is_plug_in = 0;

    batteryManagePara.led_battery_display_state = LED_BATTERY_INVALID_VALUE;

    batteryManagePara.battery_temperature = 0;
    batteryManagePara.battery_voltage = 0;

}

void batteryManagePara_clear(void)
{
    batteryManagePara.need_charge_flag = 0;
}


void check_BatteryFullyCharged_Task(void)
{
    switch(fuelgauge_check_BatteryFullyCharged())
    {
        case 1:
            if(!batteryManagePara.battery_fully_charged)
            {
                if(bq25703_stop_charge() != 0)
                {
                    break;
                }

                printf("fully charged, stop charging!\n");
            }

            batteryManagePara.battery_fully_charged = 1;
            batteryManagePara.need_charge_flag = 0;
            break;

        case 0:
            if(!batteryManagePara.need_charge_flag)
            {
                if(batteryManagePara.can_charge_flag)
                {
                    if(get_Chg_OK_Pin_value() == '1')
                    {
                        if(bq25703_enable_charge() != 0)
                        {
                            return;
                        }
                    }
                }
            }

            batteryManagePara.battery_fully_charged = 0;
            batteryManagePara.need_charge_flag = 1;
            break;

        default:
            break;
    }
}


int batteryTemperature_is_overstep_ChargeStopThreshold(int battery_temperature)
{
    if((battery_temperature >= BATTERY_CHARGE_STOP_TEMPERATURE_HIGH_THRESHOLD)
       || (battery_temperature <= BATTERY_CHARGE_STOP_TEMPERATURE_LOW_THRESHOLD))
    {
        return 1;
    }

    return 0;
}

int batteryTemperature_is_in_ChargeAllowThreshold(int battery_temperature)
{
    if(( battery_temperature < BATTERY_CHARGE_ALLOW_TEMPERATURE_HIGH_THRESHOLD)
       && (battery_temperature > BATTERY_CHARGE_ALLOW_TEMPERATURE_LOW_THRESHOLD))
    {
        return 1;
    }

    return 0;
}

int batteryTemperature_is_overstep_AdjustEQThreshold(int battery_temperature)
{
    if((battery_temperature >= BATTERY_DISCHARGE_ADJUST_EQ_TEMPERATURE_HIGH_THRESHOLD)
       && (battery_temperature < BATTERY_DISCHARGE_STOP_TEMPERATURE_HIGH_THRESHOLD))
    {
        return 1;
    }

    return 0;
}

int batteryTemperature_is_in_RecoveryEQThreshold(int battery_temperature)
{
    if(( battery_temperature < BATTERY_DISCHARGE_RECOVERY_EQ_TEMPERATURE_HIGH_THRESHOLD)
       && (battery_temperature > BATTERY_DISCHARGE_STOP_TEMPERATURE_LOW_THRESHOLD))
    {
        return 1;
    }

    return 0;
}

int batteryTemperature_is_overstep_DischargeStopThreshold(int battery_temperature)
{
    if((battery_temperature >= BATTERY_DISCHARGE_STOP_TEMPERATURE_HIGH_THRESHOLD)
       || (battery_temperature <= BATTERY_DISCHARGE_STOP_TEMPERATURE_LOW_THRESHOLD))
    {
        return 1;
    }

    return 0;
}


int batteryVoltage_is_over_MaxThreshold(int battery_voltage)
{
    if(battery_voltage > BATTERY_MAX_VOLTAGE_THRESHOLD)
    {
        return 1;
    }

    return 0;
}


int check_Battery_allow_charge(void)
{
    int battery_temperature;
    int battery_voltage;

    battery_temperature = fuelgauge_get_Battery_Temperature();
    if(battery_temperature == Temperature_UNVALID)
    {
        return -1;
    }

    batteryManagePara.battery_temperature = battery_temperature;


    battery_voltage = fuelgauge_get_Battery_Voltage();
    if(battery_voltage == -1)
    {
        return -1;
    }

    batteryManagePara.battery_voltage = battery_voltage;


    if(batteryTemperature_is_in_ChargeAllowThreshold(batteryManagePara.battery_temperature)
       && (!batteryVoltage_is_over_MaxThreshold(batteryManagePara.battery_voltage)))
    {
        printf("battery allow charging!, temperature %d, voltage %dmV\n",batteryManagePara.battery_temperature, batteryManagePara.battery_voltage);
        return 1;
    }

    printf("battery not allow charging!, temperature %d, voltage %dmV\n",batteryManagePara.battery_temperature, batteryManagePara.battery_voltage);
    return 0;
}


int check_TypeC_current_type(void)
{
    int tps65987_TypeC_current_type;

    //check TypeC Current type
    tps65987_TypeC_current_type = tps65987_get_TypeC_Current();

    if(tps65987_TypeC_current_type == -1)
    {
        return -1;
    }

    switch(tps65987_TypeC_current_type)
    {
        case USB_Default_Current:
            batteryManagePara.charger_is_plug_in = 0;
            break;

        case C_1d5A_Current:
        case C_3A_Current:
        case PD_contract_negotiated:
            batteryManagePara.charger_is_plug_in = 1;
            break;
    }

    return 0;
}


int create_batteryTemperture_logFile(void)
{
    fp_batt_temp = fopen("/data/battery_temperature_log","a+");
    if(fp_batt_temp == NULL)
    {
        printf("fail to create battery_temperature_log\n");
        return -1;
    }

    log_batt_temp_flag = 1;

    printf("open battery_temperature_log file success, fd is %d\n", fileno(fp_batt_temp));

    time_t nSeconds;
    struct tm * pTM;

    time(&nSeconds);
    pTM = localtime(&nSeconds);

    if(fprintf(fp_batt_temp, "\n\nstart_new_log: %04d-%02d-%02d %02d:%02d:%02d\n",
               pTM->tm_year + 1900, pTM->tm_mon + 1, pTM->tm_mday,
               pTM->tm_hour, pTM->tm_min, pTM->tm_sec) >= 0)
    {
        fflush(fp_batt_temp);
    }
    else
    {
        printf("write file error");
    }

    return 0;
}

int system_power_off(void)
{
    int pin_number = 41; //system power pin

    return set_value(pin_number, 1);
}


int update_fuelgauge_BatteryInfo(void)
{
    //get by fuelgauge IC
    int battery_temperature;
    int battery_voltage;
    int battery_current;
    int battery_relativeStateOfCharge;


    battery_voltage = fuelgauge_get_Battery_Voltage();
    if(battery_voltage == -1)
    {
        return -1;
    }

    batteryManagePara.battery_voltage = battery_voltage;

    battery_current = fuelgauge_get_Battery_Current();

    battery_relativeStateOfCharge = fuelgauge_get_RelativeStateOfCharge();
    if(battery_relativeStateOfCharge != -1)
    {
        if(battery_relativeStateOfCharge < 10)
        {
            batteryManagePara.low_battery_flag = 1;
        }
        else
        {
            batteryManagePara.low_battery_flag = 0;
        }
    }

    battery_temperature = fuelgauge_get_Battery_Temperature();
    if(battery_temperature == Temperature_UNVALID)
    {
        return -1;
    }

    batteryManagePara.battery_temperature = battery_temperature;

    return 0;
}


void batteryCharge_handle_Task(int battery_temperature)
{
    if(batteryTemperature_is_overstep_ChargeStopThreshold(battery_temperature))
    {
        if(!batteryManagePara.stop_charge_flag)
        {
            if(bq25703_stop_charge() != 0)
            {
                return;
            }

            printf("battery temperature %d, is over threshold, stop charging!\n",battery_temperature);
        }

        batteryManagePara.stop_charge_flag = 1;
        batteryManagePara.can_charge_flag = 0;
    }
    else if(batteryTemperature_is_in_ChargeAllowThreshold(battery_temperature))
    {
        if(!batteryManagePara.can_charge_flag)
        {
            if(!batteryManagePara.battery_fully_charged)
            {
                if(get_Chg_OK_Pin_value() == '1')
                {
                    if(bq25703_enable_charge() != 0)
                    {
                        return;
                    }
                }
            }

            printf("battery temperature %d, is in threshold, can charging!\n",battery_temperature);
        }

        batteryManagePara.stop_charge_flag = 0;
        batteryManagePara.can_charge_flag = 1;
    }
}


void batteryDisCharge_handle_Task(int battery_temperature)
{
    //when Adapter is pluged, no need to adjust EQ
    if(!batteryManagePara.charger_is_plug_in)
    {
        if(batteryTemperature_is_overstep_AdjustEQThreshold(battery_temperature))
        {
            if(!batteryManagePara.adjust_eq_flag)
            {
                //adjust EQ
                printf("battery temperature %d, overstep AdjustEQThreshold, adjust EQ\n",battery_temperature);
            }

            batteryManagePara.adjust_eq_flag = 1;
        }
        else if(batteryTemperature_is_in_RecoveryEQThreshold(battery_temperature))
        {
            if(batteryManagePara.adjust_eq_flag)
            {
                //recovery EQ
                printf("battery temperature %d, in RecoveryEQThreshold, recovery EQ\n",battery_temperature);
            }

            batteryManagePara.adjust_eq_flag = 0;
        }
    }

    if(batteryTemperature_is_overstep_DischargeStopThreshold(battery_temperature))
    {
        //power off
        if(system_power_off() != 0)
        {
            printf("system power_off fail\n\n");
        }
    }
}


void batteryTemperature_handle_Task(void)
{
    if(update_fuelgauge_BatteryInfo() != 0)
    {
        return;
    }

    if(log_batt_temp_flag)
    {
        //log the battery_temperature for debug
        if(fprintf(fp_batt_temp, "%d\n", batteryManagePara.battery_temperature) >= 0)
        {
            fflush(fp_batt_temp);
        }
        else
        {
            printf("write file error");
        }
    }

    batteryCharge_handle_Task(batteryManagePara.battery_temperature);

    batteryDisCharge_handle_Task(batteryManagePara.battery_temperature);
}



void led_battery_display_init(void)
{
    /*export_gpio(BATTERY_LED_R_PIN);
    set_direction(BATTERY_LED_R_PIN, "out");

    export_gpio(BATTERY_LED_G_PIN);
    set_direction(BATTERY_LED_G_PIN, "out");

    export_gpio(BATTERY_LED_B_PIN);
    set_direction(BATTERY_LED_B_PIN, "out");*/

    /**********************************************
     It is init by kernel now:
     /sys/class/leds/power_led_r/brightness
     /sys/class/leds/power_led_g/brightness
     /sys/class/leds/power_led_b/brightness
    **********************************************/

}


void led_battery_display(LED_BATTERY_DISPLAY_STATE type)
{
    switch(type)
    {
        case LED_BATTERY_FULLY_CHARGED:
            //system("adk-message-send 'led_start_pattern{pattern:31}'");

            //now the 0/1 is reversed
            set_battery_led('r', 0);
            set_battery_led('g', 0);
            set_battery_led('b', 0);

            printf("display LED_BATTERY_FULLY_CHARGED\n\n");
            break;

        case LED_BATTERY_CHARGEING:
            //system("adk-message-send 'led_start_pattern{pattern:30}'");

            set_battery_led('r', 1);
            set_battery_led('g', 0);
            set_battery_led('b', 1);

            printf("display LED_BATTERY_CHARGEING\n\n");
            break;

        case LED_BATTERY_LOW:
            //system("adk-message-send 'led_start_pattern{pattern:32}'");

            set_battery_led('r', 0);
            set_battery_led('g', 1);
            set_battery_led('b', 1);

            printf("display LED_BATTERY_LOW\n\n");
            break;

        case LED_BATTERY_OFF:
            //system("adk-message-send 'led_start_pattern{pattern:29}'");

            set_battery_led('r', 1);
            set_battery_led('g', 1);
            set_battery_led('b', 1);

            printf("display LED_BATTERY_OFF\n\n");
            break;

        default:
            break;

    }
}


void led_battery_display_handle(void)
{
    if(batteryManagePara.battery_is_charging)
    {
        if(batteryManagePara.led_battery_display_state != LED_BATTERY_CHARGEING)
        {
            led_battery_display(LED_BATTERY_CHARGEING);
        }

        batteryManagePara.led_battery_display_state = LED_BATTERY_CHARGEING;
    }
    else
    {
        if(batteryManagePara.battery_fully_charged)
        {
            if(batteryManagePara.charger_is_plug_in)
            {

                if(batteryManagePara.led_battery_display_state != LED_BATTERY_FULLY_CHARGED)
                {
                    led_battery_display(LED_BATTERY_FULLY_CHARGED);
                }

                batteryManagePara.led_battery_display_state = LED_BATTERY_FULLY_CHARGED;
            }
            else
            {
                if(batteryManagePara.led_battery_display_state != LED_BATTERY_OFF)
                {
                    led_battery_display(LED_BATTERY_OFF);
                }

                batteryManagePara.led_battery_display_state = LED_BATTERY_OFF;
            }
        }

        if(batteryManagePara.low_battery_flag)
        {
            if(batteryManagePara.led_battery_display_state != LED_BATTERY_LOW)
            {
                led_battery_display(LED_BATTERY_LOW);
            }

            batteryManagePara.led_battery_display_state = LED_BATTERY_LOW;
        }

        if((!batteryManagePara.battery_fully_charged) && (!batteryManagePara.low_battery_flag))
        {
            if(batteryManagePara.led_battery_display_state != LED_BATTERY_OFF)
            {
                led_battery_display(LED_BATTERY_OFF);
            }

            batteryManagePara.led_battery_display_state = LED_BATTERY_OFF;
        }

    }

}


int battery_shutdown_mode_check(void)
{
    //just fot debug use
    FILE *fp;

    unsigned char t_buf[64];

    int ret;
    int i;

    fp = fopen("/tmp/batt_shutdown","r");
    if(fp == NULL)
    {
        //no such file
        return -1;
    }

    ret = fread(t_buf,1,3,fp);

    if(ret != 3)
    {
        //no valid value
        return -1;
    }

    printf("read %d data from file:", ret);
    for(i=0; i<ret; i++)
    {
        printf("%c",t_buf[i]);
    }
    printf("\n");

    t_buf[3] = '\0';

    if(strcmp(t_buf,"yes") == 0)
    {
        return 1;
    }

    return 0;
}

void battery_shutdown_mode_handle(void)
{
    if(battery_shutdown_mode_check() == 1)
    {
        printf("make battery enter shutdown mode\n");

        if(bq25703_stop_charge() != 0)
        {
            return;
        }

        sleep(1);

        if(fuelgauge_battery_enter_shutdown_mode() != 0)
        {
            printf("battery enter_shutdown_mode err\n");
            return;
        }

        fuelgauge_disable_communication();
    }
    else
    {
        fuelgauge_enable_communication();
    }
}


void *bq25703a_chgok_irq_thread(void *arg)
{
    int ret;
    unsigned char j = 0;

    fds_chg_ok_pin[0].fd = fd_chg_ok_pin;
    fds_chg_ok_pin[0].events = POLLPRI;

    while(1)
    {
        /*
        * When VBUS rises above 3.5V or
        * falls below 24.5V, CHRG_OK is HIGH after 50ms deglitch time. When VBUS is falls below
        * 3.2 V or rises above 26 V, CHRG_OK is LOW. When fault occurs, CHRG_OK is asserted
        * LOW.
        */

        //wait for CHRG_OK Pin to be RISING HIGH
        ret = poll(fds_chg_ok_pin, 1, -1);
        printf("poll rising return = %d\n",ret);

        if(ret > 0)
        {
            if(fds_chg_ok_pin[0].revents & POLLPRI)
            {
                printf("CHRG_OK Edge detect, count = %d\n", j++);

                usleep(50*1000); //wait for a while

                unsigned char pin_value = get_Chg_OK_Pin_value();

                //AC unplug
                if(pin_value == '0')
                {
                    batteryManagePara.charger_is_plug_in = 0;
                    batteryManagePara.battery_is_charging = 0;
                }
                else if(pin_value == '1')
                {
                    sleep(1); //wait for status to be stable and tps check, no more shorter

                    //reset the params when AC plug in
                    batteryManagePara_clear();

                    int ret_val;

                    int tps_err_cnt = 0;
                    int err_cnt = 0;

                    while(get_Chg_OK_Pin_value() == '1')
                    {
                        if(check_TypeC_current_type() == -1)
                        {
                            if(tps_err_cnt++ > 3)
                            {
                                break;
                            }

                            usleep(10*1000);
                            continue;
                        }

                        ret_val = check_Battery_allow_charge();

                        if(ret_val == 1)
                        {
                            if(bq25703_enable_charge() == 0)
                            {
                                break;
                            }
                        }
                        else if(ret_val == 0)
                        {
                            break;
                        }

                        if(err_cnt++ > 3)
                        {
                            break;
                        }

                        usleep(10*1000);
                    }

                }

                led_battery_display_handle();
            }
        }
    }
}

#define TIMER_PIRIOD    5

int main(int argc, char* argv[])
{
    int i;
    int err_cnt = 0;

    int timer_cnt = TIMER_PIRIOD;

    unsigned int VBUS_vol;
    unsigned int PSYS_vol;

    unsigned int charge_current_set;

    int tps65987_port_role;
    int tps65987_TypeC_current_type;

    pthread_t thread_check_chgok_ntid;

    if(argc > 1)
    {
        for(i = 0; i < argc; i++)
        {
            printf("Argument %d is %s\n", i, argv[i]);
        }

        if(strcmp(argv[1],"log_batt_temp") == 0)
        {
            if(create_batteryTemperture_logFile() != 0)
            {
                return -1;
            }
        }
    }

    batteryManagePara_init();

    if(i2c_open_bq25703() != 0)
    {
        printf("i2c can't open bq25703!\n");
        return -1;
    }

    while(bq25703a_charge_function_init() != 0)
    {
        if(err_cnt++ > 3)
        {
            return -1;
        }

        usleep(100*1000);
    }


    if(i2c_open_tps65987() != 0)
    {
        printf("i2c can't open tps65987!\n");
        return -1;
    }

    if(i2c_open_fuelgauge() != 0)
    {
        printf("i2c can't open fuelgauge!\n");
        return -1;
    }


    if(init_Chg_OK_Pin() != 0)
    {
        printf("init Chg_OK_Pin fail!\n");
        return -1;
    }

    led_battery_display_init();

    //start irq thread
    pthread_create(&thread_check_chgok_ntid, NULL, bq25703a_chgok_irq_thread, NULL);

    while(1)
    {
        if(timer_cnt++ >= TIMER_PIRIOD)
        {

            timer_cnt = 0;

            bq25703a_get_PSYS_and_VBUS(&PSYS_vol, &VBUS_vol);
            charge_current_set = bq25703a_get_ChargeCurrentSetting();

            check_BatteryFullyCharged_Task();

            batteryTemperature_handle_Task();

            led_battery_display_handle();

            printf("\n\n\n");
        }

        battery_shutdown_mode_handle();

        sleep(1);
    }

    return 0;
}


