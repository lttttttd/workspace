#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define CHRDEV_NAME "/dev/ltd"
#define THREAD1 1
#define THREAD2 2
#define START 1
#define PAUSE 9
#define CONTINUE 3
#define STOP 4
#define STARTALL 5
#define PAUSEALL 6
#define CONTINUEALL 7
#define STOPALL 8

int main(int argc,char *argv[])
{
	int choice;
	int fd;
	fd=open(CHRDEV_NAME,O_RDONLY);
	if(fd<0){
		perror(argv[1]);
		return -1;
	}
	while(1){
		printf("1:Start all 2:Pause all 3:Continue all 4:Stop all\n");
		printf("5:Start thread1 6:Pause thread1 7:Continue thread1 8:Stop thread1\n");
		printf("9:Start thread2 10:Pause thread2 11:Continue thread2 12:Stop thread2\n");	
		printf("input yout choice:");
		scanf("%d",&choice);
		switch(choice){
		case 1:
			printf("Start all\n");
			ioctl(fd,STARTALL,THREAD1);
			break;
		case 2:
			printf("Pause all\n");
                        ioctl(fd,PAUSEALL,THREAD1);
                        break;
		case 3:
			printf("Continue all\n");
                        ioctl(fd,CONTINUEALL,THREAD1);
                        break;
		case 4:
			printf("Stop all\n");
                        ioctl(fd,STOPALL,THREAD1);
                        break;
		case 5:
			printf("Start thread1\n");
                        ioctl(fd,START,THREAD1);
                        break;
		case 6:
			printf("Pause thread1\n");
                        ioctl(fd,PAUSE,THREAD1);
                        break;
		case 7:
			printf("Continue thread1\n");
                        ioctl(fd,CONTINUE,THREAD1);
                        break;
		case 8:
			printf("Stop thread1\n");
                        ioctl(fd,STOP,THREAD1);
                        break;
		case 9:
                        printf("Start thread2\n");
                        ioctl(fd,START,THREAD2);
                        break;
                case 10:
                        printf("Pause thread2\n");
                        ioctl(fd,PAUSE,THREAD2);
                        break;
                case 11:
                        printf("Continue thread2\n");
                        ioctl(fd,CONTINUE,THREAD2);
                        break;
                case 12:
                        printf("Stop thread2\n");
                        ioctl(fd,STOP,THREAD2);
                        break;
		default:
			break;
		}
	}
	close(fd);
	return 0;

}
















































