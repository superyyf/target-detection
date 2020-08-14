#include <sys/types.h>
#include <sys/socket.h>
#include<stdio.h>      /*标准输入输出定义*/
#include<stdlib.h>     /*标准函数库定义*/
#include<unistd.h>     /*Unix 标准函数定义*/
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>      /*文件控制定义*/
#include<termios.h>    /*PPSIX 终端控制定义*/
#include<errno.h>      /*错误号定义*/
#include<string.h>
#include<vector>
#include<stdint.h>
#include<netinet/in.h>
#include<arpa/inet.h>
struct SendInfo {
        uint8_t flag1 = 0xAB;
        uint8_t flag2;
        uint8_t f_num;
        uint8_t t_h;
        uint8_t t_m;
        uint8_t t_s;
        uint8_t t_ms;
        uint16_t x1; 
        uint16_t y1; 
        uint8_t flag3 = 0xBB;    
} __attribute__((packed));




//-----------------------------UDP传输目标信息---------------------
void Net_Send_new(int sockClient, struct sockaddr_in addrSrv,  SendInfo *data_pack)
{

        data_pack->flag2 = 0x00;//标志位 有无检测到目标
        if (data_pack->x1 != 0)
        {
                data_pack->flag2 = 0x01;
        }
        unsigned size = sizeof(data_pack);
        int set = sendto(sockClient, &data_pack, size, 0, (struct sockaddr*)&addrSrv, sizeof(struct sockaddr));
}

int main(){
     	//初始化套接字init socket       
        int sockClient = socket(AF_INET, SOCK_DGRAM, 0); 
       	if (sockClient == -1) 
       	{
      	        printf("socket error!");
       	        return 0;
       	}
       	struct sockaddr_in addrSrv;
       	addrSrv.sin_addr.s_addr = inet_addr("192.168.1.11");//ip地址重要！！！Srv IP is "192.168.1.10"
        addrSrv.sin_family = AF_INET;
        addrSrv.sin_port = htons(10011);//重要！！！端口编号10011

	SendInfo sendinfos;
        sendinfos.f_num = 100;
        sendinfos.t_h = 12;
        sendinfos.t_m = 30;
        sendinfos.t_ms = 800;
        sendinfos.x1 = 256;
        sendinfos.y1 = 320;
	while(1){
        	Net_Send_new(sockClient, addrSrv, &sendinfos);
		printf("sending......\n");
		sleep(1);
	}
	close(sockClient);
}
