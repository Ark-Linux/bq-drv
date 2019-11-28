/**
*  @file      gpio_config.c
*  @brief     gpio_config 
*  @author    Zack Li
*  @date      11 -2019
*  @copyright
*/

#include "gpio_config.h"

int register_gpiox(int pin_number)
{
	FILE *p=NULL;
	char *open_path = "/sys/class/gpio/export";
	p = fopen(open_path,"w");
	if (p == NULL)
		{
			printf("open '%s' error\n",open_path);
			return 1;
		}
	
	if (fprintf(p,"%d",pin_number) < 0)
		{
			printf("write '%s' error\n",open_path);
			return 1;
		}
	fclose(p);

	return 0;
}

int unregister_gpiox(int pin_number)
{
	FILE *p=NULL;
	char *open_path = "/sys/class/gpio/unexport";
	p = fopen(open_path,"w");
	if (p == NULL)
		{
			printf("open '%s' error\n",open_path);
			return 1;
		}
	
	if (fprintf(p,"%d",pin_number) < 0)
		{
			printf("write '%s' error\n",open_path);
			return 1;
		}
	fclose(p);

	return 0;
}

int set_direction(int pin_number, char *direction)
{
	FILE *p=NULL;
	char open_path[64]={0};
	
	sprintf(open_path, "/sys/class/gpio/gpio%d/direction",pin_number);
	p = fopen(open_path,"w");
	if (p == NULL)
		{
			printf("open '%s' error\n",open_path);
			return 1;
		}
	
	if (fprintf(p,"%s",direction) < 0)
		{
			printf("write '%s' error\n",open_path);
			return 1;
		}
	fclose(p);

	return 0;
}

int set_edge(int pin_number, char *edge)
{
	FILE *p=NULL;
	char open_path[64]={0};
	
	sprintf(open_path, "/sys/class/gpio/gpio%d/edge",pin_number);
	p = fopen(open_path,"w");
	if (p == NULL)
		{
			printf("open '%s' error\n",open_path);
			return 1;
		}
	
	if (fprintf(p,"%s",edge) < 0)
		{
			printf("write '%s' error\n",open_path);
			return 1;
		}
	fclose(p);
	
	return 0;
}

int set_value(int pin_number, int value)
{
	FILE *p=NULL;
	char open_path[64]={0};
	
	sprintf(open_path, "/sys/class/gpio/gpio%d/value", pin_number);
	p = fopen(open_path,"w");
	if (p == NULL)
		{
			printf("open '%s' error\n", open_path);
			return 1;
		}
	
	if (fprintf(p,"%d",value) < 0)
		{
			printf("write '%s' error\n", open_path);
			return 1;
		}
	fclose(p);
	
	return 0;	
}

int set_gpiox_high(int pin_number){
	if (register_gpiox(pin_number) == 1){
		return 1;
	}
	if (set_direction(pin_number, "out") == 1){
		return 1;
	}
	if (set_value(pin_number, 1) == 1){
		return 1;
	}
	if (unregister_gpiox(pin_number) == 1){
		return 1;
	}
	return 0;
}

int set_gpiox_low(int pin_number){
	if (register_gpiox(pin_number) == 1){
		return 1;
	}
	if (set_direction(pin_number, "out") == 1){
		return 1;
	}
	if (set_value(pin_number, 0) == 1){
		return 1;
	}
	if (unregister_gpiox(pin_number) == 1){
		return 1;
	}
	return 0;
}


int get_irq_gpiox(int pin_number)
{
	char file_path[64]={0};
	register_gpiox(pin_number);
	set_direction(pin_number, "in");
	set_edge(pin_number, "both");
	
	sprintf(file_path, "/sys/class/gpio/gpio%d/value", pin_number);
	int fd = open(file_path, O_RDONLY);
    if(fd < 0)
    {
        printf("open %s failed!\n", file_path);  
        return -1;
    }

	struct pollfd fds[1];
    fds[0].fd=fd;
    fds[0].events=POLLPRI;
	
    while(1)
    {
        if(poll(fds, 1, 0) == -1) //the last argumenxt is timeout , 0->不阻塞, x->x毫秒
        {
            perror("poll failed!\n");
            return -1;
        }
        if(fds[0].revents&POLLPRI)
        {
            if(lseek(fd, 0, SEEK_SET) == -1)
            {
                perror("lseek failed!\n");
                return -1;
            }
            char buffer[2];
            int len;
            if((len=read(fd, buffer, sizeof(buffer))) == -1)
            {
                perror("read failed!\n");
                return -1;
            }
            buffer[len]=0;
            printf("\n\n\n %d thread irq read: %s\n\n\n", pin_number, buffer);
			//do something
			
			set_charge_voltage_current();
			printf("\n\n\n CHARGE_FUNCTION \n\n\n");
		
        }
		
    }	
	unregister_gpiox(pin_number);
	return 0;
}




