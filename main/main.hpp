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

struct ReceiveInfo {//北斗数据
        uint8_t flag1;
        uint8_t f_num;
        uint8_t t_h;
        uint8_t t_m;
        uint8_t t_s;
        uint8_t t_ms;
 	uint8_t flag2;    
} __attribute__((packed));

struct SendInfo {//发送给上位机的数据
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

struct DetectInfo {//目标信息
    double x,y;
    int area;
    friend bool operator>(const DetectInfo &d1, const DetectInfo &d2) {
        return d1.area > d2.area;
    }  
};

struct SendData {//时间信息
    uint8_t t_h, t_m, t_s;
    uint16_t t_ms; //unit: 1ms
};

void init_time(const struct ReceiveInfo *recv);//初始化时间

void get_remote_time(struct SendData *send);//时间换算

vector<DetectInfo> detection(Mat background, Mat img, int area_threshold = 80);//目标检测

int UART0_Recv(int fd, char *rcv_buf,int data_len);//串口接收    

bool UART0_Send(int fd);//串口发送

 
int serialport_inti();//串口初始化

void open_video_flow(int fd);//开启北斗和视频流

void close_video_flow(int fd);//关闭北斗和视频流

void set_system_time();//北斗对时
    

#endif /* ifndef MAIN_HPP */

