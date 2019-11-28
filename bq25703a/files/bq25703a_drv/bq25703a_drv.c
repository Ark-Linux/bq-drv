/**
*  @file      gpio_config.c
*  @brief     gpio_config
*  @author    Zack Li
*  @date      11 -2019
*  @copyright
*/

#include "bq25703a_drv.h"
#include "gpio_config.h"

#define I2C_FILE_NAME   "/dev/i2c-2"
#define BQ_I2C_ADDR        0x6B
int fd;

int i2c_open()
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

    printf("i2c: set i2c device address success%d\n");

    val = 3;
    ret = ioctl(fd, I2C_RETRIES, val);
    if(ret < 0)
    {
        printf("i2c: set i2c retry times err\n");
    }

    printf("i2c: set i2c retry times %d\n",val);

    return 0;
}
int i2c_write(unsigned char dev_addr, unsigned char *val, unsigned char len)
{
    int ret;

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
        perror("---");
        printf("write ioctl err %d\n",fd);

        return 1;
    }

    usleep(1000);

    return 0;
}
int i2c_read(unsigned char addr, unsigned char reg, unsigned char *val, unsigned char len)
{
    int ret;

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

        return 1;
    }

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

int bq25703a_charge_function_init(int voltage, int current_mA)
{
    unsigned char val[2]= {0};
	
    //charge option 0
    little_to_big(0x020E, val);
    bq25703a_i2c_write(BQ_I2C_ADDR, 0x00, val, sizeof(val));

    set_charge_voltage_current(voltage, current_mA);

    little_to_big(0x0210, val);
    bq25703a_i2c_write(BQ_I2C_ADDR, 0x30, val, sizeof(val));

    //ADC
    little_to_big(0xE0FF, val);
    bq25703a_i2c_write(BQ_I2C_ADDR, 0x3A, val, sizeof(val));

    return 0;
}
int set_charge_voltage_current(int voltage, int current_mA)
{
    unsigned char val[2]= {0};
	//printf("-----%x-----%x-----\n\n",voltage, current_mA);
	
    little_to_big(0X04D0, val);
    if(1 == bq25703a_i2c_write(BQ_I2C_ADDR, 0x04, val, sizeof(val)))
    {
        return 1;
    }

    little_to_big(0X0800, val);

    if(1 == bq25703a_i2c_write(BQ_I2C_ADDR, 0x02, val, sizeof(val)))
    {
        return 1;
    }
    return 0;
}

int bq25703a_otg_function_init(int voltage, int current_mA)
{

    unsigned char val[2]= {0};
    //Set EN_OTG high -> GPIO set high
    //set_gpiox(36, "out", 1);
    set_gpiox_high(36);

    //charge option0 0x00->E20E
    little_to_big(0xE20E, val);
    bq25703a_i2c_write(BQ_I2C_ADDR, 0x00, val, sizeof(val));

    //Write the Charge Voltage Register to 0x20D0
    little_to_big(0x20D0, val);
    bq25703a_i2c_write(BQ_I2C_ADDR, 0x04, val, sizeof(val));

    set_otg_vol_current(voltage, current_mA);

    //EN_OTG in Charge Option 3
    little_to_big(0x1000, val);
    bq25703a_i2c_write(BQ_I2C_ADDR, 0x34, val, sizeof(val));

    //ADC
    little_to_big(0xE0FF, val);
    bq25703a_i2c_write(BQ_I2C_ADDR, 0x3A, val, sizeof(val));

    return 0;
}

/*
*   set otg voltage and current
*   voltage：
*   current_mA :
*/
int set_otg_vol_current(int voltage, int current_mA)
{
    unsigned char val[2]= {0};
    little_to_big(0x1000, val);
    if(1 == bq25703a_i2c_write(BQ_I2C_ADDR, 0x34, val, sizeof(val)))
    {
        return 1;
    }

    switch (voltage)
    {
        case OTG_VOL_1:
            little_to_big(0x0200, val);// 4992 mv
            break;

        case OTG_VOL_2:
            little_to_big(0x11C0, val);// 9024 mv
            break;

        case OTG_VOL_3:
            little_to_big(0x2940, val);// 15040 mv
            break;

        default:
            little_to_big(0x0200, val);// 5008 mv
            break;
    }

    if(1 == bq25703a_i2c_write(BQ_I2C_ADDR, 0x06, val, sizeof(val)))
    {
        return 1;
    }

    switch (current_mA)
    {
        case OTG_CUR_1:
            little_to_big(0x0400, val);// 1024 mA
            break;

        case OTG_CUR_2:
            little_to_big(0x0800, val);//  2048 mA
            break;

        case OTG_CUR_3:
            little_to_big(0x0BC0, val);// 3008 mA
            break;

        default:
            little_to_big(0x0400, val);// 1024 mA
            break;
    }

    if(1 == bq25703a_i2c_write(BQ_I2C_ADDR, 0x08, val, sizeof(val)))
    {
        return 1;
    }

    return 0;
}

/*tery and system voltage
*/
int bq25703a_battery_system_vol_read( int bs_vol[] )
{
    /*
    *   vol = 2880 + buf[1]*64
    */
    unsigned char buf[2] = {0};

    //buf[1] is 8-15 bit SYSTEM_VOL
    //buf[0] is 0-7 bit  Voltage_SYSTEM
    if(1 == bq25703a_i2c_read(BQ_I2C_ADDR, 0x2C, buf, sizeof(buf)))
    {
        return 1;
    }
    else
    {
        bs_vol[0] = 2880 + (int)buf[0] * 64;
        bs_vol[1] = 2880 + (int)buf[1] * 64;
    }

    return 0;
}
int bq25703a_input_current_read( int *bs_vol )
{
    unsigned char current = 0;

    //buf[1] is 8-15 bit SYSTEM_VOL
    //buf[0] is 0-7 bit  Voltage_SYSTEM
    if(1 == bq25703a_i2c_read(BQ_I2C_ADDR, 0x2B, &current, sizeof(current)))
    {
        return 1;
    }
    else
    {
        *bs_vol = (int)current * 50;
    }

    return 0;
}

void little_to_big(short int val, unsigned char *res_val)
{
    res_val[0] = val & 0x00ff; //low 8-bit
    res_val[1] = val >> 8; //high 8-bit
}

void *gpiox_irq_thread_function(void *arg)
{
    struct thread_argument *arg_thread;
    arg_thread = ( struct thread_argument * )arg;

    get_irq_gpiox(arg_thread->pin_number, arg_thread->do_what, arg_thread->voltage, arg_thread->current_mA);
    pthread_exit("gpiox_irq_thread_function exit");
}

int main(void)
{
    pthread_t thread1;
    int input_current = 0;
    i2c_open();

    bq25703a_charge_function_init(VOLTAGE, CURRENT_MA);

    //start irq thread
    struct thread_argument arg1;
    arg1.pin_number = CHG_OK_PIN;
    arg1.do_what = CHARGE_FUNCTION;
    arg1.voltage = VOLTAGE;
    arg1.current_mA = CURRENT_MA;

    pthread_create( &thread1, NULL, gpiox_irq_thread_function, (void*)&arg1 );

    while(1)
    {
        bq25703a_input_current_read(&input_current);
        printf("read input current: %d\n\n", input_current);
        sleep(2);
    }

    return 0;
}


