#if !defined(FORMAT) && !defined(FORMATFILE)

#define FORMATFILE	"/usr/local/xcap/data/ddd.fmt"	  // using format file saved by XCAP
#endif

/*
 *  2.1) Set number of expected PIXCI(R) image boards.
 *  The XCLIB Simple 'C' Functions expect that the boards are
 *  identical and operated at the same resolution.
 *
 *  For PIXCI(R) frame grabbers with multiple, functional units,
 *  the XCLIB presents the two halves of the
 *  PIXCI(R) E1DB, E4DB, E8CAM, E8DB, e104x4-2f, ECB2, EL1DB, ELS2, SI2, or SV7 frame grabbers,
 *  or the three parts of the PIXCI(R) e104x4-f2b frame grabber,
 *  or the four quarters of the PIXCI(R) e104x4-4b or SI4 frame grabbers,
 *  as two, three, or four independent PIXCI(R) frame grabbers, respectively.
 *
 *  This example expects only one unit.
 */
#if !defined(UNITS)
#define UNITS	1
#endif
#define UNITSMAP    ((1<<UNITS)-1)  // shorthand - bitmap of all units


 /*
  *  2.2) Optionally, set driver configuration parameters.
  *  These are normally left to the default, "".
  *  The actual driver configuration parameters include the
  *  desired PIXCI(R) frame grabbers, but to make configuation easier,
  *  code, below, will automatically add board selection to this.
  *
  *  Note: Under Linux, the image frame buffer memory can't be set as
  *  a run time option. It MUST be set via insmod so the memory can
  *  be reserved during Linux's initialization.
  */
#if !defined(DRIVERPARMS)
  //#define DRIVERPARMS "-QU 0"       // don't use interrupts
  //#define DRIVERPARMS "-IM 8192"    // request 8192 mbyte of frame buffer memory
#define DRIVERPARMS ""	      // default
#endif


/*
 *  3)	Choose whether the optional PXIPL Image Processing Library
 *	is available.
 */
#if !defined(USE_PXIPL)
#define USE_PXIPL	0
#endif


 /*
  *  4) Select directory for saving of images.
  *  This example will not overwrite existing images files;
  *  so directory selection is not critical.
  */
#if !defined(IMAGEFILE_DIR)
#define IMAGEFILE_DIR    "."
#endif



  /*
   *  NECESSARY INCLUDES:
   */
#include <stdio.h>		// c library
#include <signal.h>		// c library
#include <stdlib.h>		// c library
#include <stdarg.h>		// c library
#include <unistd.h>		// c library
#include <limits.h>		// c library
#include "xcliball.h"		// function prototypes
#if USE_PXIPL
#include "pxipl.h"		// function prototypes
#endif

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

static void user(char *mesg)
{
	if (mesg && *mesg)
		printf("%s\n", mesg);
	fprintf(stderr, "\n\nContinue (Key ENTER) ?");
	while (getchar() != '\n');
	fprintf(stderr, "\n");
}

/*
 * Close the PIXCI(R) frame grabber
 */
static void do_close(void)
{
	pxd_PIXCIclose();
	user("PIXCI(R) frame grabber closed");
}

/*
 * Here and elsewhere: we declare as 'static'
 * to avoid compiler's 'missing prototype' warnings.
 */
static void hello(void)
{
	printf("\n\n");
	printf("PIXCI(R) Frame Grabbers -  XCLIB 'C' Library\n");
	printf("XCLIBEL1.C - Example program\n");
	printf("Copyright (C)  2004-2017  EPIX, Inc.  All rights reserved.\n");
	printf("\n");
	printf("This program is best used by executing this program\n");
	printf("one step at a time, while simultaneously reading\n");
	printf("the XCLIB documentation and program listing.\n");
	user("");
}

/*
 *  MAIN:
 *
 *  Each library function is demonstrated in its own subroutine,
 *  the main calls each subroutine to produce the interactive demonstration.
 *
 *  It is suggested that this program source be read at the same time
 *  as the compiled program is executed.
 *
 */
#define XDIM 640 //width
#define YDIM 512 //height
#define COLORS 1
#define AOI_X 160
#define AOI_Y 128
#define AOI_WIDTH 320
#define AOI_HIGH 256
#define AREA_THRESHOLD  50

uchar transf_fun[16384] = { 0 };//映射关系数组

bool end_flag = false;
void sign_handle(int sign)
{
	end_flag = true;
}


void *img_enhance_thread(Queue<ImageData> *q)
{
	

	//定义存储16bits前景图像的缓冲区
	static ushort   colorimage_buf1[YDIM*XDIM*COLORS];
	//设置AOI感兴趣区域，一般不要修改
	int     i = 0;
	int     j = 0;
	int     cx = 0;	// left coordinate of centered AOI
	int     cy = 0;	// top	coordinate of centered AOI
	
	int nr = 512;//行
	int nc = 640;//列
	int total = nr*nc;//像素数
	int FrameNum = 0; //帧数
	namedWindow("Frame");

	//pxd_doSnap(UNITSMAP, 1, 0); //保存单张图片
	//pxd_readushort函数捕捉16bits数据到缓冲区，成功i>0,i<0失败，函数中止运行
	//捕捉16bits图像到缓冲区colorimage_buf1
	//pxd_goneLive函数源源不断的捕获图像，手册有介绍
	pxd_goLive(UNITSMAP, 1);
	pxd_readushort(UNITSMAP, 1, cx, cy, cx + XDIM, cy + YDIM, colorimage_buf1, sizeof(colorimage_buf1) / sizeof(ushort), "Grey");

	struct timeval start_1, end_1, start_p;
	gettimeofday(&start_1, NULL);
	while (pxd_goneLive(UNITSMAP, 0))//capture picture
	{
		gettimeofday(&start_p, NULL);
		if ((i = pxd_readushort(UNITSMAP, 1, cx, cy, cx + XDIM, cy + YDIM, colorimage_buf1, sizeof(colorimage_buf1) / sizeof(ushort), "Grey")) != XDIM * YDIM*COLORS) {/*xiugai*/
			if (i < 0)
				printf("pxd_readuchar: %s\n", pxd_mesgErrorCode(i));
			else
				printf("pxd_readuchar error: %d != %d\n", i, XDIM*YDIM * 3);
			user("");
			break;
		}
		//将colorimage_buf1中的16bits数据赋值给Mat矩阵
		Mat src(YDIM, XDIM, CV_16UC1, colorimage_buf1);//原始图像
		
		Mat dst_2(512, 640, CV_8UC1);
		ushort *p_1 = NULL;
		uchar * p_2 = NULL;
        	//uchar img_con[512*640];
		for (int i = 0; i < 512; i++)
		{
			//获取第i行像素数组首指针
			p_2 = dst_2.ptr<uchar>(i);
			p_1 = src.ptr<ushort>(i);
			//根据映射关系将原图像灰度替换成直方图均衡后的灰度
			for (int j = 0; j < 640; j++)
			{
				p_2[j] = transf_fun[p_1[j]];
                        	//img_con[i*640+j] = transf_fun[p_1[j]];
			}
		}	
		FrameNum++;
		ImageData imgdata;
		imgdata.image = dst_2.clone();
		while(true)
		{	
			imshow("Frame",imgdata.image);
			if(cvWaitKey(10) & 0xff == ord('q'))
				break;
		}
		cvDestroyAllWindows();
		imgdata.frame_num = FrameNum;
		q->push(move(imgdata));

		if(end_flag)//信号标志位
		{
			break;
		}

		gettimeofday(&end_1, NULL);
		printf("Image Enhance = %fms / %fms-------------------------------------------------------\n", (double)((end_1.tv_usec - start_p.tv_usec)/1000), (double)((1000000*(end_1.tv_sec - start_1.tv_sec)+(end_1.tv_usec - start_1.tv_usec))/1000/FrameNum));
	
	}
	printf("\n------------------------------------结束图像增强线程-------------------------------\n");
	q->end();
	do_close();//关闭视频流
	return NULL;
}



void *image_process_thread(Pipe<ImageData, TargetData> *p1)
{
	char prefix[] = "/home/nvidia/pic/target_";
	char postfix[] = ".png";
	char filename[255];
	int target_count = 0;
	int detect_num = 0;

	unsigned short x1 = 0;
	unsigned short y1 = 0;
	bool update_flag = true;
	time_t timefinal;
	unsigned short time_ms;
	struct timeval start_2, end_2, time_end;
	Mat img_back(512, 640, CV_8UC1);
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
		 
		//背景初始化
		if(update_flag)
		{
			img_back = image_pro.clone();
			printf("\n--------------------------------------背景初始化-----------------------------\n");
			update_flag = false;
		}

		TargetData targetdata;

		//目标检测
		vector<DetectInfo> detect_infos = detection(img_back, image_pro, AREA_THRESHOLD);
		if(detect_infos.size())
		{
			detect_num++;
			target_count++;
			x1 = (unsigned short)detect_infos[0].x;
			y1 = (unsigned short)detect_infos[0].y;
			printf("Target : [ %d , %d ]\n", x1, y1);
			sprintf(filename, "%s%d%s", prefix, target_count,postfix);
			imwrite(filename, image_pro);
			//SendData sendata;
			//get_remote_time(&sendata);
			//targetdata.t_h = sendata.t_h;
			//targetdata.t_m = sendata.t_m;
			//targetdata.t_s = sendata.t_s;
			//targetdata.t_ms = sendata.t_ms;
			//printf("target_th = %d\ntarget_tm = %d\ntarget_ts = %d\ntarget_tms = %d\n", targetdata.t_h, targetdata.t_m, targetdata.t_s, targetdata.t_ms);
			
		}
		else
		{
			x1 = 0;
			y1 = 0;
			detect_num = 0;
		}
		
		targetdata.x = x1;
		targetdata.y = y1;
		targetdata.frame_num = frame_num;
		p1->output->push(move(targetdata));
		//背景更新
		printf("************************frame_num = %d***********************\n",frame_num);
		if ((frame_num % 50 == 0 && x1 == 0) | detect_num >= 50)
		{
			img_back = image_pro.clone();
			printf("****************************背景更新************************\n");
		}
		
		gettimeofday(&end_2, NULL);
		printf("--------------------Image Process = %fms-----------------------------------\n", (double)(end_2.tv_usec - start_2.tv_usec)/1000);
	}
	printf("\n------------------------------------------结束目标检测线程----------------------------------\n");
	p1->output->end();
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
	addrSrv.sin_addr.s_addr = inet_addr("192.168.1.11");//ip地址重要！！！Srv IP is "192.168.1.10"
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(10011);//重要！！！端口编号10011
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
			
			//printf("f_num : %d\nt_h : %d\nt_m : %d\nt_s : %d\nms : %d\n", sendinfos.f_num, sendinfos.t_h, sendinfos.t_m, sendinfos.t_s, sendinfos.t_ms);
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

void compute_map()
{
	static ushort   colorimage_buf1[YDIM*XDIM*COLORS];
	//设置AOI感兴趣区域，一般不要修改
	int     i = 0;
	int     j = 0;
	int     cx = 0;	// left coordinate of centered AOI
	int     cy = 0;	// top	coordinate of centered AOI
	int nr = 512;//行
	int nc = 640;//列
	int total = nr*nc;//像素数
	
	int FrameNum = 0;
	pxd_goLive(UNITSMAP, 1);
	pxd_readushort(UNITSMAP, 1, cx, cy, cx + XDIM, cy + YDIM, colorimage_buf1, sizeof(colorimage_buf1) / sizeof(ushort), "Grey");

	struct timeval start_1, end_1, start_p;
	gettimeofday(&start_1, NULL);
	while (pxd_goneLive(UNITSMAP, 0))//capture picture
	{
		gettimeofday(&start_p, NULL);
		if ((i = pxd_readushort(UNITSMAP, 1, cx, cy, cx + XDIM, cy + YDIM, colorimage_buf1, sizeof(colorimage_buf1) / sizeof(ushort), "Grey")) != XDIM * YDIM*COLORS) {/*xiugai*/
			if (i < 0)
				printf("pxd_readuchar: %s\n", pxd_mesgErrorCode(i));
			else
				printf("pxd_readuchar error: %d != %d\n", i, XDIM*YDIM * 3);
			user("");
			break;
		}
		//将colorimage_buf1中的16bits数据赋值给Mat矩阵
		Mat src(YDIM, XDIM, CV_16UC1, colorimage_buf1);//原始图像
		FrameNum++;
		ushort *p_1 = NULL;
		if (FrameNum  == 1000)//每50帧更新一次映射关系
		{
			//存储直方图统计结果的数组
			unsigned int hist[16384] = { 0 };
			//扫描原图像
			for (int i = 0; i < nr; i++)
			{
				//获取第i行像素数组首指针
				p_1 = src.ptr<ushort>(i);
				for (int j = 0; j < nc; j++)
				{
					hist[p_1[j]] ++;
				}
			}

			//计算灰度变换函数
			//transf_fun存储均衡前像素值与均衡后像素值的映射关系
			transf_fun[0] = (uchar)(255 * (hist[0] * 1.0) / (total*1.0));
			//累积
			for (int i = 1; i < 16384; i++)
			{
				hist[i] = hist[i - 1] + hist[i];
				transf_fun[i] = (uchar)(255 * (hist[i] * 1.0) / (total*1.0));
			}
			printf("**************************映射关系初始化**********************\n");
			break;
		}
	}
}

int main(void)
{
	// Say Hello
	//判断是否xclib与相机连接成功
	hello();
	//pxd_PIXCIopen函数很重要，注意函数内部文件路径的格式，.fmt文件如何保存
	pxd_PIXCIopen("", "", "/home/nvidia/Desktop/try1/ddd14.fmt");//important
	//pxd_PIXCIopen("", "", "/home/nvidia/Desktop/histeq_detect_udp/ddd_1024.fmt");//important
	printf("Image frame buffer memory size: %.3f Kbytes\n", (double)pxd_infoMemsize(UNITSMAP) / 1024);
	printf("Image frame buffers           : %d\n", pxd_imageZdim());
	printf("Number of boards              : %d\n", pxd_infoUnits());

	printf("Frame Grabber %d\n", pxd_infoModel(UNITSMAP));

	printf("xdim           = %d\n", pxd_imageXdim());
	printf("ydim           = %d\n", pxd_imageYdim());
	printf("colors         = %d\n", pxd_imageCdim());
	printf("bits per pixel = %d\n", pxd_imageCdim()*pxd_imageBdim());
	

	//set_system_time();

	compute_map();

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

	int fd1 = serialport_inti();//初始化串口
	close_video_flow(fd1);
	return 0;	
}




