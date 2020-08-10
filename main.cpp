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
#define AOI_XDIM 640 //width
#define AOI_YDIM 512 //height
#define COLORS 1

#define BAOTOU 8


#define SYSFS_GPIO_EXPORT           "/sys/class/gpio/export"  
#define SYSFS_GPIO_RST_PIN_VAL      "388"   
#define SYSFS_GPIO_RST_DIR          "/sys/class/gpio/gpio388/direction"
#define SYSFS_GPIO_RST_DIR_VAL      "IN"  
#define SYSFS_GPIO_RST_VAL          "/sys/class/gpio/gpio388/value"
#define SYSFS_GPIO_RST_VAL_H        "1"
#define SYSFS_GPIO_RST_VAL_L        "0"
 //------------------------------全局变量--------------
const int AREA_THRESHOLD = 150;//面积阈值




void *img_enhance_thread(Queue<ImageData> *q)
{
	

	//定义存储16bits前景图像的缓冲区
	static ushort   colorimage_buf1[AOI_YDIM*AOI_XDIM*COLORS];
	//设置AOI感兴趣区域，一般不要修改
	int     i = 0;
	int     j = 0;
	int     cx = 0;	// left coordinate of centered AOI
	int     cy = 0;	// top	coordinate of centered AOI
	int FrameNum = 0; //帧数
	//pxd_doSnap(UNITSMAP, 1, 0); //保存单张图片
	//pxd_readushort函数捕捉16bits数据到缓冲区，成功i>0,i<0失败，函数中止运行
	//捕捉16bits图像到缓冲区colorimage_buf1
	//pxd_goneLive函数源源不断的捕获图像，手册有介绍
	while (pxd_goneLive(UNITSMAP, 0))//capture picture
	{
		if ((i = pxd_readushort(UNITSMAP, 1, cx, cy, cx + AOI_XDIM, cy + AOI_YDIM, colorimage_buf1, sizeof(colorimage_buf1) / sizeof(ushort), "Grey")) != AOI_XDIM * AOI_YDIM*COLORS) {/*xiugai*/
			if (i < 0)
				printf("pxd_readuchar: %s\n", pxd_mesgErrorCode(i));
			else
				printf("pxd_readuchar error: %d != %d\n", i, AOI_XDIM*AOI_YDIM * 3);
			user("");
			break;
		}
		//将colorimage_buf1中的16bits数据赋值给Mat矩阵
		Mat src(AOI_YDIM, AOI_XDIM, CV_16UC1, colorimage_buf1);//原始图像
		
	//---------------------------16bits 图像直方图均衡化----------------------------------//
		int nr = src.rows;//512
		int nc = src.cols;//640
		//像素总数
		int total = nr * nc;
		//转换后的目标矩阵，直方图均衡化结果
		Mat dst_2(nr, nc, CV_8UC1);
		//printf("total=%i\n",total);	
		//获取原图像直方图
		ushort *p_1 = NULL;
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
		uchar transf_fun[16384] = { 0 };
		transf_fun[0] = (uchar)(255 * (hist[0] * 1.0) / (total*1.0));
		//累积
		for (int i = 1; i < 16384; i++)
		{
			hist[i] = hist[i - 1] + hist[i];
			transf_fun[i] = (uchar)(255 * (hist[i] * 1.0) / (total*1.0));
		}

		uchar * p_2 = NULL;
        	uchar img_con[512*640];
		for (int i = 0; i < nr; i++)
		{
			//获取第i行像素数组首指针
			p_2 = dst_2.ptr<uchar>(i);
			p_1 = src.ptr<ushort>(i);
			//根据映射关系将原图像灰度替换成直方图均衡后的灰度
			for (int j = 0; j < nc; j++)
			{
				p_2[j] = transf_fun[p_1[j]];
                        	img_con[i*640+j] = transf_fun[p_1[j]];
			}
		}	
		//将直方图均衡化结果dst_2复制给img，img进行网络传输。
		//注意！！！考虑等号赋值条件与深拷贝 浅拷贝之间的关系
		FrameNum++;
		ImageData imgdata;
		imgdata.image = dst_2.clone();
		imgdata.frame_num = FrameNum;
		q->push(move(imgdata));
		
		if(FrameNum >= 100)
		{
			FrameNum = 0;
		}
	
	}
	q->end();
	do_close();//关闭视频流
	return NULL;
}


void *image_process_thread(Pipe<ImageData, TargetData> *p1)
{
	int detect = 0;
	unsigned short x1 = 0;
	unsigned short y1 = 0;
	int update_flag = 0;
	while(true)
	{

		
		unique_ptr<ImageData> imgdata;
		imgdata = p1->input->pop();
		if(imgdata == NULL)
		{
			p1->output->end();
			break;
		}
		Mat image_pro = imgdata->image;
		int frame_num = imgdata->frame_num;
		 
		Mat img_back(256, 320, CV_8UC1);
		//背景初始化
		if(update_flag == 0)
		{
			img_back = image_pro.clone();
			update_flag = 1;
		}

		//感兴趣区域
		Mat img_windows(256, 320, CV_8UC1);
		for (int i = 0; i < 256; i++)
		{
			for (int j = 0; j < 320; j++)
			{
				img_windows.at<uchar>(i,j) = image_pro.at<uchar>(128+i,160+j);
			}
		}	
		
		//目标检测
		vector<DetectInfo> detect_infos = detection(img_back, img_windows, AREA_THRESHOLD);
		if(detect_infos.size())
		{
			x1 = 128 + (unsigned short)detect_infos[0].x;
			y1 = 160 + (unsigned short)detect_infos[0].y;
			detect = 1;
		}
		else
		{
			detect = 0;
		}

		//背景更新
		if (frame_num % 50 == 0 && detect == 0)
		{
			img_back = img_windows.clone();
			printf("---------------------------------背景更新----------------------------------\n");
		}
		
		TargetData targetdata;
		targetdata.x = x1;
		targetdata.y = y1;
		p1->output->push(move(targetdata));
	}
	return NULL;
}

void *receive_data_thread(Queue<ReceiveInfo> *r)
{
	int fd = serialport_inti();//初始化串口
	char rcv_buf[10];
	ReceiveInfo *rcv_info;
	while(true)
	{
		int len = UART0_Recv(fd, rcv_buf,sizeof(ReceiveInfo));    
        	if(len >= sizeof(ReceiveInfo))    
        	{    
			rcv_info = reinterpret_cast<ReceiveInfo *>(rcv_buf);
			printf("FrameNum = %d    time = %d:%d:%d\n", rcv_info->f_num, rcv_info->t_h, rcv_info->t_m, rcv_info->t_ms);

                }    
                else    
                {    
                    	printf("cannot receive data\n");    
                }    
		
		r->push(move(*rcv_info));
	}
	close(fd);
	return NULL;
}

void *send_data_thread(Pipe<TargetData, ReceiveInfo> *p2)
{
			
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


	while(true)
	{
		unique_ptr<ReceiveInfo> rcvinfos;
		rcvinfos = p2->output->pop();
		
		unique_ptr<TargetData> targetdata;
		targetdata = p2->input->pop();
		
		if(rcvinfos == NULL || targetdata == NULL)
		{
			break;
		}

		SendInfo sendinfos;
		sendinfos.f_num = rcvinfos->f_num;
		sendinfos.t_h = rcvinfos->t_h;
		sendinfos.t_m = rcvinfos->t_m;
		sendinfos.t_ms = rcvinfos->t_ms;
		sendinfos.x1 = targetdata->x;
		sendinfos.y1 = targetdata->y;
		Net_Send_new(sockClient, addrSrv, &sendinfos);
	}
	
	close(sockClient);//关闭socket
	return NULL;
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

	Queue<ImageData> imagedata;
	Queue<TargetData> targetdata;
	Queue<ReceiveInfo> rcvinfos;
	Pipe<ImageData, TargetData> p1(&imagedata, &targetdata);
	Pipe<TargetData, ReceiveInfo> p2(&targetdata, &rcvinfos);
	
	pthread_t t1, t2, t3, t4;
	pthread_create(&t1, NULL, (THREAD_FUNC)img_enhance_thread, &imagedata);
	pthread_create(&t2, NULL, (THREAD_FUNC)image_process_thread, &p1);
	pthread_create(&t3, NULL, (THREAD_FUNC)receive_data_thread, &rcvinfos);
	pthread_create(&t4, NULL, (THREAD_FUNC)send_data_thread, &p2); 

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	pthread_join(t3, NULL);
	pthread_join(t4, NULL);
	return 0;	
}




