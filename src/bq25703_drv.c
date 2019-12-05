/**
*  @file      gpio_config.c
*  @brief     gpio_config
*  @author    Zack Li
*  @date      11 -2019
*  @copyright
*/

#include "bq25703_drv.h"
#include "gpio_config.h"



#define I2C_FILE_NAME   "/dev/i2c-2"
#define BQ_I2C_ADDR        0x6B

static int fd;

uint16_t CHARGE_REGISTER_DDR_VALUE_BUF[]=
{
    /*0*/       CHARGE_OPTION_0_WR,         0x020E,
    /*2*/       CHARGE_CURRENT_REGISTER_WR, 0x0800,
    /*4*/       CHARGE_VOLTAGE_REGISTER_WR, 0x3070,
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


static int i2c_open()
{
    int ret;
    int val;

    fd = open(I2C_FILE_NAME, O_RDWR);

    if(fd < 0)
    {
        perror("Unable to open i2c control file");

        return 1;
    }

    printf("open i2c file success %d\n",fd);

    ret = ioctl(fd, I2C_SLAVE_FORCE, BQ_I2C_ADDR);
    if (ret < 0)
    {
        perror("i2c: Failed to set i2c device address\n");
        return 1;
    }

    printf("i2c: set i2c device address success\n");

    val = 3;
    ret = ioctl(fd, I2C_RETRIES, val);
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

    if(ioctl(fd, I2C_RDWR, &data) < 0)
    {
        printf("write ioctl err %d\n",fd);
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

    if(ioctl(fd, I2C_RDWR, &data) < 0)
    {
        perror("---");
        printf("read ioctl err %d\n",fd);

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


int set_charge_voltage_current()
{
    int charge_current = 0x0B80; //2944mA
    int charge_vol = 0x3070; // 12400mV

    printf("set charge current: %d\n",charge_current);

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


    printf("set charge voltage: %d\n",charge_vol);

    if(0 != bq25703a_i2c_write(
           BQ_I2C_ADDR,
           CHARGE_VOLTAGE_REGISTER_WR,
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
        printf("read charge_current_reg: %02x %02x\n\n",buf[0],buf[1]);

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

        printf("Charge Current: %d\n",charge_current);

        return charge_current;
    }

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


int bq25703a_get_BatteryVol_and_SystemVol(unsigned int *p_BatteryVol, unsigned int *p_SystemVol)
{
    unsigned char buf[2] = {0};

    if(bq25703a_i2c_read(BQ_I2C_ADDR, SYSTEM_AND_BATTERY_VOLTAGE_READ_BACK_R, buf, 2) != 0)
    {
        return -1;
    }
    else
    {
        printf("read SYSTEM_AND_BATTERY_VOL reg: %02x %02x\n\n",buf[0],buf[1]);

        //vol = 2880mv + buf[1]*64
        *p_BatteryVol = 2880 + buf[0] * 64;
        *p_SystemVol = 2880 + buf[1] * 64;

        printf("Battery Voltage: %d\n",*p_BatteryVol);
        printf("System Voltage: %d\n",*p_SystemVol);
    }

    return 0;
}

int bq25703a_get_VBUS_and_PSYS(unsigned int *p_PSYS_vol, unsigned int *p_VBUS_vol)
{
    unsigned char buf[2] = {0};

    if(bq25703a_i2c_read(BQ_I2C_ADDR, VBUS_AND_PSYS_VOLTAGE_READ_BACK_R, buf, 2) != 0)
    {
        return -1;
    }
    else
    {
        printf("read VBUS_and_PSYS reg: %02x %02x\n\n",buf[0],buf[1]);

        //psys = value*12
        *p_PSYS_vol = buf[0] * 12;

        //vbus = 3200mv + value*64
        *p_VBUS_vol = 3200 + buf[1] * 64;

        printf("VBUS: %d\n",*p_VBUS_vol);
        printf("PSYS: %d\n",*p_PSYS_vol);
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
        printf("read CMPINVol_and_InputCurrent reg: %02x %02x\n\n",buf[0],buf[1]);

        //CMPIN: Full range: 3.06 V, LSB: 12 mV
        *p_CMPIN_vol = buf[0] * 50;

        //IIN: Full range: 12.75 A, LSB: 50 mA
        *p_input_current = buf[1] * 12;

        printf("CMPIN Voltage: %d\n",*p_CMPIN_vol);
        printf("Input Current: %d\n",*p_input_current);
    }

    return 0;
}


void *bq25703a_chgok_irq_thread(void *arg)
{
    int ret;
    int n;

    unsigned char value[4];
    unsigned char j = 0;

    char file_path[64]= {0};

    int pin_number = CHG_OK_PIN;

    register_gpiox(pin_number);
    set_direction(pin_number, "in");
    set_edge(pin_number, "rising");

    sprintf(file_path, "/sys/class/gpio/gpio%d/value", pin_number);
    int fd = open(file_path, O_RDONLY);
    if(fd < 0)
    {
        printf("can't open %s!\n", file_path);
        return ((void *)0);
    }

    struct pollfd fds[1];
    fds[0].fd = fd;
    fds[0].events = POLLPRI;

    while(1)
    {
        ret = poll(fds, 1, -1);
        printf("poll return = %d\n",ret);

        if(ret > 0)
        {
            if(fds[0].revents & POLLPRI)
            {
                if(lseek(fd, 0, SEEK_SET) == -1)
                {
                    printf("lseek failed!\n");
                    break;
                }

                n = read(fd, value, sizeof(value));
                printf("read %d bytes %c %c, count = %d\n", n, value[0],value[1],j++);

                if(value[0] == '1')
                {
                    set_charge_voltage_current();
                }
            }
        }
    }

    unregister_gpiox(pin_number);
    pthread_exit("bq25703a_chgok_irq_thread exit");
}


int main(int argc, char* argv[])
{
    int i;

    unsigned int battery_vol;
    unsigned int system_vol;

    unsigned int CMPIN_vol;
    unsigned int input_current;

    unsigned int VBUS_vol;
    unsigned int PSYS_vol;

    unsigned int charge_current;

    pthread_t thread_check_chgok_ntid;

    if(argc > 1)
    {
        for(i = 0; i < argc; i++)
        {
            printf("Argument %d is %s\n", i, argv[i]);
        }
    }

    i2c_open();

    bq25703a_charge_function_init();

    //start irq thread
    pthread_create(&thread_check_chgok_ntid, NULL, bq25703a_chgok_irq_thread, NULL);

    while(1)
    {
        /*if((argc > 1) && (strcmp(argv[1],"Vol") == 0))
        {
            bq25703a_get_BatteryVol_and_SystemVol(&battery_vol,&system_vol);
        }
        else if((argc > 1) && (strcmp(argv[1],"Current") == 0))
        {
            bq25703a_get_CMPINVol_and_InputCurrent(&CMPIN_vol,&input_current);
        }*/

        bq25703a_get_BatteryVol_and_SystemVol(&battery_vol, &system_vol);

        bq25703a_get_VBUS_and_PSYS(&PSYS_vol, &VBUS_vol);

        bq25703a_get_CMPINVol_and_InputCurrent(&CMPIN_vol, &input_current);

        charge_current = bq25703a_get_ChargeCurrent();

        printf("\n\n\n");

        sleep(5);
    }

    return 0;
}


