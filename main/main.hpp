#ifndef MAIN_HPP
#define MAIN_HPP

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
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include<opencv2/imgproc/imgproc_c.h>
#include <cv.h>


//宏定义    
#define FALSE  -1    
#define TRUE   0

using namespace std;
using namespace cv;

/***********************************目标检测*************************/
/********************************************************************/
/*********************************************************************/


//目标信息
struct DetectInfo {
    /* 质心(x,y) */
    double x,y;
    /* 目标面积 */
    int area;

    friend bool operator>(const DetectInfo &d1, const DetectInfo &d2) {
        return d1.area > d2.area;
    }  
};

/* 目标检测函数--------------------------------------------------yyf
 * img: 待检测图像
 * area_threshold: 面积阈值
 * 输出目标信息数组
 */
vector<DetectInfo> detection(Mat background, Mat img, int area_threshold = 80) {
	Mat imgFront,imglabel, stats, centroids;
	Mat element = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
	absdiff(img, background, imgFront);
	threshold(imgFront, imgFront, 70, 255, CV_THRESH_BINARY);
	morphologyEx(imgFront, imgFront, CV_MOP_OPEN, element); //消除孤立的点
	int count = connectedComponentsWithStats(imgFront, imglabel, stats, centroids, 8);
	vector<DetectInfo> detectinfos(count);
	for(int i = 1; i != count; ++i){
		detectinfos[i].area = stats.at<int>(i, CC_STAT_AREA);
		detectinfos[i].x = centroids.at<double>(i, 0);
		detectinfos[i].y = centroids.at<double>(i, 1);
	}
	sort(detectinfos.begin(), detectinfos.end(), std::greater<DetectInfo>());
	for (int i = 0; i < count; ++i) {
		if (detectinfos[i].area < area_threshold) {
			detectinfos.resize(i);
			break;
		}
	}
	return detectinfos;
}



/********************************串口通信**************************/
/*****************************************************************/
/******************************************************************/
//串口数据结构体
struct ReceiveInfo {
        uint8_t flag1;
        uint8_t f_num;
        uint8_t t_h;
        uint8_t t_m;
        uint8_t t_s;
        uint8_t t_ms;
 	uint8_t flag2;    
} __attribute__((packed));


/*******************************************************************  
    * 名称：            UART0_Recv  
    * 功能：            接收串口数据  
    * 入口参数：        fd         文件描述符      
    *                   rcv_buf    接收串口中数据存入rcv_buf缓冲区中  
    *                   data_len   一帧数据的长度  
    * 出口参数：        正确返回为1，错误返回为0  
    *******************************************************************/  
int UART0_Recv(int fd, char *rcv_buf,int data_len)    
    {    
        int len,fs_sel;    
        fd_set fs_read;    
           
        struct timeval time;    
           
        FD_ZERO(&fs_read);    
        FD_SET(fd,&fs_read);    
           
        time.tv_sec = 0;    
        time.tv_usec = 12000;    
           
        //使用select实现串口的多路通信    
        fs_sel = select(fd+1,&fs_read,NULL,NULL,&time);    
        if(fs_sel)    
        {    
            len = read(fd,rcv_buf,data_len);    
            return len;    
        }    
        else    
        {    
            return FALSE;    
        }         
    } 



/********************************************************************  
    * 名称：            UART0_Send  
    * 功能：            发送数据  
    * 入口参数：        fd           文件描述符      
    *                   send_buf     存放串口发送数据  
    *                   data_len     一帧数据的个数  
    * 出口参数：        正确返回为1，错误返回为0  
    *******************************************************************/    
bool UART0_Send(int fd)    
{    
	int len = 0;
        char start_buf[] = {0x01};	
        len = write(fd, start_buf, sizeof(start_buf));    
	if(len != sizeof(start_buf))
        {    
            	printf("Error\n");           
            	tcflush(fd,TCOFLUSH);    
            	return false;    
        }	    
	else
	{
		printf("Send message sucessed!\n");
		return true;
	}

           
}    






 
//串口初始化
int serialport_inti()
{

	/*打开串口*/
	int fd = open( "/dev/ttyTHS2", O_RDWR|O_NOCTTY|O_NDELAY);//   串口号(ttyS0,ttyS1,ttyS2)
	//int fd = open( "/dev/ttyS0", O_RDWR|O_NOCTTY|O_NDELAY);
	if (fd<0)    
        {    
            perror("Can't Open Serial Port");   
	    return(FALSE);     
        }  
 	if(fcntl(fd, F_SETFL, 0) < 0)    
        {    
            printf("fcntl failed!\n");  
	    return(FALSE);     
        }    
	if(0 == isatty(STDIN_FILENO))    
        {    
            printf("standard input is not a terminal device\n");    
            return(FALSE);    
        }    
        else    
        {    
            printf("isatty success!\n");    
        }                  
        printf("fd->open=%d\n",fd);


	/*设置串口参数*/
	int speed = 115200;//比特率
	int flow_ctrl = 0;//流控制
	int databits = 8;//数据位
	int stopbits = 1;//停止位
	int parity = 'N';//效验类型 取值为N,E,O,,S
  
	int   i;    
        int   status;    
        int   speed_arr[] = { B115200, B19200, B9600, B4800, B2400, B1200, B300};    
        int   name_arr[] = {115200,  19200,  9600,  4800,  2400,  1200,  300};    
                 
        struct termios options;    
           
        /*  tcgetattr(fd,&options)得到与fd指向对象的相关参数，并将它们保存于options,该函数还可以测试配置是否正确，
            该串口是否可用等。若调用成功，函数返回值为0，若调用失败，函数返回值为1.  */    
        if( tcgetattr( fd,&options)  !=  0)    
        {    
            perror("SetupSerial 1");        
            return(FALSE);     
        }    
          
        //设置串口输入波特率和输出波特率    
        for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++)    
        {    
            if  (speed == name_arr[i])    
            {                 
                cfsetispeed(&options, speed_arr[i]);     
                cfsetospeed(&options, speed_arr[i]);      
            }    
        }         
           
        //修改控制模式，保证程序不会占用串口    
        options.c_cflag |= CLOCAL;    
        //修改控制模式，使得能够从串口中读取输入数据    
        options.c_cflag |= CREAD;    
          
        //设置数据流控制    
        switch(flow_ctrl)    
        {    
              
            case 0 ://不使用流控制    
                  options.c_cflag &= ~CRTSCTS;    
                  break;       
              
            case 1 ://使用硬件流控制    
                  options.c_cflag |= CRTSCTS;    
                  break;    
            case 2 ://使用软件流控制    
                  options.c_cflag |= IXON | IXOFF | IXANY;    
                  break;    
        }    
        //设置数据位    
        //屏蔽其他标志位    
        options.c_cflag &= ~CSIZE;    
        switch (databits)    
        {      
            case 5    :    
                         options.c_cflag |= CS5;    
                         break;    
            case 6    :    
                         options.c_cflag |= CS6;    
                         break;    
            case 7    :        
                     options.c_cflag |= CS7;    
                     break;    
            case 8:        
                     options.c_cflag |= CS8;    
                     break;      
            default:       
                     fprintf(stderr,"Unsupported data size\n");    
                     return (FALSE);     
        }    
        //设置校验位    
        switch (parity)    
        {      
            case 'n':    
            case 'N': //无奇偶校验位。    
                     options.c_cflag &= ~PARENB;     
                     options.c_iflag &= ~INPCK;        
                     break;     
            case 'o':      
            case 'O'://设置为奇校验        
                     options.c_cflag |= (PARODD | PARENB);     
                     options.c_iflag |= INPCK;                 
                     break;     
            case 'e':     
            case 'E'://设置为偶校验      
                     options.c_cflag |= PARENB;           
                     options.c_cflag &= ~PARODD;           
                     options.c_iflag |= INPCK;          
                     break;    
            case 's':    
            case 'S': //设置为空格     
                     options.c_cflag &= ~PARENB;    
                     options.c_cflag &= ~CSTOPB;    
                     break;     
            default:      
                     fprintf(stderr,"Unsupported parity\n");        
                     return (FALSE);     
        }     
        // 设置停止位     
        switch (stopbits)    
        {      
            case 1:       
                     options.c_cflag &= ~CSTOPB; break;     
            case 2:       
                     options.c_cflag |= CSTOPB; break;    
            default:       
                           fprintf(stderr,"Unsupported stop bits\n");     
                           return (FALSE);    
        }    
           
        //修改输出模式，原始数据输出    
        options.c_oflag &= ~OPOST;    
          
        options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);    
        //options.c_lflag &= ~(ISIG | ICANON);    
           
        //设置等待时间和最小接收字符    
        options.c_cc[VTIME] = 0.1; /* 读取一个字符等待1*(1/10)s */      
        options.c_cc[VMIN] = 1; /* 读取字符的最少个数为1 */    
           
        //如果发生数据溢出，接收数据，但是不再读取 刷新收到的数据但是不读    
        tcflush(fd,TCIFLUSH);    
           
        //激活配置 (将修改后的termios数据设置到串口中）    
        if (tcsetattr(fd,TCSANOW,&options) != 0)      
        {    
            perror("com set error!\n");      
            return (FALSE);     
        }

return fd;
 
}	


void set_system_time()
{
	int fd = serialport_inti();//初始化串口
	char rcv_buf[10];
	ReceiveInfo *rcv_info;
	
	while(true){
		if(UART0_Send(fd))
			printf("Send Message Sucessed!\n");
		else
			printf("Send Message Failed!\n");
		
		int len = UART0_Recv(fd, rcv_buf,sizeof(ReceiveInfo));    
        	if(len < 0){    
	
                    	printf("Cannot Receive Data!\n");    
                }    
		else{
			rcv_info = reinterpret_cast<ReceiveInfo *>(rcv_buf);
			if(rcv_info->t_h >= 0 && rcv_info->t_h <= 24 && 
				rcv_info->t_m >= 0 && rcv_info->t_m < 60 && 
				rcv_info->t_s >= 0 && rcv_info->t_s < 60 && 
				rcv_info->t_ms >= 0 && rcv_info->t_ms < 1000 ){

				struct tm *time_p = new struct tm();
				struct timeval time_tv;
				gettimeofday(&time_tv, NULL);
				time_p->tm_hour = rcv_info->t_h;
				time_p->tm_min = rcv_info->t_m;
				time_p->tm_sec = rcv_info->t_s;
				time_t time_sec = mktime(time_p);
				delete(time_p);
				time_tv.tv_sec = time_sec;
				time_tv.tv_usev = rcv_info->t_ms * 1000;

				if(settimeofday(&time_tv,NULL) < 0){
					printf("Time Setting Failed!\n");
				}
				else{
					printf("Time Setting Sucessed!\n");
					close(fd);
					break;
				}
			}
		}
		sleep(1);
	}	
	return NULL;
}		
    
/***********************************UDP网口通信**************/
/************************************************************/
/************************************************************/
struct SendInfo {
        uint8_t flag1 = 0xAB;
	uint8_t f_num;
        uint8_t t_h;
        uint8_t t_m;
        uint8_t t_s;
        uint16_t t_ms;
        uint16_t x1; 
        uint16_t y1;
	uint8_t flag2 = 0xBB;    
} __attribute__((packed));






#endif /* ifndef MAIN_HPP */

