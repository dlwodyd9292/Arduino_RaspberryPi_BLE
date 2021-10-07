#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <pthread.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

#define BT_CH 1
#define BUF_SIZE 128
//#define DEBUG 

void* thread_btsend(void* arg);
void* thread_btrecv(void* arg);


int main(int argc, char** argv){
    struct sockaddr_rc addr = { 0 };
    int btsfd, ret;
	char dest[18] = "00:20:04:BD:D1:F5";		//hc-06 chip MAC address
	int i;
	char buf[128];
    pthread_t id_t1, id_t2;

    if (argc > 2) {
        printf("Usage:\n");
        printf("%s [MacAddress]\n",argv[0]);
        exit(1);
    }
	if(argc == 2)
		strcpy(dest,argv[1]);
    // allocate a socket

    btsfd = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
	if(btsfd < 0){
		perror("socket()");
		exit(1);
	}
	
    // set the connection parameters (who to connect to)
    addr.rc_family = AF_BLUETOOTH;
    addr.rc_channel = (uint8_t) BT_CH;
    str2ba(dest, &addr.rc_bdaddr);

    // connect to server
    ret = connect(btsfd, (struct sockaddr *)&addr, sizeof(addr));
	if(ret < 0){
		perror("connect()");
		exit(1);
	}
    pthread_create(&id_t1, NULL, thread_btsend, (void *)&btsfd);
    pthread_create(&id_t2, NULL, thread_btrecv, (void *)&btsfd);
	pthread_join(id_t1,NULL);
	pthread_join(id_t2,NULL);

    close(btsfd);
    return 0;
}
void* thread_btsend(void* arg)  //keyboard -> bluetooth
{
	int ret;
	char buf[BUF_SIZE];
	int btfd = *((int *)arg);
	puts("Bluetooth On. Press 1 or 0 to blink..");
	do {
		ret = read(STDIN_FILENO,buf,BUF_SIZE);	
		buf[ret-1] = '\0';
		if(!strcmp(buf,"q"))
			exit(1);
		if(strcmp(buf,"0") && strcmp(buf,"1"))
		{
			puts("Command Error!! Press 1 or 0 to blink..");
			continue;
		}
#ifdef DEBUG
		printf("%s %d %s\n",__func__, ret, buf);
#endif
		write(btfd,buf,ret);
	}while(1);
	return NULL;
}
void* thread_btrecv(void* arg)   //bluetooth -> terminal
{
	int ret;
	int total=0;
	char buf[BUF_SIZE];
	int btfd = *((int *)arg);
	do {
		ret = read(btfd,buf+total,BUF_SIZE);
//		ret = read(btfd,&buf[total],BUF_SIZE);
		if(ret > 0)
		{
			total = total + ret;	
		}
		if(buf[total-1] == '\n')	
		{
			buf[total-1] = '\0';
			total = 0;
		}
		else
			continue;
#ifdef DEBUG
		printf("%s %d %s\n",__func__, ret, buf);
#endif
		fputs(buf,stdout);
	}while(1);
	return NULL;
}
