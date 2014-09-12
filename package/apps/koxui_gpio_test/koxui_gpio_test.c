#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
void main(void)
{
	int fd=0;
	int i=3;
	fd = open("/dev/koxui_gpio_test",O_RDWR);
	if(!write(fd,&i,sizeof(i))){
		printf("write success\n");
		close(fd);
	}else{
		printf("write fail\n");
		close(fd);
	}
}
