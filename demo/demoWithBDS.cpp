#include <stdio.h>		// c library
#include <signal.h>		// c library
#include <stdlib.h>		// c library
#include <stdarg.h>		// c library
#include <unistd.h>		// c library
#include <limits.h>		// c library

#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <malloc.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <stdarg.h>
#include <fcntl.h>
#include <stdint.h>
#include <vector>

#include<sys/socket.h>
#include<netinet/in.h>

#include<iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include<opencv2/imgproc/imgproc_c.h>
#include <cv.h>
#include <memory>
#include "main.hpp"
#include "queue.hpp"

#include<pthread.h>
#include<chrono>

using namespace cv;
using namespace std;

typedef void *(*THREAD_FUNC)(void *);

struct ImageData {
	Mat image;
	int frame_num;
};

struct TargetData{
	unsigned short x;
	unsigned short y;
	int frame_num;
	uint8_t t_h;
	uint8_t t_m;
	uint8_t t_s;
	uint16_t t_ms;
};

template<typename Input, typename Output> struct Pipe{
	Queue<Input> *input;
	Queue<Output> *output;
	Pipe(Queue<Input> *a, Queue<Output> *b):input(a), output(b){}
};

static void hello(void){
	printf("\n\n");
	printf("This program is running on ARM LINUX & NVIDA Terga GPU.\n" );
	printf("This program is a demo.\n");
	printf("\n\n");
	
	printf("输入任意键启动程序：\n");
	char ch = getchar();
	return;
}


#define XDIM 640 //width
#define YDIM 512 //height
#define COLORS 1
#define AOI_X 63
#define AOI_Y 133
#define AOI_WIDTH 320
#define AOI_HIGH 256
#define AREA_THRESHOLD  50

bool end_flag = false;
bool gpu_load_flag = false;
void sign_handle(int sign)
{
        end_flag = true;
}


void *img_enhance_thread(Queue<ImageData> *q)
{
	int FrameNum = 0;
	char prefix[] = "/media/nvidia/Elements SE/太原实验数据/8bits/第五发处理完(6.2km)/frame_";
	char postfix[] = ".png";
	char filename[255];


	struct timeval start_1, end_1;
	for(int k = 2000; k<=3999; k++)
	{
		gettimeofday(&start_1, NULL);
		FrameNum++;
		sprintf(filename, "%s%d%s", prefix, k, postfix);	
		Mat src  = imread(filename,IMREAD_GRAYSCALE);//原始图像
		if(!src.data || k <= 3)
		{
			continue;
		}
		


		ImageData imgdata;
		imgdata.image = src.clone();
		imgdata.frame_num = FrameNum;
		q->push(move(imgdata));
		
		usleep(5000);
                if(end_flag)//信号标志位
                {
                        break;
                }
		while(!gpu_load_flag){
			printf("Loading...\n");
			sleep(1);
		}

		gettimeofday(&end_1, NULL);
		printf("Image Enhance = %fms-------------------------------------------------------\n", (double)((end_1.tv_usec - start_1.tv_usec)/1000));
	
	}
	printf("\n------------------------------------结束图像增强线程-------------------------------\n");
	q->end();
	return NULL;
}
	
	



void *image_process_thread(Pipe<ImageData, TargetData> *p1)
{
	/*
	char prefix[] = "/home/nvidia/pic/target_";
	char postfix[] = ".png";
	char filename[255];
	int target_count = 0;
	int detect_num = 0;
	*/
	unsigned short x1 = 0;
	unsigned short y1 = 0;
	
	Scalar color(0, 0, 255);

	struct timeval start_2, end_2, time_target;
	struct tm* tm_target;
	while(true)
	{

		unique_ptr<ImageData> imgdata;
		imgdata = p1->input->pop();

		gettimeofday(&start_2, NULL);
		if(imgdata == NULL)
		{
			p1->output->end();
			break;
		}
		Mat image_pro = imgdata->image;
		int frame_num = imgdata->frame_num;
		 

		TargetData targetdata;

		//目标检测
 		Point point = cuda_detection(image_pro, AREA_THRESHOLD);

		if(point.x != 0)
		{
			x1 = point.x;
			y1 = point.y;	

			SendData timedata;
			get_remote_time(&timedata);

			targetdata.t_h = timedata.t_h;
			targetdata.t_m = timedata.t_m;
			targetdata.t_s = timedata.t_s;
			targetdata.t_ms = timedata.t_ms;

			drawMarker(image_pro,point,color);

			printf("target_th = %d\ntarget_tm = %d\ntarget_ts = %d\ntarget_tms = %d\n", targetdata.t_h, targetdata.t_m, targetdata.t_s, targetdata.t_ms);
			
		}
		else
		{
			x1 = 0;
			y1 = 0;
		}

		imshow("Frame", image_pro);
		cvWaitKey(1);		

		if(gpu_load_flag == false){
			gpu_load_flag = true;
		}

		targetdata.x = x1;
		targetdata.y = y1;
		targetdata.frame_num = frame_num;
		p1->output->push(move(targetdata));
		
		gettimeofday(&end_2, NULL);
		printf("--------------------Image Process = %fms-----------------------------------\n", (double)(end_2.tv_usec - start_2.tv_usec)/1000);
	}
	printf("\n------------------------------------------结束目标检测线程----------------------------------\n");
	return NULL;
}


void *send_data_thread(Queue<TargetData> *t)
{
			
	//初始化套接字init socket 	
	
	int sockClient = socket(AF_INET, SOCK_DGRAM, 0);//初始化socket
	if (sockClient == -1){
		printf("socket error!");
		return NULL;
	}
	struct sockaddr_in addrSrv;
	addrSrv.sin_addr.s_addr = inet_addr("192.168.1.13");//ip地址重要！！！Srv IP is "192.168.1.10"
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(10016);//重要！！！端口编号10011
	struct timeval start_3, end_3;
	while(true){
		gettimeofday(&start_3, NULL);
		unique_ptr<TargetData> targetdata;
		targetdata = t->pop();
		
		if(targetdata == NULL){
			break;
		}

		if(targetdata->x != 0){
			SendInfo sendinfos;
			sendinfos.f_num = targetdata->frame_num;
			sendinfos.t_h = targetdata->t_h;
			sendinfos.t_m = targetdata->t_m;
			sendinfos.t_s = targetdata->t_s;
			sendinfos.t_ms = targetdata->t_ms;
			
			printf("f_num : %d\nt_h : %d\nt_m : %d\nt_s : %d\nms : %d\n", sendinfos.f_num, sendinfos.t_h, sendinfos.t_m, sendinfos.t_s, sendinfos.t_ms);
			sendinfos.x1 = targetdata->x;
			sendinfos.y1 = targetdata->y;
			if(int set = sendto(sockClient, &sendinfos, sizeof(sendinfos), 0, (struct sockaddr*)&addrSrv, sizeof(struct sockaddr)) < 0){
				perror("UDP Error ");
			}
				
		}
		gettimeofday(&end_3, NULL);
		printf("----------------------------------------------Send Data = %fms\n", (double)(end_3.tv_usec - start_3.tv_usec)/1000);
	}
	printf("\n------------------------------------------结束发送线程-----------------------------------------\n");
	close(sockClient);//关闭socket
	return NULL;
}
int main(void)
{
	hello();
	set_system_time();

        struct sigaction signinfo;//信号处理：Ctr+C结束图像增强线程
        signinfo.sa_handler = sign_handle;
        signinfo.sa_flags = SA_RESETHAND;
        sigemptyset(&signinfo.sa_mask);
        sigaction(SIGINT, &signinfo, NULL);



	Queue<ImageData> imagedata;
	Queue<TargetData> targetdata;
	Pipe<ImageData, TargetData> p1(&imagedata, &targetdata);

	pthread_t t1, t2, t3;
	pthread_create(&t1, NULL, (THREAD_FUNC)img_enhance_thread, &imagedata);
	pthread_create(&t2, NULL, (THREAD_FUNC)image_process_thread, &p1);
	pthread_create(&t3, NULL, (THREAD_FUNC)send_data_thread, &targetdata); 

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	pthread_join(t3, NULL);

	return 0;	
}




