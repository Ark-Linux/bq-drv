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

#include "bq25703_drv.h"
#include "gpio_config.h"

#include "tps65987_interface.h"
#include "bq40z50_interface.h"


#define I2C_FILE_NAME   "/dev/i2c-2"
#define BQ_I2C_ADDR        0x6B

static int fd_i2c;

int fd_chg_ok_pin;
struct pollfd fds_chg_ok_pin[1];


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


uint16_t CHARGE_REGISTER_DDR_VALUE_BUF[]=
{
    /*0*/       CHARGE_OPTION_0_WR,         0x020E,
    /*2*/       CHARGE_CURRENT_REGISTER_WR, CHARGE_CURRENT_0,
    /*4*/       MaxChargeVoltage_REGISTER_WR, MAX_CHARGE_VOLTAGE,
    /*6*/       OTG_VOLTAGE_REGISTER_WR,    0x0000,
    /*8*/       OTG_CURRENT_REGISTER_WR,    0x0000,
    /*10*/      INPUT_VOLTAGE_REGISTER_WR,  INPUT_VOLTAGE_LIMIT_3V2, //here should use the default value:0x0000, means 3200mV
    /*12*/      MINIMUM_SYSTEM_VOLTAGE_WR,  0x2400, //The charger provides minimum system voltage, means 9216mV
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
    /*4*/       MaxChargeVoltage_REGISTER_WR, 0x0000,
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


struct BATTERY_MANAAGE_PARA
{
    unsigned char battery_fully_charged;
    unsigned char need_charge_flag;

    unsigned char stop_charge_flag;
    unsigned char can_charge_flag;

} batteryManagePara;



int i2c_open_bq25703(void)
{
    int ret;
    int val;

    fd_i2c = open(I2C_FILE_NAME, O_RDWR);

    if(fd_i2c < 0)
    {
        perror("Unable to open i2c control file");

        return -1;
    }

    printf("open i2c file success %d\n",fd_i2c);

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


int bq25703a_i2c_write(unsigned char dev_addr, unsigned char reg, unsigned char *val, unsigned char data_len)
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


int bq25703a_i2c_read(unsigned char addr, unsigned char reg, unsigned char *val, unsigned char len)
{
    return i2c_read(addr, reg, val, len);
}


int bq25703a_otg_function_init()
{
    for (int i = 0; i < sizeof(OTG_REGISTER_DDR_VALUE_BUF) - 1; i ++)
    {
        if (i%2 == 0)
        {
            if(0 != bq25703a_i2c_write(
                   BQ_I2C_ADDR,
                   OTG_REGISTER_DDR_VALUE_BUF[i],
                   ((unsigned char*)(&OTG_REGISTER_DDR_VALUE_BUF[i+1])),
                   2)
              )
            {
                printf("write register addr %d eer\n", OTG_REGISTER_DDR_VALUE_BUF[i]);
                return -1;
            }
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

    for (int i = 0; i < sizeof(CHARGE_REGISTER_DDR_VALUE_BUF) - 1; i ++)
    {
        if (i%2 == 0)
        {
            if(0 != bq25703a_i2c_write(
                   BQ_I2C_ADDR,
                   CHARGE_REGISTER_DDR_VALUE_BUF[i],
                   ((unsigned char*)(&CHARGE_REGISTER_DDR_VALUE_BUF[i+1])),
                   2)
              )
            {
                printf("write %d eer\n",CHARGE_REGISTER_DDR_VALUE_BUF[i]);
                return -1;
            }
        }
    }

    printf("bq25703a charge_function init success\n");

    return 0;
}


int bq25703_set_MaxChargeVoltage_and_Current(unsigned int charge_current_set)
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


    printf("set charge voltage: %dmA\n\n",charge_vol);

    if(0 != bq25703a_i2c_write(
           BQ_I2C_ADDR,
           MaxChargeVoltage_REGISTER_WR,
           ((unsigned char*)(&charge_vol)),
           2)
      )
    {
        printf("write VOLTAGE eer\n");
        return -1;
    }

    return 0;
}


int bq25703a_get_ChargeCurrent(void)
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
        printf("read VBUS_and_PSYS reg: 0x%02x 0x%02x\n",buf[0],buf[1]);

        //psys = value*12
        *p_PSYS_vol = buf[0] * 12;

        //vbus = 3200mv + value*64
        *p_VBUS_vol = 3200 + buf[1] * 64;

        printf("VBUS: %dmV\n",*p_VBUS_vol);
        printf("PSYS: %dmV\n\n",*p_PSYS_vol);
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


int bq25703_stop_charge(void)
{
    return bq25703_set_MaxChargeVoltage_and_Current(CHARGE_CURRENT_0);
}


int bq25703_enable_charge(void)
{
    int ret;

    unsigned int VBus_vol = 0;
    unsigned int PSys_vol = 0;

    int tps65987_TypeC_current_type;

    bq25703a_get_PSYS_and_VBUS(&PSys_vol, &VBus_vol);
    printf("get VBus_vol = %d\n",VBus_vol);

    /*if(VBus_vol < 5500)
    {
        //bq25703_set_MaxChargeVoltage_and_Current(CHARGE_CURRENT_FOR_USB_Default);
        //just disable 5V charge now
        printf("do not charge at 5V\n");
    }
    else
    {
        bq25703_set_MaxChargeVoltage_and_Current(CHARGE_CURRENT_FOR_PD);
    }*/


    //check TypeC Current type to decide the charge current
    tps65987_TypeC_current_type = tps65987_get_TypeC_Current();

    switch(tps65987_TypeC_current_type)
    {
        case USB_Default_Current:
        case C_1d5A_Current:
            ret = bq25703_set_MaxChargeVoltage_and_Current(CHARGE_CURRENT_FOR_USB_Default);
            break;

        case C_3A_Current:
        case PD_contract_negotiated:
            ret = bq25703_set_MaxChargeVoltage_and_Current(CHARGE_CURRENT_FOR_PD);
            break;

    }

    return ret;

}


int init_Chg_OK_Pin(void)
{
    char file_path[64]= {0};

    int pin_number = CHG_OK_PIN;

    register_gpiox(pin_number);
    set_direction(pin_number, "in");
    set_edge(pin_number, "rising");

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
}

void batteryManagePara_clear(void)
{
    batteryManagePara.battery_fully_charged = 0;
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
                if(!batteryManagePara.stop_charge_flag)
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


int check_BatteryTemperature_is_in_threshold(void)
{
    int battery_temperature;

    battery_temperature = fuelgauge_get_Battery_Temperature();
    if(battery_temperature == -1)
    {
        return -1;
    }

    if(( battery_temperature < BATTERY_CHARGE_ALLOW_TEMPERATURE_HIGH_THRESHOLD)
       && (battery_temperature > BATTERY_CHARGE_ALLOW_TEMPERATURE_LOW_THRESHOLD))
    {
        printf("battery temperature %d, is in threshold, allow charging!\n",battery_temperature);
        return 1;
    }

    return 0;
}


void check_BatteryTemperature_Task(void)
{
    //get by fuelgauge IC
    int battery_temperature;
    int battery_voltage;
    int battery_current;
    int battery_relativeStateOfCharge;


    battery_voltage = fuelgauge_get_Battery_Voltage();
    battery_current = fuelgauge_get_Battery_Current();
    battery_relativeStateOfCharge = fuelgauge_get_RelativeStateOfCharge();

    battery_temperature = fuelgauge_get_Battery_Temperature();
    if(battery_temperature == -1)
    {
        return;
    }

    if((battery_temperature >= BATTERY_CHARGE_STOP_TEMPERATURE_HIGH_THRESHOLD)
       || (battery_temperature <= BATTERY_CHARGE_STOP_TEMPERATURE_LOW_THRESHOLD))
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
    else if(( battery_temperature < BATTERY_CHARGE_ALLOW_TEMPERATURE_HIGH_THRESHOLD)
            && (battery_temperature > BATTERY_CHARGE_ALLOW_TEMPERATURE_LOW_THRESHOLD))
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
                printf("CHRG_OK Rising HIGH, count = %d\n", j++);

                sleep(1); //wait for status to be stable, typical it takes 200ms for VBUS rise from 5V to 15V

                //reset the params when AC plug IN
                batteryManagePara_clear();

                if(get_Chg_OK_Pin_value() == '1')
                {
                    if(check_BatteryTemperature_is_in_threshold() == 1)
                    {
                        bq25703_enable_charge();
                    }
                }
            }
        }
    }

    pthread_exit("bq25703a_chgok_irq_thread exit");
}


int main(int argc, char* argv[])
{
    int i;

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
    }

    batteryManagePara_init();

    if(i2c_open_bq25703() != 0)
    {
        printf("i2c can't open bq25703!\n");
        return -1;
    }

    bq25703a_charge_function_init();


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

    //start irq thread
    pthread_create(&thread_check_chgok_ntid, NULL, bq25703a_chgok_irq_thread, NULL);

    while(1)
    {
        bq25703a_get_PSYS_and_VBUS(&PSYS_vol, &VBUS_vol);
        charge_current_set = bq25703a_get_ChargeCurrent();

        check_BatteryFullyCharged_Task();

        check_BatteryTemperature_Task();

        printf("\n\n\n");

        sleep(5);
    }

    return 0;
}


